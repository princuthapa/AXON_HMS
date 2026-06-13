#include "mainwindow.h"
#include "adminwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QFile>
#include <QKeyEvent>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->setStyleSheet("QLineEdit {"
                        "    background-color: #ffffff;"
                        "    color: #000000;"
                        "    placeholder-text-color: #36454F;"
                        "}");

    // Install event filter on both input fields
    ui->userInput->installEventFilter(this);
    ui->passInput->installEventFilter(this);

    // Set tab order explicitly
    QWidget::setTabOrder(ui->userInput, ui->passInput);
    QWidget::setTabOrder(ui->passInput, ui->roleComboBox);
    QWidget::setTabOrder(ui->roleComboBox, ui->loginButton);

    // Set the first field as focused when window opens
    ui->userInput->setFocus();
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

        if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {

            // Enter on username field → jump to password field
            if (obj == ui->userInput) {
                ui->passInput->setFocus();
                return true;
            }

            // Enter on password field → trigger login
            if (obj == ui->passInput) {
                attemptLogin();
                return true;
            }
        }
    }

    // Pass everything else to the default handler
    return QMainWindow::eventFilter(obj, event);
}

// Extracted login logic into its own method so both
// the button click and Enter key can call it
void MainWindow::attemptLogin()
{
    QString enteredUser = ui->userInput->text().trimmed();
    QString enteredPass = ui->passInput->text().trimmed();
    QString selectedRole = ui->roleComboBox->currentText().trimmed();

    if (enteredUser.isEmpty() || enteredPass.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Please fill in all fields.");
        ui->userInput->setFocus(); // bring focus back to start
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
            QMessageBox::information(this, "Success",
                                     "Login successful! Opening Admin Dashboard...");
            adminwindow *adminWin = new adminwindow();
            adminWin->setAttribute(Qt::WA_DeleteOnClose);
            adminWin->show();
            this->hide();
        }
        else if (selectedRole == "Doctor") {
            // doctorWindow *docWin = new doctorWindow();
            // docWin->setAttribute(Qt::WA_DeleteOnClose);
            // docWin->show();
            // this->hide();
        }
        else {
            // other roles here
        }
    } else {
        QMessageBox::warning(this, "Login Failed",
                             "Invalid username, password, or role.");
        ui->passInput->clear();
        ui->passInput->setFocus(); // let user retype password quickly
    }
}

void MainWindow::on_forgotPass_linkActivated(const QString &/*link*/)
{
    QMessageBox::information(this,
                             "Account Recovery",
                             "Please contact the System Administrator "
                             "to reset your password.");
}

void MainWindow::on_loginButton_clicked()
{
    attemptLogin();
}