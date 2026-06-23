#include "receptionistwindow.h"
#include "ui_receptionistwindow.h" // Generated automatically by Qt
#include <QDebug>

ReceptionistWindow::ReceptionistWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ReceptionistWindow)
{
    ui->setupUi(this);

    // 1. Connect UI elements to our functions
    setupConnections();

    // 2. Load data onto the screen
    refreshDashboardData();
}

ReceptionistWindow::~ReceptionistWindow()
{
    delete ui;
}

void ReceptionistWindow::setupConnections()
{
    // Modern Qt5/Qt6 compile-time connection syntax.
    connect(ui->btnDashboard, &QPushButton::clicked, this, &ReceptionistWindow::onDashboardClicked);
    connect(ui->btnRegisterPatient, &QPushButton::clicked, this, &ReceptionistWindow::onRegisterPatientClicked);
    connect(ui->btnSchedule, &QPushButton::clicked, this, &ReceptionistWindow::onScheduleClicked);
    connect(ui->btnBilling, &QPushButton::clicked, this, &ReceptionistWindow::onBillingClicked);

    connect(ui->btnViewAllAppointments, &QPushButton::clicked, this, &ReceptionistWindow::onViewAllAppointmentsClicked);
}

void ReceptionistWindow::refreshDashboardData()
{
    // Triggers the data fetching functions
    populateAppointmentsTable();
    populateRecentPatientsTable();
}

// --- Sidebar Actions (Page Navigation) ---

void ReceptionistWindow::onDashboardClicked()
{
    qDebug() << "Switching to Dashboard View (Page Index 0)";
    ui->stackedWidget->setCurrentIndex(0);
}

void ReceptionistWindow::onRegisterPatientClicked()
{
    qDebug() << "Switching to Patient Registration Form (Page Index 1)";
    ui->stackedWidget->setCurrentIndex(1);
}

void ReceptionistWindow::onScheduleClicked()
{
    qDebug() << "Switching to Master Schedule (Page Index 2)";
    ui->stackedWidget->setCurrentIndex(2);
}

void ReceptionistWindow::onBillingClicked()
{
    qDebug() << "Switching to Billing Module (Page Index 3)";
    ui->stackedWidget->setCurrentIndex(3);
}

void ReceptionistWindow::onViewAllAppointmentsClicked()
{
    qDebug() << "View All Appointments clicked!";
}

// --- Data Connections for PatientManager ---

void ReceptionistWindow::populateAppointmentsTable()
{
    // Left empty for now so it doesn't overwrite your visual UI design.
    // TODO: Pull live data here from PatientManager later, e.g.:
    // ui->tableAppointments->setText(patientManager->getAppointmentsText());
}

void ReceptionistWindow::populateRecentPatientsTable()
{
    // Left empty for now so it doesn't overwrite your visual UI design.
    // TODO: Pull live data here from PatientManager later, e.g.:
    // ui->tableRecentPatients->setText(patientManager->getRecentPatientsText());
}