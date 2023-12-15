#include "sftpclient.h"
#include <QDebug>
#include <QDateTime>

SftpClient::SftpClient(QObject *parent) :
    QThread(parent)
{

}

SftpClient::~SftpClient()
{
    qDebug() << "~SftpClient()";
    this->disconnectFromSftpServer();
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
        rc = ssh_session_update_known_hosts(ssh_client_session);
        if (rc < 0) {
            fprintf(stderr, "Error %s\n", strerror(errno));
            return -1;
        }
        break;
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
        /*Update known host anyway*/
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

int SftpClient::connectToSftpServer(char *serverName, int port, char *user, char *pass)
{
    int ret;
    ssh_client_session = ssh_new();
    if(ssh_client_session==NULL) {
        qDebug() << "Error: open session error";
    }
    int timeout_sec = 5;
    ssh_options_set(ssh_client_session, SSH_OPTIONS_TIMEOUT, &timeout_sec);
    qDebug() << "Connecting to: " << serverName << ":" << port;
    ssh_options_set(ssh_client_session, SSH_OPTIONS_HOST, serverName);
    ssh_options_set(ssh_client_session, SSH_OPTIONS_PORT, &port);

    ret = ssh_connect(ssh_client_session);
    if(ret != SSH_OK) {
        qDebug() << "Error: connecting to:" << serverName << ":" << port << ssh_get_error(ssh_client_session);
        sshClientStatus = SSH_CLIENT_STATUS_ERR_CONNECTION;
        emit sftpClientStatus(sshClientStatus);
        ssh_disconnect(ssh_client_session);
        ssh_free(ssh_client_session);
        ssh_client_session = NULL;
        return ret;
    } else {
        qDebug() << "Connected to: " << serverName;
        sshClientStatus = SSH_CLIENT_STATUS_CONNECTED;
        emit sftpClientStatus(sshClientStatus);
    }

    ret = this->verifyKnownHost();
    if(ret == 0) {
        sshClientStatus = SSH_CLIENT_STATUS_HOST_VERIFIED;
        emit sftpClientStatus(sshClientStatus);
    } else {
        sshClientStatus = SSH_CLIENT_STATUS_ERR_KNOWNHOST;
        emit sftpClientStatus(sshClientStatus);

    }

    ret = ssh_userauth_password(ssh_client_session, user, pass);
    if (ret != SSH_AUTH_SUCCESS)
    {
        fprintf(stderr, "Error authenticating with password: %s\n",
                ssh_get_error(ssh_client_session));
        sshClientStatus = SSH_CLIENT_STATUS_ERR_AUTHENTICATION;
        emit sftpClientStatus(sshClientStatus);
        ssh_disconnect(ssh_client_session);
        ssh_free(ssh_client_session);
        ssh_client_session = NULL;
        return ret;
    } else {
        sshClientStatus = SSH_CLIENT_STATUS_AUTHENTICATED;
        emit sftpClientStatus(sshClientStatus);
    }

    ret = sftpClientSessionInit();
    return ret;
}

void SftpClient::disconnectFromSftpServer() {
    if(sshClientStatus == SSH_CLIENT_STATUS_SFTP_ESTABLISHED) {
        sftpClientSessionDestroy();
        ssh_disconnect(ssh_client_session);
        ssh_free(ssh_client_session);
        ssh_client_session = NULL;
    }
    sshClientStatus = SSH_CLIENT_STATUS_DISCONNECTED;
    emit sftpClientStatus(sshClientStatus);
}

int SftpClient::sftpClientSessionInit()
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
            return -1;
        }
        qDebug() << "SFTP init session ";
        if(sftp_init(sftp_client_session) != SSH_OK) {
            fprintf(stderr, "Error initializing SFTP session: code %d.\n",
                    sftp_get_error(sftp_client_session));
            sftp_free(sftp_client_session);
            sftp_client_session = NULL;
            qDebug() << "ERR: SFTP free session ";
            return -2;
        } else {
            sshClientStatus = SSH_CLIENT_STATUS_SFTP_ESTABLISHED;
            emit sftpClientStatus(sshClientStatus);
            qDebug() << "SSH_CLIENT_STATUS_SFTP_ESTABLISHED ";
            return 0;
        }

    }
    return 0;
}

void SftpClient::sftpClientSessionDestroy() {
    sftp_free(sftp_client_session);
    sftp_client_session = NULL;
}

int SftpClient::sftpChangeListDir(const char *dirname)
{
  sftp_dir dir;
  sftp_attributes attributes;
  QStringList fileList;
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
          QString filenamepro = QString::fromUtf8(attributes->name) + QString(",")
                           + QString::number(attributes->size/1024) +QString(" KB") + QString(",")
                           + QDateTime::fromSecsSinceEpoch(attributes->mtime).toString("yyyy/MM/dd hh:mm");

          fileList.append(filenamepro);
          sftp_attributes_free(attributes);
      }
  }
  fileList.sort();
  emit sftpClientListFileResponse(fileList);

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

  fclose(downloadedFile);
  rc = sftp_close(file);

  if (rc != SSH_OK) {
      fprintf(stderr, "Can't close the read file: %s\n",
              ssh_get_error(ssh_client_session));
      return rc;
  }
  return SSH_OK;
}

int SftpClient::downloadSelectedFile(QString fileName, QString localDir)
{
    if(this->fileDownLoadSync(fileName, localDir) == SSH_OK) {
        return 0;
    } else {
        return -1;
    }
}

int SftpClient::deleteSelectedFile(QString fileName)
{
    QString filePath = sftpCurDir + "/" + fileName;
    if(sftp_unlink(sftp_client_session, filePath.toLocal8Bit().data()) == SSH_OK) {
        return 0;
    } else {
        return -1;
    }
}

void SftpClient::onKeepAliveEvent()
{
    if((sshClientStatus == SSH_CLIENT_STATUS_SFTP_ESTABLISHED)&&
            (ssh_client_session!=NULL)) {
        ssh_send_ignore(ssh_client_session, "ignore");
    }
}
