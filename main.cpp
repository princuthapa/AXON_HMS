#include "mainwindow.h"

#include <QApplication>
#include <QStyleFactory>
#include <QPalette>
#include <QColor>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Force Fusion style
    QApplication::setStyle(QStyleFactory::create("Fusion"));

    // Force LIGHT palette
    QPalette palette;
    palette.setColor(QPalette::Window, QColor(255, 255, 255));
    palette.setColor(QPalette::WindowText, Qt::black);
    palette.setColor(QPalette::Base, Qt::white);
    palette.setColor(QPalette::AlternateBase, QColor(245, 245, 245));
    palette.setColor(QPalette::ToolTipBase, Qt::white);
    palette.setColor(QPalette::ToolTipText, Qt::black);
    palette.setColor(QPalette::Text, Qt::black);
    palette.setColor(QPalette::Button, Qt::white);
    palette.setColor(QPalette::ButtonText, Qt::black);
    palette.setColor(QPalette::BrightText, Qt::red);
    palette.setColor(QPalette::Highlight, QColor(0, 120, 215));
    palette.setColor(QPalette::HighlightedText, Qt::white);

    a.setPalette(palette);

    MainWindow w;
    w.show();

    return a.exec();
}