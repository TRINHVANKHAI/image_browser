#include "settingmenu.h"
#include "ui_settingmenu.h"
#include <QFileDialog>
#include <QDir>
#include <QStandardPaths>
#include <QJsonDocument>
#include <QJsonObject>

SettingMenu::SettingMenu(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingMenu)
{
    ui->setupUi(this);
    connect(this, SIGNAL(localDownloadDirChanged(QString)),
            this, SLOT(on_localDownloadDirChanged(QString)));
}

SettingMenu::~SettingMenu()
{
    delete ui;
}

int SettingMenu::saveSettings()
{
    QJsonObject settingsInfo;
    settingsInfo["localDownloadDir"] = usrSettings.localDownloadDir;
    QJsonDocument usrSettingsJDoc(settingsInfo);

    QFile usrSettingInfoFile("settings.json");
    if(!usrSettingInfoFile.open((QIODevice::WriteOnly))) {
        qWarning("Couldn't open file to save data");
        return -1;
    }

    usrSettingInfoFile.write(usrSettingsJDoc.toJson());
    usrSettingInfoFile.close();
    return 0;
}

int SettingMenu::loadSettings()
{
    QFile usrSettingInfoFile("settings.json");
    QString localDownloadDirStr;
    localDownloadDirStr.clear();
    if(!usrSettingInfoFile.open((QIODevice::ReadOnly))) {
        qWarning("Couldn't open file to read data");
        this->loadDefaultSettings();
        return 0;
    }

    QByteArray usrSettingsInfoData = usrSettingInfoFile.readAll();
    usrSettingInfoFile.close();
    QJsonObject usrSettingsInfoJson(QJsonDocument::fromJson(usrSettingsInfoData).object());
    localDownloadDirStr = usrSettingsInfoJson["localDownloadDir"].toString();
    if (localDownloadDirStr != this->usrSettings.localDownloadDir) {
        this->usrSettings.localDownloadDir = localDownloadDirStr;
        emit localDownloadDirChanged(this->usrSettings.localDownloadDir);
    }
    return 0;
}

int SettingMenu::loadDefaultSettings()
{
    this->usrSettings.localDownloadDir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    emit localDownloadDirChanged(this->usrSettings.localDownloadDir);
    return 0;
}

void SettingMenu::on_pushButtonSettingApply_clicked()
{
    QString tmpLocalDownloadDirStr = ui->labelLocalDownloadDirValue->text();
    if (tmpLocalDownloadDirStr != usrSettings.localDownloadDir) {
        this->usrSettings.localDownloadDir = tmpLocalDownloadDirStr;
        emit localDownloadDirChanged(this->usrSettings.localDownloadDir);
    }
    this->saveSettings();
    this->accept();
}


void SettingMenu::on_pushButtonSettingCancel_clicked()
{
    ui->labelLocalDownloadDirValue->setText(this->usrSettings.localDownloadDir);
    this->reject();
}


void SettingMenu::on_pushButtonChangeLocalDownloadDir_clicked()
{

    QString tmpLocalDownloadDirStr = QFileDialog::getExistingDirectory(this, "ダウンロードディレクトリを選択してください",
                                                                    QDir::homePath(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if(tmpLocalDownloadDirStr.isEmpty()) {
        tmpLocalDownloadDirStr = usrSettings.localDownloadDir;
    }
    ui->labelLocalDownloadDirValue->setText(tmpLocalDownloadDirStr);

}

void SettingMenu::on_localDownloadDirChanged(QString dir)

{
    ui->labelLocalDownloadDirValue->setText(dir);
}
