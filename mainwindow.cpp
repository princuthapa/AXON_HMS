#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QIcon>
#include <QDebug>
#include <QMessageBox> // Required for the popup box
#include <QMouseEvent>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->usernamelineEdit, &QLineEdit::returnPressed, this, [this]() {
        ui->passwordlineEdit->setFocus();
    });

    connect(ui->passwordlineEdit, &QLineEdit::returnPressed, this, &MainWindow::handleLogin);
    connect(ui->loginButton, &QPushButton::clicked, this, &MainWindow::handleLogin);

    eyeAction = new QAction(this);
    eyeAction->setIcon(QIcon(":/eye/ihide.png"));
    ui->passwordlineEdit->addAction(eyeAction, QLineEdit::TrailingPosition);

    connect(eyeAction, &QAction::triggered, this, &MainWindow::togglePasswordVisibility);

    // Make the forgot password label clickable like a link
    ui->forgotPasswordLabel->setCursor(Qt::PointingHandCursor);
    ui->forgotPasswordLabel->setStyleSheet("QLabel { color: #0056b3; text-decoration: underline; } "
                                           "QLabel:hover { color: #003366; }");

    // Connect the label's click event to our new popup slot
    ui->forgotPasswordLabel->installEventFilter(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

// Intercepts the click event on the label widget cleanly
bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->forgotPasswordLabel && event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            handleForgotPassword();
            return true;
        }
    }
    return QMainWindow::eventFilter(obj, event);
}

void MainWindow::togglePasswordVisibility()
{
    if (ui->passwordlineEdit->echoMode() == QLineEdit::Password) {
        ui->passwordlineEdit->setEchoMode(QLineEdit::Normal);
        eyeAction->setIcon(QIcon(":/eye/ishow.png"));
    } else {
        ui->passwordlineEdit->setEchoMode(QLineEdit::Password);
        eyeAction->setIcon(QIcon(":/eye/ihide.png"));
    }
}

void MainWindow::handleForgotPassword()
{
    // Create the message box instance
    QMessageBox msgBox(this);

    // Set up the window title and your exact text message
    msgBox.setWindowTitle("Reset Password");
    msgBox.setText("Contact to administration");
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setStandardButtons(QMessageBox::Ok);

    // Force the text to display in black color so it bypasses global styling conflicts
    msgBox.setStyleSheet("QLabel { color: #000000; font-size: 14px; } "
                         "QPushButton { background-color: #000000; color: #ffffff; border-radius: 10px; padding: 5px 15px; }");

    // Execute and show the popup box
    msgBox.exec();
}
void MainWindow::handleLogin()
{
    QString username = ui->usernamelineEdit->text();
    QString password = ui->passwordlineEdit->text();
    QString role = ui->comboBox->currentText();

    if (!username.isEmpty() && !password.isEmpty()) {
        qDebug() << "Success! Opening Dashboard for role:" << role;
    } else {
        qDebug() << "Error: Fields cannot be blank!";
    }
}