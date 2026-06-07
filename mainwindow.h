#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

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
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void on_forgotPass_linkActivated(const QString &link);
    void on_loginButton_clicked();

private:
    Ui::MainWindow *ui;
    void attemptLogin();
};

#endif // MAINWINDOW_H