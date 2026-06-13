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
    connect(ui->usernamelineEdit, &QLineEdit::returnPressed, this, [this]() {
        ui->passwordlineEdit->setFocus();
    });
    connect(ui->passwordlineEdit, &QLineEdit::returnPressed, this, &MainWindow::handleLogin);
    connect(ui->loginButton, &QPushButton::clicked, this, &MainWindow::handleLogin);

    // Password visibility action setup
    eyeAction = new QAction(this);
    eyeAction->setIcon(QIcon(":/images/ihide.png"));
    ui->passwordlineEdit->addAction(eyeAction, QLineEdit::TrailingPosition);
    connect(eyeAction, &QAction::triggered, this, &MainWindow::togglePasswordVisibility);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::togglePasswordVisibility()
{
    if (ui->passwordlineEdit->echoMode() == QLineEdit::Password) {
        ui->passwordlineEdit->setEchoMode(QLineEdit::Normal);
        eyeAction->setIcon(QIcon(":/images/ishow.png"));
    } else {
        ui->passwordlineEdit->setEchoMode(QLineEdit::Password);
        eyeAction->setIcon(QIcon(":/images/ihide.png"));
    }
}

void MainWindow::handleLogin()
{
    QString username = ui->usernamelineEdit->text().trimmed();
    QString password = ui->passwordlineEdit->text();
    QString selectedRole = ui->comboBox->currentText();

    // Print attempt parameters to debug terminal console
    qDebug() << "--- Login Attempt ---";
    qDebug() << "Input Username:" << username;
    qDebug() << "Input Role:" << selectedRole;

    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox msgBox(QMessageBox::Warning, "Error", "Username and password cannot be empty!", QMessageBox::Ok, this);
        msgBox.setStyleSheet("QLabel { color: black; }");
        msgBox.exec();
        return;
    }

    // DYNAMIC PATH: Looks for the CSV right next to where your app's .exe is executing
    QString filePath = QCoreApplication::applicationDirPath() + "/users.csv";
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "CRITICAL ERROR: Cannot access CSV file at path:" << filePath;

        QString errorPrompt = "Cannot open users.csv file!\n\nPlease copy 'users.csv' and paste it into:\n" + filePath;
        QMessageBox msgBox(QMessageBox::Critical, "Error", errorPrompt, QMessageBox::Ok, this);
        msgBox.setStyleSheet("QLabel { color: black; }");
        msgBox.exec();
        return;
    }

    bool credentialsValid = false;
    QTextStream in(&file);

    while (!in.atEnd()) {
        QString line = in.readLine();
        if (line.trimmed().isEmpty()) continue;

        QStringList parts = line.split(",");

        if (parts.size() >= 3) {
            QString fileUsername = parts[0].trimmed();
            QString filePassword = parts[1].trimmed();
            QString fileRole = parts[2].trimmed();

            if (username == fileUsername && password == filePassword && selectedRole == fileRole) {
                credentialsValid = true;
                break;
            }
        }
    }
    file.close();

    if (credentialsValid) {
        qDebug() << "Authentication Status: SUCCESS! Redirecting to dashboard routing layer.";

        // 1. Hide the current Login window cleanly
        this->hide();

        // 2. Route based on selected dashboard context role
        if (selectedRole == "Admin") {
            // Allocate Admin window to the heap memory architecture
            adminwindow *adminDash = new adminwindow();

            // Set up clean memory freeing attributes when closed
            adminDash->setAttribute(Qt::WA_DeleteOnClose);
            adminDash->show();

        } else if (selectedRole == "Doctor") {
            // Placeholder: When you have DoctorWindow set up, implement it similarly:
            // DoctorWindow *doctorDash = new DoctorWindow();
            // doctorDash->setAttribute(Qt::WA_DeleteOnClose);
            // doctorDash->show();

            QMessageBox::information(this, "Doctor Dashboard", "Login Successful!\nDoctor interface window mapping placeholder context.");
        } else {
            // General Fallback dashboard router routing context
            QMessageBox::information(this, "Success", "Login successful for role: " + selectedRole);
        }

    } else {
        qDebug() << "Authentication Status: FAILED. Credentials mismatch in data loops.";

        QMessageBox msgBox(QMessageBox::Warning, "Login Failed", "Invalid username, password, or role!", QMessageBox::Ok, this);
        msgBox.setStyleSheet("QLabel { color: black; }");
        msgBox.exec();

        ui->passwordlineEdit->clear();
    }
}