#include "mainwindow.h"
#include "adminwindow.h"
#include "ui_adminwindow.h"
#include <QDesktopServices>
#include <QUrl>
#include <QDateTime>
#include <QTimer>
#include <QMenu>
#include <QAction>

adminwindow::adminwindow(const QString &employeeName, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Form),// Connects perfectly to your form!
    currentAdminName(employeeName)
{
    ui->setupUi(this);

    adminBackend = nullptr;
    staffMgr = nullptr;

    // 1. Initialize Stacked Widget
    if (ui->stackedWidget) {
        ui->stackedWidget->setCurrentIndex(0);
    }

    // 2. Real-Time Date & Time Clock
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &adminwindow::updateDateTime);
    timer->start(1000); // Ticks every 1000ms (1 second)
    updateDateTime();   // Initial display call

    // 3. Load AXON Logo into Sidebar Label
    // Fixed path to match your resources.qrc (/images prefix)
    QPixmap logoPixmap(":/images/AXON_Monogram.png");
    if (ui->lblLogo && !logoPixmap.isNull()) {
        //ui->lblLogo->setPixmap(logoPixmap.scaled(70, 70, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
}

adminwindow::~adminwindow()
{
    delete ui;
    if (adminBackend) delete adminBackend;
    if (staffMgr) delete staffMgr;
}

// Updates clock dynamically
void adminwindow::updateDateTime()
{
    if (ui->lblDateTime) {
        QDateTime current = QDateTime::currentDateTime();
        // Formats cleanly as: Mon Jun 20 2026, 03:14 PM
        QString timeStr = current.toString("ddd MMM d yyyy, hh:mm AP");

        // FIX: Changed '::' to '->' to fix compile error C2510
        ui->lblDateTime->setText(timeStr);
    }
}

// 3-Dots Menu Dropdown and Logout Controller
#include <QMenu>
#include <QAction>

void adminwindow::on_btnMenu_clicked()
{
    // 1. Create the popup context menu
    QMenu menu(this);

    // 2. Style the dropdown menu to match your premium HMS theme
    menu.setStyleSheet(
        "QMenu {"
        "    background-color: #ffffff;"
        "    border: 1px solid #e2e8f0;"
        "    border-radius: 8px;"
        "    padding: 4px 0px;"
        "}"
        "QMenu::item {"
        "    padding: 8px 24px;"
        "    font-family: 'Segoe UI', sans-serif;"
        "    font-size: 14px;"
        "    color: #475569;"
        "}"
        "QMenu::item:selected {"
        "    background-color: #fee2e2;" /* Soft red accent for logout hover */
        "    color: #991b1b;"             /* Dark red text on hover */
        "}"
        );

    // 3. Add the Logout action item to the menu
    QAction *logoutAction = menu.addAction("Logout");

    // 4. Pop up the menu directly beneath the 3-bars button
    // mapToGlobal ensures it displays right under the button on your screen
    QPoint popupPos = ui->btnMenu->mapToGlobal(QPoint(0, ui->btnMenu->height()));
    QAction *clickedAction = menu.exec(popupPos);

    // 5. Check if the user actually clicked the "Logout" option
    if (clickedAction == logoutAction) {
        MainWindow *loginScreen = new MainWindow();
        loginScreen->show();
        this->close();
    }
}

// ---- UI Navigation Slots ----
void adminwindow::on_btnOverview_clicked()
{
    if (ui->stackedWidget) ui->stackedWidget->setCurrentIndex(0);
}

void adminwindow::on_btnManageStaff_clicked()
{
    if (ui->stackedWidget) ui->stackedWidget->setCurrentIndex(1);
}

void adminwindow::on_btnScheduling_clicked()
{
    if (ui->stackedWidget) ui->stackedWidget->setCurrentIndex(2);

    // Quick Launch helper for Google Calendar integration
    QDesktopServices::openUrl(QUrl("https://calendar.google.com/"));
}
