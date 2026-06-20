#include "mainwindow.h"

#include <QApplication>
#include <QStyleFactory>
#include <QStyle>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QPalette lightPalette = a.style()->standardPalette();
    a.setPalette(lightPalette);
    MainWindow w;
    w.show();
    return QApplication::exec();
}
