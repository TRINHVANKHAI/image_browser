#include "sftpclient.h"
#include <QDebug>

SftpClient::SftpClient(QObject *parent)
    : QObject{parent}
{
    ssh_client_session = ssh_new();
    if(ssh_client_session==NULL) {
        qDebug() << "Error: open session error";
    }
}

SftpClient::~SftpClient()
{
    qDebug() << "~SftpClient()";

    if(sftp_client_session) {
        sftp_free(sftp_client_session);
        sftp_client_session = NULL;
    }
    this->disconnectFromServer();
    if(ssh_client_session) {
        ssh_free(ssh_client_session);
        ssh_client_session = NULL;
    }
}

int SftpClient::verifyKnownHost()
{
    enum ssh_known_hosts_e state;
    unsigned char *hash = NULL;
    ssh_key srv_pubkey = NULL;
    size_t hlen;
    char buf[10];
    char *hexa;
    char *p;
    int cmp;
    int rc;

    rc = ssh_get_server_publickey(ssh_client_session, &srv_pubkey);
    if (rc < 0) {
        return -1;
    }

    rc = ssh_get_publickey_hash(srv_pubkey,
                                SSH_PUBLICKEY_HASH_SHA1,
                                &hash,
                                &hlen);
    ssh_key_free(srv_pubkey);
    if (rc < 0) {
        return -1;
    }

    state = ssh_session_is_known_server(ssh_client_session);
    switch (state) {
    case SSH_KNOWN_HOSTS_OK:
        /* OK */

        break;
    case SSH_KNOWN_HOSTS_CHANGED:
        fprintf(stderr, "Host key for server changed: it is now:\n");
        ssh_print_hexa("Public key hash", hash, hlen);
        fprintf(stderr, "For security reasons, connection will be stopped\n");
        ssh_clean_pubkey_hash(&hash);

        return -1;
    case SSH_KNOWN_HOSTS_OTHER:
        fprintf(stderr, "The host key for this server was not found but an other"
                        "type of key exists.\n");
        fprintf(stderr, "An attacker might change the default server key to"
                        "confuse your client into thinking the key does not exist\n");
        ssh_clean_pubkey_hash(&hash);

        return -1;
    case SSH_KNOWN_HOSTS_NOT_FOUND:
        fprintf(stderr, "Could not find known host file.\n");
        fprintf(stderr, "If you accept the host key here, the file will be"
                        "automatically created.\n");

        /* FALL THROUGH to SSH_SERVER_NOT_KNOWN behavior */

    case SSH_KNOWN_HOSTS_UNKNOWN:
        hexa = ssh_get_hexa(hash, hlen);
        fprintf(stderr,"The server is unknown. Do you trust the host key?\n");
        fprintf(stderr, "Public key hash: %s\n", hexa);
        ssh_string_free_char(hexa);
        ssh_clean_pubkey_hash(&hash);
#if 0
        p = fgets(buf, sizeof(buf), stdin);
        if (p == NULL) {
            return -1;
        }

        cmp = strncasecmp(buf, "yes", 3);
        if (cmp != 0) {
            return -1;
        }
#endif
        rc = ssh_session_update_known_hosts(ssh_client_session);
        if (rc < 0) {
            fprintf(stderr, "Error %s\n", strerror(errno));
            return -1;
        }

        break;
    case SSH_KNOWN_HOSTS_ERROR:
        fprintf(stderr, "Error %s", ssh_get_error(ssh_client_session));
        ssh_clean_pubkey_hash(&hash);
        return -1;
    }

    ssh_clean_pubkey_hash(&hash);
    return 0;
}

int SftpClient::connectToServer(char *serverName, int port, char *user)
{
    int ret;
    ssh_options_set(ssh_client_session, SSH_OPTIONS_HOST, serverName);
    ssh_options_set(ssh_client_session, SSH_OPTIONS_PORT, &port);
    ssh_options_set(ssh_client_session, SSH_OPTIONS_USER, user);
    ret = ssh_connect(ssh_client_session);
    if(ret != SSH_OK) {
        qDebug() << "Error: connecting to local host:" << ssh_get_error(ssh_client_session);
        return ret;
    } else {
        qDebug() << "Connected to localhost";
        sshClientStatus = SSH_CLIENT_STATUS_CONNECTED;
        emit sftpClientStatus(sshClientStatus);
    }

    ret = this->verifyKnownHost();
    if(ret == 0) {
        sshClientStatus = SSH_CLIENT_STATUS_HOST_VERIFIED;
        emit sftpClientStatus(sshClientStatus);
    }
    return ret;
}

void SftpClient::disconnectFromServer() {
    ssh_disconnect(ssh_client_session);
    sshClientStatus = SSH_CLIENT_STATUS_DISCONNECTED;
    emit sftpClientStatus(sshClientStatus);
}

int SftpClient::loginAuthPassword(char *pass)
{
    int rc;
    rc = ssh_userauth_password(ssh_client_session, NULL, pass);
    if (rc != SSH_AUTH_SUCCESS)
    {
        fprintf(stderr, "Error authenticating with password: %s\n",
                ssh_get_error(ssh_client_session));
    } else {
        sshClientStatus = SSH_CLIENT_STATUS_AUTHENTICATED;
        emit sftpClientStatus(sshClientStatus);
    }
    return rc;
}

void SftpClient::sftpClientSessionInit()
{
    if(sshClientStatus == SSH_CLIENT_STATUS_AUTHENTICATED) {
        qDebug() << "SFTP create session ";
        sftp_client_session = sftp_new(ssh_client_session);
        if(sftp_client_session == NULL)
        {
            fprintf(stderr, "Error allocating SFTP session: %s\n",
                    ssh_get_error(ssh_client_session));
            sftp_client_session = NULL;
            qDebug() << "Error: SFTP create session failed";
        }
        qDebug() << "SFTP init session ";
        if(sftp_init(sftp_client_session) != SSH_OK) {
            fprintf(stderr, "Error initializing SFTP session: code %d.\n",
                    sftp_get_error(sftp_client_session));
            sftp_free(sftp_client_session);
            sftp_client_session = NULL;
            qDebug() << "ERR: SFTP free session ";
        } else {
            sshClientStatus = SSH_CLIENT_STATUS_SFTP_ESTABLISHED;
            emit sftpClientStatus(sshClientStatus);
            qDebug() << "SSH_CLIENT_STATUS_SFTP_ESTABLISHED ";
        }

    }
}

int SftpClient::sftpChangeListDir(const char *dirname)
{
  sftp_dir dir;
  sftp_attributes attributes;
  int rc;
  if((sftp_client_session == NULL) ||
          (sshClientStatus != SSH_CLIENT_STATUS_SFTP_ESTABLISHED)) {
      fprintf(stderr, "sftp_client_session is not established\n");
      return -1;
  }
  sftpCurDir = dirname;
  dir = sftp_opendir(sftp_client_session, dirname);
  if (!dir)
  {
    fprintf(stderr, "Directory not opened: %s\n",
            ssh_get_error(ssh_client_session));
    return SSH_ERROR;
  }

  while ((attributes = sftp_readdir(sftp_client_session, dir)) != NULL)
  {
      if(attributes->type == 1) {
          QString filename = QString::fromUtf8(attributes->name);
          emit sftpClientListFileResponse(filename, attributes->size, attributes->mtime);
          sftp_attributes_free(attributes);
      }
  }

  if (!sftp_dir_eof(dir))
  {
    fprintf(stderr, "Can't list directory: %s\n",
            ssh_get_error(ssh_client_session));
    sftp_closedir(dir);
    return SSH_ERROR;
  }

  rc = sftp_closedir(dir);
  if (rc != SSH_OK)
  {
    fprintf(stderr, "Can't close directory: %s\n",
            ssh_get_error(ssh_client_session));
    return rc;
  }
}

// Good chunk size
#define MAX_XFER_BUF_SIZE 16384

int SftpClient::fileDownLoadSync(QString fileName, QString localDir)
{
  int access_type;
  sftp_file file;
  char buffer[MAX_XFER_BUF_SIZE];
  int nbytes, nwritten, rc;
  FILE *downloadedFile;

  access_type = O_RDONLY;
  QString filePath = sftpCurDir + "/" + fileName;
  QString localFilePath = localDir + "/" + fileName;
  qDebug() << "fileDownLoadSync: SRC " << filePath;
  qDebug() << "fileDownLoadSync: DST " << localFilePath;
  file = sftp_open(sftp_client_session, filePath.toLocal8Bit().data(),
                   access_type, 0);
  if (file == NULL) {
      fprintf(stderr, "Can't open file for reading: %s\n",
              ssh_get_error(ssh_client_session));
      return SSH_ERROR;
  }

  downloadedFile = fopen(localFilePath.toLocal8Bit().data(), "w");
  if (downloadedFile == NULL) {
      fprintf(stderr, "Can't open file for writing: %s\n",
              strerror(errno));
      return SSH_ERROR;
  }

  for (;;) {
      nbytes = sftp_read(file, buffer, sizeof(buffer));
      if (nbytes == 0) {
          break; // EOF
      } else if (nbytes < 0) {
          fprintf(stderr, "Error while reading file: %s\n",
                  ssh_get_error(ssh_client_session));
          sftp_close(file);
          return SSH_ERROR;
      }

      fwrite(buffer, nbytes, 1, downloadedFile);
  }
  qDebug() << "Fclose file afterdownload";
  fclose(downloadedFile);
  rc = sftp_close(file);

  qDebug() << "sftp_close file afterdownload";
  if (rc != SSH_OK) {
      fprintf(stderr, "Can't close the read file: %s\n",
              ssh_get_error(ssh_client_session));
      return rc;
  }
  qDebug() << "return file afterdownload";
  return SSH_OK;
}

void SftpClient::onFileDownloadRequested(QString fileName, QString localDir)
{

    qDebug() << "File requested to download: " << fileName;

    if(this->fileDownLoadSync(fileName, localDir) == SSH_OK) {
        emit this->requestedFileDownloadStatus(fileName, 1);
    } else {
        emit this->requestedFileDownloadStatus(fileName, 0);
    }
}
