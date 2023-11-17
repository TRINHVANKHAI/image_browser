#ifndef SETTINGMENU_H
#define SETTINGMENU_H

#include <QDialog>

namespace Ui {
class SettingMenu;
}

struct usrSettingsData {
    QString localDownloadDir;
};

class SettingMenu : public QDialog
{
    Q_OBJECT

public:
    explicit SettingMenu(QWidget *parent = nullptr);
    ~SettingMenu();
    int loadSettings();

private slots:
    void on_pushButtonSettingApply_clicked();
    void on_pushButtonSettingCancel_clicked();
    void on_pushButtonChangeLocalDownloadDir_clicked();
    void on_localDownloadDirChanged(QString dir);
signals:
    void localDownloadDirChanged(QString dir);

private:
    int saveSettings();
    int loadDefaultSettings();
    struct usrSettingsData usrSettings;
    Ui::SettingMenu *ui;
};

#endif // SETTINGMENU_H
