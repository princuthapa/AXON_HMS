#include "receptionistwindow.h"
#include "ui_receptionistwindow.h" // Generated automatically by Qt
#include <QDebug>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QDir>
#include <QFileInfo>
ReceptionistWindow::ReceptionistWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ReceptionistWindow)
{
    ui->setupUi(this);
        this->setWindowTitle("AXON-HMS: Receptionist Dashboard");

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
    connect(ui->btnClearForm, &QPushButton::clicked, this, &ReceptionistWindow::onClearFormClicked);
    connect(ui->btnSubmitRegistration, &QPushButton::clicked, this, &ReceptionistWindow::onSubmitRegistrationClicked);
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
    ui->widgetstackedtogether->setCurrentIndex(0);
}

void ReceptionistWindow::onRegisterPatientClicked()
{
    qDebug() << "Switching to Patient Registration Form (Page Index 1)";
    ui->widgetstackedtogether->setCurrentIndex(1);
}

void ReceptionistWindow::onScheduleClicked()
{
    qDebug() << "Switching to Master Schedule (Page Index 2)";
    ui->widgetstackedtogether->setCurrentIndex(2);
}

void ReceptionistWindow::onBillingClicked()
{
    qDebug() << "Switching to Billing Module (Page Index 3)";
    ui->widgetstackedtogether->setCurrentIndex(3);
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
void ReceptionistWindow::onClearFormClicked()
{
    ui->namebox->clear();
    ui->agebar->clear();
    ui->phonebox->clear();
    ui->addressbox->clear();

    // Resets dropdowns to their first option (index 0)
    ui->genderbox->setCurrentIndex(0);
    ui->doctorbox->setCurrentIndex(0);
    ui->departmentbox->setCurrentIndex(0);
}
void ReceptionistWindow::onSubmitRegistrationClicked()
{
    // 1. GRAB ALL DATA FROM THE FORM
    QString name = ui->namebox->text();
    QString age = QString::number(ui->agebar->value());
    QString gender = ui->genderbox->currentText();
    QString phone = ui->phonebox->text();
    QString address = ui->addressbox->text();
    QString doctor = ui->doctorbox->currentText();
    QString dept = ui->departmentbox->currentText();

    if (name.isEmpty()) return; // Don't register empty names

    // Generate a dummy Patient ID
    int nextId = 1280 + ui->listRecentPatientsReg->count();
    QString patientID = "P-" + QString::number(nextId);

    // 2. CLEAN AND SAVE TO CSV DATABASE
    name.replace(",", " ");
    address.replace(",", " ");

    QFile file("patients_database.csv");
    bool isNewFile = !file.exists();

    if (file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&file);

        if (isNewFile) {
            out << "PatientID,Name,Age,Gender,Phone,Address,AssignedDoctor,Department\n";
        }

        out << patientID << "," << name << "," << age << "," << gender << ","
            << phone << "," << address << "," << doctor << "," << dept << "\n";

        file.close(); // File is saved silently here!
    } else {
        // We keep the error message just in case the file gets locked by Excel
        QMessageBox::critical(this, "Database Error", "Could not save patient to CSV file!");
        return;
    }

    // 3. BUILD THE CUSTOM LIST WIDGET UI (The Circle Avatar & Badges)
    QString initials = "";
    QStringList nameParts = name.split(" ", Qt::SkipEmptyParts);
    if (nameParts.size() > 0) initials += nameParts[0].left(1).toUpper();
    if (nameParts.size() > 1) initials += nameParts[1].left(1).toUpper();

    QWidget *customItem = new QWidget();
    QHBoxLayout *mainLayout = new QHBoxLayout(customItem);
    mainLayout->setContentsMargins(5, 5, 5, 5);

    QLabel *avatar = new QLabel(initials);
    avatar->setFixedSize(36, 36);
    avatar->setAlignment(Qt::AlignCenter);
    avatar->setStyleSheet("background-color: #2b4c7e; color: white; border-radius: 18px; font-weight: bold; font-size: 14px;");

    QVBoxLayout *middleLayout = new QVBoxLayout();
    QLabel *nameLabel = new QLabel(name);
    nameLabel->setStyleSheet("font-weight: bold; font-size: 14px; color: #e0e0e0;");

    QLabel *detailsLabel = new QLabel("Age " + age + " • " + gender + " • " + dept);
    detailsLabel->setStyleSheet("color: #aaaaaa; font-size: 11px;");

    middleLayout->addWidget(nameLabel);
    middleLayout->addWidget(detailsLabel);
    middleLayout->setAlignment(Qt::AlignVCenter);

    QVBoxLayout *rightLayout = new QVBoxLayout();
    QLabel *idLabel = new QLabel(patientID);
    idLabel->setStyleSheet("color: #aaaaaa; font-size: 11px;");
    idLabel->setAlignment(Qt::AlignRight);

    QLabel *statusBadge = new QLabel("Checked in");
    statusBadge->setStyleSheet("background-color: rgba(76, 175, 80, 0.2); color: #4caf50; border-radius: 10px; padding: 2px 8px; font-size: 11px;");
    statusBadge->setAlignment(Qt::AlignCenter);

    rightLayout->addWidget(idLabel);
    rightLayout->addWidget(statusBadge);
    rightLayout->setAlignment(Qt::AlignVCenter | Qt::AlignRight);

    mainLayout->addWidget(avatar);
    mainLayout->addSpacing(10);
    mainLayout->addLayout(middleLayout);
    mainLayout->addStretch();
    mainLayout->addLayout(rightLayout);
    customItem->setLayout(mainLayout);

    // 4. ADD TO THE REGISTRATION LIST
    QListWidgetItem *listItem = new QListWidgetItem(ui->listRecentPatientsReg);
    listItem->setSizeHint(QSize(0, 60));
    ui->listRecentPatientsReg->addItem(listItem);
    ui->listRecentPatientsReg->setItemWidget(listItem, customItem);

    // 5. CLEAR FORM
    onClearFormClicked();
}