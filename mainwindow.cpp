#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QFile>
#include <QTextStream>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

// Forgot password — contact admin message
void MainWindow::on_forgotPasswordLabel_linkActivated(const QString &/*link*/)
{
    QMessageBox::information(this,
                             "Account Recovery",
                             "Please contact the System Administrator to reset your password.");
}

// Login button — authenticate from CSV, open admin/doctor/receptionist window if valid
void MainWindow::on_loginButton_clicked()
{
    QString enteredUser = ui->usernameInput->text().trimmed();
    QString enteredPass = ui->passwordInput->text().trimmed();
    QString selectedRole = ui->roleComboBox->currentText().trimmed();

    if (enteredUser.isEmpty() || enteredPass.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Please fill in all fields.");
        return;
    }

    QFile file(":/database/users.csv");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Error", "Database file not found.");
        return;
    }

    QTextStream in(&file);
    bool authenticated = false;

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;

        QStringList fields = line.split(",");

        // CSV format: username, password, role
        if (fields.size() >= 3) {
            QString csvUser = fields[0].trimmed();
            QString csvPass = fields[1].trimmed();
            QString csvRole = fields[2].trimmed();

            if (enteredUser == csvUser &&
                enteredPass == csvPass &&
                selectedRole == csvRole) {
                authenticated = true;
                break;
            }
        }
    }
    file.close();

    if (authenticated) {
        if (selectedRole == "Admin") {
            // Pass 'this' so the admin window can show the login screen on logout
            adminWindow = new admin(this);
            adminWindow->setAttribute(Qt::WA_DeleteOnClose);
            adminWindow->show();
            this->hide();
        } else {
            // Doctor / Receptionist windows — add here when ready
            QMessageBox::information(this, "Login Successful",
                                     QString("Welcome, %1!\n(%2 dashboard coming soon.)")
                                         .arg(enteredUser).arg(selectedRole));
        }
    } else {
        QMessageBox::warning(this, "Login Failed",
                             "Invalid username, password, or role.");
    }
}