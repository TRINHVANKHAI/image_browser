QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
RC_ICONS = image_browser.ico

SOURCES += \
    dialog.cpp \
    filedirectorylistitem.cpp \
    loginwindow.cpp \
    main.cpp \
    imagebrowser.cpp \
    settingmenu.cpp \
    sftpclient.cpp

HEADERS += \
    dialog.h \
    filedirectorylistitem.h \
    imagebrowser.h \
    loginwindow.h \
    settingmenu.h \
    sftpclient.h

FORMS += \
    dialog.ui \
    filedirectorylistitem.ui \
    imagebrowser.ui \
    loginwindow.ui \
    settingmenu.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

LIBS += \
    -L "C:\vcpkg\vcpkg\packages\libssh_x64-windows\bin" -lssh

INCLUDEPATH += \
    "C:\vcpkg\vcpkg\packages\libssh_x64-windows\include"

RESOURCES += \
    image_browser.qrc
