#include "receptionistwindow.h"
#include "mainwindow.h"
#include "ui_receptionistwindow.h" // Generated automatically by Qt
#include <QDebug>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QMenu>
#include <QAction>
#include <QListWidgetItem>
#include <QDate>
#include <QDateEdit>

ReceptionistWindow::ReceptionistWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ReceptionistWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("AXON-HMS: Receptionist Dashboard");

    // Shared backend managers — these read/write the SAME CSV files that
    // Admin and Doctor windows use, so every dashboard shows the same data.
    patientMgr = new PatientManager();
    staffMgr   = new StaffManager();
    apptMgr    = new AppointmentManager();

    // 1. Connect UI elements to our functions
    setupConnections();

    // 2. Populate doctor dropdowns from the real staff roster
    populateDoctorDropdowns();

    // 3. Load data onto the screen
    refreshDashboardData();
    refreshRegisteredPatientsList();

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

    // Time slot dropdown — not backed by staff/patient data, static is fine
    ui->timeSlotComboBox->addItems({"Select Time Slot", "09:00 AM", "09:30 AM", "10:00 AM", "10:30 AM",
                                    "11:00 AM", "11:30 AM", "02:00 PM", "02:30 PM", "03:00 PM"});

    // 3. Connect the Book Appointment button to the slot logic
    connect(ui->bookAppointmentBtn, &QPushButton::clicked, this, &ReceptionistWindow::onBookAppointmentClicked);

    // 4. Load any existing appointments straight into the schedule table
    refreshScheduleTable();

    dateTimeTimer = new QTimer(this);
    connect(dateTimeTimer, &QTimer::timeout, this, &ReceptionistWindow::updateDateTime);
    dateTimeTimer->start(1000); // Ticks every 1 second

    // Call it immediately so the label isn't blank on startup
    updateDateTime();

}

ReceptionistWindow::~ReceptionistWindow()
{
    delete ui;
    delete patientMgr;
    delete staffMgr;
    delete apptMgr;
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

    patientMgr->reload();
    apptMgr->reload();

    populateAppointmentsTable();
    populateRecentPatientsTable();
}



void ReceptionistWindow::onDashboardClicked()
{
    qDebug() << "Switching to Dashboard View (Page Index 0)";
    ui->widgetstackedtogether->setCurrentIndex(0);
    refreshDashboardData();
}

void ReceptionistWindow::onRegisterPatientClicked()
{
    qDebug() << "Switching to Patient Registration Form (Page Index 1)";
    ui->widgetstackedtogether->setCurrentIndex(1);
    staffMgr->reload();
    populateDoctorDropdowns();
    refreshRegisteredPatientsList();
}

void ReceptionistWindow::onScheduleClicked()
{
    qDebug() << "Switching to Master Schedule (Page Index 2)";
    ui->widgetstackedtogether->setCurrentIndex(2);
    staffMgr->reload();
    populateDoctorDropdowns();
    refreshScheduleTable();
}

void ReceptionistWindow::onBillingClicked()
{
    qDebug() << "Switching to Billing Module (Page Index 3)";
    ui->widgetstackedtogether->setCurrentIndex(3);
}

void ReceptionistWindow::onViewAllAppointmentsClicked()
{
    qDebug() << "View All Appointments clicked!";
    apptMgr->reload();
    refreshScheduleTable();
}

//Doctor dropdowns, sourced from the real staff roster

void ReceptionistWindow::populateDoctorDropdowns()
{
    QStringList doctorNames;
    for (const auto &s : staffMgr->getAllStaff()) {
        if (s.role.trimmed().compare("Doctor", Qt::CaseInsensitive) == 0)
            doctorNames << s.name;
    }

    // Assign Doctor (Patient Registration page)
    if (ui->doctorbox) {
        QString previous = ui->doctorbox->currentText();
        ui->doctorbox->clear();
        ui->doctorbox->addItems(doctorNames);
        int idx = ui->doctorbox->findText(previous);
        if (idx >= 0) ui->doctorbox->setCurrentIndex(idx);
    }

    // Book Appointment (Scheduling page)
    if (ui->doctorComboBox) {
        QString previous = ui->doctorComboBox->currentText();
        ui->doctorComboBox->clear();
        ui->doctorComboBox->addItem("Select Doctor");
        ui->doctorComboBox->addItems(doctorNames);
        int idx = ui->doctorComboBox->findText(previous);
        ui->doctorComboBox->setCurrentIndex(idx >= 0 ? idx : 0);
    }
}

//Dashboard stat widgets

void ReceptionistWindow::populateAppointmentsTable()
{
    if (!ui->appointmenttable) return;
    QVector<Appointment> todays = apptMgr->getTodaysAppointments();

    int rowCount = ui->appointmenttable->rowCount(); // row 0 is used as the visual header
    for (int r = 1; r < rowCount; ++r) {
        int dataIdx = r - 1;
        if (dataIdx < todays.size()) {
            const Appointment &a = todays.at(dataIdx);
            ui->appointmenttable->setItem(r, 0, new QTableWidgetItem(a.time));
            ui->appointmenttable->setItem(r, 1, new QTableWidgetItem(a.patientName));
            ui->appointmenttable->setItem(r, 2, new QTableWidgetItem(a.doctorName));
            ui->appointmenttable->setItem(r, 3, new QTableWidgetItem(a.status));
        } else {
            for (int c = 0; c < ui->appointmenttable->columnCount(); ++c)
                ui->appointmenttable->setItem(r, c, new QTableWidgetItem(""));
        }
    }

    if (ui->numberofappointment) ui->numberofappointment->setText(QString::number(todays.size()));
}

void ReceptionistWindow::populateRecentPatientsTable()
{
    if (!ui->registeredpatienttable) return;
    QVector<Patient> patients = patientMgr->getAllPatients();

    int rowCount = ui->registeredpatienttable->rowCount(); // row 0 is the visual header
    for (int r = 1; r < rowCount; ++r) {
        int dataIdx = patients.size() - r; // most recently added first
        if (dataIdx >= 0) {
            const Patient &p = patients.at(dataIdx);
            ui->registeredpatienttable->setItem(r, 0, new QTableWidgetItem(p.id));
            ui->registeredpatienttable->setItem(r, 1, new QTableWidgetItem(p.name));
            ui->registeredpatienttable->setItem(r, 2, new QTableWidgetItem(p.age));
            ui->registeredpatienttable->setItem(r, 3, new QTableWidgetItem(p.gender));
            ui->registeredpatienttable->setItem(r, 4, new QTableWidgetItem(p.status));
        } else {
            for (int c = 0; c < ui->registeredpatienttable->columnCount(); ++c)
                ui->registeredpatienttable->setItem(r, c, new QTableWidgetItem(""));
        }
    }

    if (ui->numberofpatient) ui->numberofpatient->setText(QString::number(patients.size()));
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
    QString name    = ui->namebox->text().trimmed();
    QString age     = QString::number(ui->agebar->value());
    QString gender  = ui->genderbox->currentText();
    QString phone   = ui->phonebox->text().trimmed();
    QString address = ui->addressbox->text().trimmed();
    QString doctor  = ui->doctorbox->currentText();
    QString dept    = ui->departmentbox->currentText();

    if (name.isEmpty()) return; // Don't register empty names

    // Sanitize commas so the CSV stays well-formed
    name.replace(",", " ");
    address.replace(",", " ");

    // 2. SAVE THROUGH THE SHARED PatientManager — this is the SAME patient_database.csv that Admin and Doctor windows read from.
    Patient newPatient;
    newPatient.id                 = patientMgr->generateNextId();
    newPatient.name               = name;
    newPatient.age                = age;
    newPatient.gender             = gender;
    newPatient.diagnosisTreatment = "Awaiting Consultation (" + dept + ")";
    newPatient.assignedDoctor     = doctor;
    newPatient.status             = "Checked In";
    newPatient.bedNumber          = "";

    patientMgr->addPatient(newPatient);

    // 3. ADD DIRECTLY TO THE LIVE LIST WIDGET
    addPatientListItem(newPatient);

    // 4. Keep dashboard stat cards in sync
    populateRecentPatientsTable();

    // 5. CLEAR FORM
    onClearFormClicked();
}

void ReceptionistWindow::refreshRegisteredPatientsList()
{
    ui->listRecentPatientsReg->clear();
    QVector<Patient> patients = patientMgr->getAllPatients();
    // Show the most recently registered patients first, most-recent last N is fine too;
    // here we simply list everyone so the receptionist can see the full live roster.
    for (const auto &p : patients) {
        addPatientListItem(p);
    }
}

void ReceptionistWindow::addPatientListItem(const Patient &p)
{
    QWidget *customItem = new QWidget();
    QHBoxLayout *mainLayout = new QHBoxLayout(customItem);
    mainLayout->setContentsMargins(5, 5, 5, 5);


    QVBoxLayout *middleLayout = new QVBoxLayout();
    QLabel *nameLabel = new QLabel(p.name);
    nameLabel->setStyleSheet("font-weight: bold; font-size: 14px; color: #000000;");

    QLabel *detailsLabel = new QLabel("Age " + p.age + " • " + p.gender + " • Dr. " + p.assignedDoctor);
    detailsLabel->setStyleSheet("color: #aaaaaa; font-size: 11px;");

    middleLayout->addWidget(nameLabel);
    middleLayout->addWidget(detailsLabel);
    middleLayout->setAlignment(Qt::AlignVCenter);

    QVBoxLayout *rightLayout = new QVBoxLayout();
    QLabel *idLabel = new QLabel(p.id);
    idLabel->setStyleSheet("color: #aaaaaa; font-size: 11px;");
    idLabel->setAlignment(Qt::AlignRight);

    QLabel *statusBadge = new QLabel(p.status);
    statusBadge->setStyleSheet(" color: #4caf50; border-radius: 10px; padding: 2px 8px; font-size: 11px;");
    statusBadge->setAlignment(Qt::AlignCenter);

    rightLayout->addWidget(idLabel);
    rightLayout->addWidget(statusBadge);
    rightLayout->setAlignment(Qt::AlignVCenter | Qt::AlignRight);


    mainLayout->addLayout(middleLayout);
    mainLayout->addStretch();
    mainLayout->addLayout(rightLayout);
    customItem->setLayout(mainLayout);

    QListWidgetItem *listItem = new QListWidgetItem(ui->listRecentPatientsReg);
    listItem->setSizeHint(QSize(0, 60));
    ui->listRecentPatientsReg->addItem(listItem);
    ui->listRecentPatientsReg->setItemWidget(listItem, customItem);
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
    QString doctor  = ui->doctorComboBox->currentText();
    QString time    = ui->timeSlotComboBox->currentText();
    QString dept    = ui->deptComboBox->currentText();
    QString reason  = ui->reasonTextEdit->toPlainText().trimmed();
    QString date    = ui->Datee ? ui->Datee->date().toString("yyyy-MM-dd")
                                 : QDate::currentDate().toString("yyyy-MM-dd");
    QString status  = "Confirmed"; // Default booking status

    // 2. Simple validation guard rails
    if (patient.isEmpty() || doctor == "Select Doctor" || time == "Select Time Slot" || reason.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Please fill out all patient information fields completely!");
        return;
    }

    // Sanitize commas so the CSV stays well-formed
    patient.replace(",", " ");
    reason.replace(",", " ");

    // 3. SAVE THROUGH THE SHARED AppointmentManager — same
    //    appointment_database.csv that Admin and Doctor windows read from.
    Appointment a;
    a.id          = apptMgr->generateNextId();
    a.patientName = patient;
    a.doctorName  = doctor;
    a.department  = dept;
    a.date        = date;
    a.time        = time;
    a.reason      = reason;
    a.status      = status;
    apptMgr->addAppointment(a);

    // 4. REFRESH THE LIVE TABLE VIEW FROM THE BACKEND
    refreshScheduleTable();

    // 5. Clear fields for next entry
    ui->patientNameLineEdit->clear();
    ui->reasonTextEdit->clear();
    ui->doctorComboBox->setCurrentIndex(0);
    ui->deptComboBox->setCurrentIndex(0);
    ui->timeSlotComboBox->setCurrentIndex(0);

    QMessageBox::information(this, "Success", "Appointment registered successfully!");
}

void ReceptionistWindow::refreshScheduleTable() {
    ui->tableWidget->setRowCount(0);

    for (const auto &a : apptMgr->getAllAppointments()) {
        int rowCount = ui->tableWidget->rowCount();
        ui->tableWidget->insertRow(rowCount);
        ui->tableWidget->setItem(rowCount, 0, new QTableWidgetItem(a.patientName));
        ui->tableWidget->setItem(rowCount, 1, new QTableWidgetItem(a.doctorName));
        ui->tableWidget->setItem(rowCount, 2, new QTableWidgetItem(a.time));
        ui->tableWidget->setItem(rowCount, 3, new QTableWidgetItem(a.status));
    }
}
