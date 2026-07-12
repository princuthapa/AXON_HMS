#include "receptionistwindow.h"
#include "mainwindow.h"
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
#include <QMenu>
#include <QAction>
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
    // 1. Create the menu
    QMenu *profileMenu = new QMenu(this);

    // NOTE: This line is crucial for rounded corners on Windows so you don't get black boxes in the corners
    profileMenu->setAttribute(Qt::WA_TranslucentBackground);

    // 2. Apply the exact visual styling from the screenshot
    profileMenu->setStyleSheet(
        "QMenu {"
        "   background-color: white;"
        "   border: 1px solid #d3d3d3;"
        "   border-radius: 8px;"           /* Rounded corners */
        "   padding: 6px 0px;"             /* Spacing at top and bottom */
        "}"
        "QMenu::item {"
        "   color: #0055a4;"               /* The specific blue text color */
        "   padding: 8px 35px 8px 20px;"   /* Spacing around the word Logout */
        "   font-size: 14px;"
        "   background-color: transparent;"
        "}"
        "QMenu::item:selected {"
        "   background-color: #f0f0f0;"    /* Light grey when you hover over 'Logout' */
        "   border-radius: 4px;"
        "}"
        );

    // 3. Create the Action
    QAction *logoutAction = new QAction("Logout", this);
    profileMenu->addAction(logoutAction);

    // 4. Attach menu to the button from your UI
    ui->profileMenuButton->setMenu(profileMenu);
    ui->profileMenuButton->setPopupMode(QToolButton::InstantPopup);

    // 5. Connect the click event

        connect(logoutAction, &QAction::triggered, this, [=]() {
            // 1. Create and show the main login dashboard
            MainWindow *loginWindow = new MainWindow();
            loginWindow->show();

            // 2. Close this receptionist window
            this->close();
        });
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // 2. Populate Dropdowns with mock data
    ui->doctorComboBox->addItems({"Select Doctor", "Dr. A.K. Sharma", "Dr. Sarah Jenkins", "Dr. Michael Chang"});
    ui->deptComboBox->addItems({"Select Department", "Cardiology", "Neurology", "Pediatrics", "General Medicine"});

    ui->timeSlotComboBox->addItems({"Select Time Slot", "09:00 AM", "09:30 AM", "10:00 AM", "10:30 AM",
                                    "11:00 AM", "11:30 AM", "02:00 PM", "02:30 PM", "03:00 PM"});

    // 3. Connect the Book Appointment button to the slot logic
    connect(ui->bookAppointmentBtn, &QPushButton::clicked, this, &ReceptionistWindow::onBookAppointmentClicked);

    // 4. Load any existing records out of your CSV file straight into the table grid
    loadAppointmentsFromCSV();
    dateTimeTimer = new QTimer(this);
    connect(dateTimeTimer, &QTimer::timeout, this, &ReceptionistWindow::updateDateTime);
    dateTimeTimer->start(1000); // Ticks every 1 second

    // Call it immediately so the label isn't blank on startup
    updateDateTime();

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
void ReceptionistWindow::updateDateTime() {
    QDateTime current = QDateTime::currentDateTime();

    // Formats precisely to: "Sat Jul 11 2026, 09:27 PM"
    QString dateText = current.toString("ddd MMM dd yyyy, hh:mm AP");

    ui->dateLabel->setText(dateText);
}
void ReceptionistWindow::onBookAppointmentClicked() {
    // 1. Extract values from UI items
    QString patient = ui->patientNameLineEdit->text().trimmed();
    QString doctor = ui->doctorComboBox->currentText();
    QString time = ui->timeSlotComboBox->currentText();
    QString dept = ui->deptComboBox->currentText();
    QString reason = ui->reasonTextEdit->toPlainText().trimmed();
    QString status = "Confirmed"; // Default booking status

    // 2. Simple validation guard rails
    if (patient.isEmpty() || doctor == "Select Doctor" || time == "Select Time Slot" || reason.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Please fill out all patient information fields completely!");
        return;
    }

    // 3. SAVE TO CSV DATABASE FILE
    QFile file("appointments.csv");
    // Open in Append mode so we add rows without erasing existing histories
    if (file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&file);
        // Standard CSV format escaping values: Patient, Doctor, Time, Status, Department, Reason
        out << "\"" << patient << "\",\""
            << doctor << "\",\""
            << time << "\",\""
            << status << "\",\""
            << dept << "\",\""
            << reason << "\"\n";
        file.close();
    } else {
        QMessageBox::critical(this, "Database Error", "Could not open data storage file for writing!");
        return;
    }

    // 4. ADD DIRECTLY TO THE LIVE TABLE VIEW
    int rowCount = ui->tableWidget->rowCount();
    ui->tableWidget->insertRow(rowCount);

    ui->tableWidget->setItem(rowCount, 0, new QTableWidgetItem(patient));
    ui->tableWidget->setItem(rowCount, 1, new QTableWidgetItem(doctor));
    ui->tableWidget->setItem(rowCount, 2, new QTableWidgetItem(time));
    ui->tableWidget->setItem(rowCount, 3, new QTableWidgetItem(status));

    // 5. Clear fields for next entry
    ui->patientNameLineEdit->clear();
    ui->reasonTextEdit->clear();
    ui->doctorComboBox->setCurrentIndex(0);
    ui->deptComboBox->setCurrentIndex(0);
    ui->timeSlotComboBox->setCurrentIndex(0);

    QMessageBox::information(this, "Success", "Appointment registered successfully!");
}

void ReceptionistWindow::loadAppointmentsFromCSV() {
    // Clear out any test design headers/rows built into the UI grid beforehand
    ui->tableWidget->setRowCount(0);

    QFile file("appointments.csv");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        // File doesn't exist yet, which is completely fine for the first launch
        return;
    }

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        // Split row parsing logic safely splitting standard text lines by commas
        // Note: Simple split by comma. Remove extra quote notations saved into string rows
        QStringList rowData = line.split(",");
        if (rowData.size() >= 4) {
            int rowCount = ui->tableWidget->rowCount();
            ui->tableWidget->insertRow(rowCount);

            QString pName = rowData[0].remove("\"");
            QString dName = rowData[1].remove("\"");
            QString tSlot = rowData[2].remove("\"");
            QString stat = rowData[3].remove("\"");

            ui->tableWidget->setItem(rowCount, 0, new QTableWidgetItem(pName));
            ui->tableWidget->setItem(rowCount, 1, new QTableWidgetItem(dName));
            ui->tableWidget->setItem(rowCount, 2, new QTableWidgetItem(tSlot));
            ui->tableWidget->setItem(rowCount, 3, new QTableWidgetItem(stat));
        }
    }
    file.close();
}