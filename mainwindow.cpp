#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "doctorwindow.h"
#include "adminwindow.h"
#include "receptionistwindow.h"
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

    //rolecombobox dropdown arrow
    ui->roleComboBox->setStyleSheet(
        "QComboBox {"
        "    background-color: white;"
        "    color: black;"
        "    padding-left: 5px;"
        "    padding-right: 25px;"
        "}"
        "QComboBox::drop-down {"
        "    subcontrol-position: top right;"
        "    width: 25px;"
        "    border: none;"
        "}"
        /* Default state: Pointing Right (Scaled Down) */
        "QComboBox::down-arrow {"
        "    image: url(:/images/right-arrow-icon.png);"
        "    width: 12px;"  /* Adjust this value to make it smaller or larger */
        "    height: 12px;" /* Keep width and height equal to maintain aspect ratio */
        "}"
        /* Clicked/Open state: Pointing Down (Scaled Down) */
        "QComboBox::down-arrow:on {"
        "    image: url(:/images/down-arrow-icon.png);"
        "    width: 12px;"  /* Make sure this matches the width above */
        "    height: 12px;" /* Make sure this matches the height above */
        "}"

        "QAbstractItemView {"
        "    background-color: white;"
        "    color: black;"
        "    selection-background-color: #e0e0e0;"
        "    selection-color: black;"
        "}"
        );
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

    QFile file(":/database/staff_database.csv");
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

        // CSV format: username, password, role, full_name
        if (fields.size() >= 3) {
            QString csvUser = fields[0].trimmed();
            QString csvPass = fields[1].trimmed();
            QString csvRole = fields[2].trimmed();

            if (enteredUser == csvUser &&
                enteredPass == csvPass &&
                selectedRole == csvRole) {

                authenticated = true;

                // 1. Extract full name right here while 'fields' holds the correct row data
                QString fullName = (fields.size() >= 5) ? fields[4].trimmed() : csvUser;

                // 2. Open the respective window immediately
                if (selectedRole == "Admin") {
                    adminwindow *adminWin = new adminwindow(fullName);
                    adminWin->setAttribute(Qt::WA_DeleteOnClose);
                    adminWin->show();
                    this->hide();
                }
                else if (selectedRole.compare("Doctor", Qt::CaseInsensitive) == 0) {
                    qDebug() << "Launching Doctor Window for: " << fullName;
                    doctorwindow *doctorWin = new doctorwindow(fullName);
                    doctorWin->setAttribute(Qt::WA_DeleteOnClose);
                    doctorWin->show();
                    this->hide();
                }
                else if (selectedRole.compare("Receptionist", Qt::CaseInsensitive) == 0) {
                    qDebug() << "Launching Receptionist Window...";
                    ReceptionistWindow *receptionistWin = new ReceptionistWindow();
                    receptionistWin->setAttribute(Qt::WA_DeleteOnClose);
                    receptionistWin->show();
                    this->hide();
                }

                break; // Break out of the while loop since we found the user
            }
        }
    }
    file.close(); // File closes safely here after loop ends or breaks

    // 3. If the loop finished and authenticated is still false, show the failure message
    if (!authenticated) {
        QMessageBox::warning(this, "Login Failed",
                             "Invalid username, password, or role.");
    }
}