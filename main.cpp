#include "imagebrowser.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ImageBrowser w;
    w.setWindowTitle("Image browser");
    w.show();
    return a.exec();
}
