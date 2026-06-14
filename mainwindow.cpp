#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "adminwindow.h"
#include <QIcon>
#include <QDebug>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QCoreApplication>


// Include your other dashboard window headers here
#include "adminwindow.h"
// #include "doctorwindow.h" // Uncomment or add your other roles' headers as you create them

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Enter key navigation updates
    connect(ui->userInput, &QLineEdit::returnPressed, this, [this]() {
        ui->passInput->setFocus();
    });
    connect(ui->passInput, &QLineEdit::returnPressed, this, &MainWindow::handleLogin);
    connect(ui->loginButton, &QPushButton::clicked, this, &MainWindow::handleLogin);

    // Password visibility action setup
    eyeAction = new QAction(this);
    eyeAction->setIcon(QIcon(":/images/ihide.png"));
    ui->passInput->addAction(eyeAction, QLineEdit::TrailingPosition);
    connect(eyeAction, &QAction::triggered, this, &MainWindow::togglePasswordVisibility);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::togglePasswordVisibility()
{
    if (ui->passInput->echoMode() == QLineEdit::Password) {
        ui->passInput->setEchoMode(QLineEdit::Normal);
        eyeAction->setIcon(QIcon(":/images/ishow.png"));
    } else {
        ui->passInput->setEchoMode(QLineEdit::Password);
        eyeAction->setIcon(QIcon(":/images/ihide.png"));
    }
}

void MainWindow::handleLogin() {
    qDebug() << "Handling login via handleLogin()... Redirecting to button logic.";

    // Call the button click slot directly
    on_loginButton_clicked();
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
            adminwindow *adminWin = new adminwindow();
            adminWin->setAttribute(Qt::WA_DeleteOnClose);
            adminWin->show();
            this->hide();
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


