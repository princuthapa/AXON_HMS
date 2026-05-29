#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QFile>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // Apply to all QLineEdits globally
    this->setStyleSheet("QLineEdit { "
                        "    background-color: #fffff; " // Assuming a dark background
                        "    color: #00000; "            // Normal typed text color
                        "    placeholder-text-color: #36454F; " // Distinct placeholder color
                        "}");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_forgotPass_linkActivated(const QString &/*link*/)
{
    {
        QMessageBox::information(this,
                                 "Account Recovery",
                                 "Please contact the System Administrator to reset your password.");
    }
}

void MainWindow::on_loginButton_clicked()
{
        QString enteredUser = ui->userInput->text().trimmed();
        QString enteredPass = ui->passInput->text().trimmed();
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
                // adminWindow = new admin(this);
                // adminWindow->setAttribute(Qt::WA_DeleteOnClose);
                // adminWindow->show();
                // this->hide();
            }
            else if(selectedRole == "Doctor") {
                // Pass 'this' so the admin window can show the login screen on logout
                // adminWindow = new admin(this);
                // adminWindow->setAttribute(Qt::WA_DeleteOnClose);
                // adminWindow->show();
                // this->hide();
            }
            else {
                // Pass 'this' so the admin window can show the login screen on logout
                // adminWindow = new admin(this);
                // adminWindow->setAttribute(Qt::WA_DeleteOnClose);
                // adminWindow->show();
                // this->hide();
            }
        } else {
            QMessageBox::warning(this, "Login Failed",
                                 "Invalid username, password, or role.");
        }

}

