#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QAction>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    // This tells the compiler that we are overriding Qt's event filter function
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void handleLogin();
    void togglePasswordVisibility();
    void handleForgotPassword();

private:
    Ui::MainWindow *ui;
    QAction *eyeAction;
};

#endif // MAINWINDOW_H