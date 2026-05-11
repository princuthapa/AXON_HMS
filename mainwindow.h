#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "admin.h"   // admin dashboard window

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_forgotPasswordLabel_linkActivated(const QString &link);
    void on_loginButton_clicked();

private:
    Ui::MainWindow *ui;
    admin *adminWindow = nullptr;   // pointer to the admin dashboard
};

#endif // MAINWINDOW_H