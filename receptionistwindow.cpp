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
#include <QRadioButton>
#include <QFont>
#include <QTableWidgetItem>
#include <QStyledItemDelegate>
#include <QKeyEvent>


namespace {

class AmountColumnDelegate : public QStyledItemDelegate
{
public:
    explicit AmountColumnDelegate(QTableWidget *table, QObject *parent = nullptr)
        : QStyledItemDelegate(parent), m_table(table) {}

protected:
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override
    {
        m_editingIndex = index;
        return QStyledItemDelegate::createEditor(parent, option, index);
    }

    bool eventFilter(QObject *editor, QEvent *event) override
    {
        if (event->type() == QEvent::KeyPress) {
            auto *keyEvent = static_cast<QKeyEvent *>(event);
            const int key = keyEvent->key();

            if (key == Qt::Key_Down || key == Qt::Key_Up ||
                key == Qt::Key_Return || key == Qt::Key_Enter) {

                QWidget *editorWidget = qobject_cast<QWidget *>(editor);
                if (editorWidget) {
                    emit commitData(editorWidget);
                    emit closeEditor(editorWidget, QAbstractItemDelegate::NoHint);
                }

                if (m_table && m_editingIndex.isValid()) {
                    const int row = m_editingIndex.row();
                    const int col = m_editingIndex.column();
                    const int newRow = (key == Qt::Key_Up) ? row - 1 : row + 1;

                    if (newRow >= 0 && newRow < m_table->rowCount()) {
                        m_table->setCurrentCell(newRow, col);
                        if (m_table->model())
                            m_table->edit(m_table->model()->index(newRow, col));
                    }
                }
                return true;
            }
        }
        return QStyledItemDelegate::eventFilter(editor, event);
    }

private:
    QTableWidget *m_table;
    mutable QModelIndex m_editingIndex;
};

//Standard fee-schedule template
// The four service rows that always appear in "Current Bill Summary" —
// Service and Description are fixed; only the Amount cell is editable.
struct ServiceTemplateRow { QString code; QString description; };

const QVector<ServiceTemplateRow> kServiceTemplate = {
    {"Service Charge",      "Service Fees"},
    {"Nursing Care",      "Routine nursing care"},
    {"Lab",                "Laboratory / diagnostic services"},
    {"Pharmacy",           "Pharmacy / medication charges"},
    };

const int kServiceRowCount = static_cast<int>(kServiceTemplate.size());
const int kRowSubtotal     = kServiceRowCount;      // row 4
const int kRowDeposit      = kServiceRowCount + 1;  // row 5
const int kRowRemaining    = kServiceRowCount + 2;  // row 6

} // namespace

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
    billingMgr = new BillingManager();

    // 1. Connect UI elements to our functions
    setupConnections();

    // 2. Populate doctor dropdowns from the real staff roster
    populateDoctorDropdowns();

    // 3. Load data onto the screen
    refreshDashboardData();
    refreshRegisteredPatientsList();
    clearBillingPatientCard();

    // 4. Billing table: keyboard navigation + live totals + pre-filled rows
    ui->billingSummaryTable->setItemDelegateForColumn(
        2, new AmountColumnDelegate(ui->billingSummaryTable, this));
    connect(ui->billingSummaryTable, &QTableWidget::itemChanged,
            this, &ReceptionistWindow::recomputeBillTotals);
    connect(ui->billingDiscountEdit, &QLineEdit::textChanged,
            this, [this](const QString &) { recomputeBillTotals(); });
    populateBillingServiceTemplate();

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
    delete billingMgr;
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

    // Billing page
    connect(ui->billingSearchBtn, &QPushButton::clicked, this, &ReceptionistWindow::onBillingSearchClicked);
    connect(ui->btnGenerateBill, &QPushButton::clicked, this, &ReceptionistWindow::onGenerateBillClicked);
    connect(ui->btnProcessPayment, &QPushButton::clicked, this, &ReceptionistWindow::onProcessPaymentClicked);
}

void ReceptionistWindow::refreshDashboardData()
{
    // Re-read the shared CSVs so this window reflects whatever Admin/Doctor
    // most recently saved, then repaint the dashboard widgets.
    patientMgr->reload();
    apptMgr->reload();

    populateAppointmentsTable();
    populateRecentPatientsTable();
}

// --- Sidebar Actions (Page Navigation) ---

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
    patientMgr->reload();
    billingMgr->reload();

    // Fresh slate each time the receptionist opens the Billing tab — table
    // is already filled with the standard service rows, amounts at 0.00.
    currentBillingPatientId.clear();
    currentBillId.clear();
    clearBillingPatientCard();
    ui->billingSearchEdit->clear();
    ui->billingDiscountEdit->clear();
    ui->billingNotesEdit->clear();
    populateBillingServiceTemplate();
}

void ReceptionistWindow::onViewAllAppointmentsClicked()
{
    qDebug() << "View All Appointments clicked!";
    apptMgr->reload();
    refreshScheduleTable();
}

// --- Doctor dropdowns, sourced from the real staff roster ---

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

// --- Dashboard stat widgets ---

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

    // 2. SAVE THROUGH THE SHARED PatientManager — this is the SAME
    //    patient_database.csv that Admin and Doctor windows read from.
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

    QString initials = "";
    QStringList nameParts = p.name.split(" ", Qt::SkipEmptyParts);
    if (nameParts.size() > 0) initials += nameParts[0].left(1).toUpper();
    if (nameParts.size() > 1) initials += nameParts[1].left(1).toUpper();

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
    statusBadge->setStyleSheet("color: #4caf50; border-radius: 10px; padding: 2px 8px; font-size: 11px;");
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

// ── Billing page ─────────────────────────────────────────────────────────

QString ReceptionistWindow::selectedPaymentMode() const
{
    if (ui->billingModeCard->isChecked())      return "Card";
    if (ui->billingModeOnline->isChecked())    return "Online";
    if (ui->billingModeInsurance->isChecked()) return "Insurance";
    return "Cash"; // default / ui->billingModeCash checked
}

void ReceptionistWindow::populateBillingPatientCard(const Patient &p)
{
    QString initials;
    QStringList parts = p.name.split(" ", Qt::SkipEmptyParts);
    if (parts.size() > 0) initials += parts[0].left(1).toUpper();
    if (parts.size() > 1) initials += parts[1].left(1).toUpper();


    ui->billingPatientIdLabel->setText("Patient ID: " + p.id);
    ui->billingPatientNameLabel->setText("Name: " + p.name);
    ui->billingPatientAgeLabel->setText("Age: " + p.age);
    ui->billingPatientGenderLabel->setText("Gender: " + p.gender);
}

void ReceptionistWindow::clearBillingPatientCard()
{

    ui->billingPatientIdLabel->setText("Patient ID: —");
    ui->billingPatientNameLabel->setText("Name: —");
    ui->billingPatientAgeLabel->setText("Age: —");
    ui->billingPatientGenderLabel->setText("Gender: —");
}

// Resets "Current Bill Summary" to the standard fee-schedule rows: Service
// and Description are pre-filled and locked; only the Amount cell (column 2)
// is editable. Subtotal / Deposit / Remaining Balance rows are appended
// below — Subtotal and Remaining Balance are computed and locked, Deposit
// stays editable so the receptionist can record an upfront deposit inline.
void ReceptionistWindow::populateBillingServiceTemplate()
{
    QTableWidget *t = ui->billingSummaryTable;
    t->blockSignals(true);
    t->setRowCount(0);

    auto addServiceRow = [&](const QString &code, const QString &desc) {
        int row = t->rowCount();
        t->insertRow(row);

        QTableWidgetItem *serviceItem = new QTableWidgetItem(code);
        serviceItem->setFlags(serviceItem->flags() & ~Qt::ItemIsEditable);
        QTableWidgetItem *descItem = new QTableWidgetItem(desc);
        descItem->setFlags(descItem->flags() & ~Qt::ItemIsEditable);
        QTableWidgetItem *amountItem = new QTableWidgetItem("0.00");
        amountItem->setFlags(amountItem->flags() | Qt::ItemIsEditable);
        amountItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);

        t->setItem(row, 0, serviceItem);
        t->setItem(row, 1, descItem);
        t->setItem(row, 2, amountItem);
    };

    for (const auto &s : kServiceTemplate)
        addServiceRow(s.code, s.description);

    auto addSummaryRow = [&](const QString &label, bool amountEditable) {
        int row = t->rowCount();
        t->insertRow(row);

        QTableWidgetItem *labelItem = new QTableWidgetItem(label);
        labelItem->setFlags(labelItem->flags() & ~Qt::ItemIsEditable);
        QFont boldFont = labelItem->font();
        boldFont.setBold(true);
        labelItem->setFont(boldFont);

        QTableWidgetItem *blankItem = new QTableWidgetItem("");
        blankItem->setFlags(blankItem->flags() & ~Qt::ItemIsEditable);

        QTableWidgetItem *amountItem = new QTableWidgetItem("0.00");
        amountItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        if (amountEditable) {
            amountItem->setFlags(amountItem->flags() | Qt::ItemIsEditable);
        } else {
            amountItem->setFlags(amountItem->flags() & ~Qt::ItemIsEditable);
            amountItem->setFont(boldFont);
        }

        t->setItem(row, 0, labelItem);
        t->setItem(row, 1, blankItem);
        t->setItem(row, 2, amountItem);
    };

    addSummaryRow("Subtotal", false);
    addSummaryRow("Deposit", true);
    addSummaryRow("Remaining Balance", false);

    t->blockSignals(false);
}

// Fills a previously-generated bill's amounts into the template built by
// populateBillingServiceTemplate(). Matches items back to template rows by
// service code; any legacy item that doesn't match a known service is
// simply skipped (this app's earlier billing flow used a single generic
// "Service Charge" line, which predates this fixed fee-schedule table).
void ReceptionistWindow::loadBillIntoTable(const BillingRecord &bill)
{
    populateBillingServiceTemplate();

    QTableWidget *t = ui->billingSummaryTable;
    t->blockSignals(true);

    for (const auto &item : bill.items) {
        for (int r = 0; r < kServiceRowCount; ++r) {
            if (kServiceTemplate[r].code.compare(item.serviceCode, Qt::CaseInsensitive) == 0) {
                t->item(r, 2)->setText(QString::number(item.amount, 'f', 2));
                break;
            }
        }
    }
    t->item(kRowDeposit, 2)->setText(QString::number(bill.depositPaid, 'f', 2));

    t->blockSignals(false);

    ui->billingDiscountEdit->blockSignals(true);
    ui->billingDiscountEdit->setText(QString::number(bill.discount, 'f', 2));
    ui->billingDiscountEdit->blockSignals(false);

    // Show the bill's actual stored Subtotal / Remaining Balance (which may
    // already reflect prior payments processed against it) rather than a
    // freshly recomputed value.
    t->blockSignals(true);
    t->item(kRowSubtotal, 2)->setText(QString::number(bill.subtotal, 'f', 2));
    t->item(kRowRemaining, 2)->setText(QString::number(bill.remainingBalance, 'f', 2));
    t->blockSignals(false);
}

// Re-sums Subtotal (service rows) and Remaining Balance (Subtotal - Deposit
// - Discount, floored at 0) whenever an editable Amount cell or the Discount
// field changes. Guarded against re-entrancy since this method itself edits
// table cells, which would otherwise re-trigger itemChanged.
void ReceptionistWindow::recomputeBillTotals()
{
    if (m_updatingBillTable) return;

    QTableWidget *t = ui->billingSummaryTable;
    if (t->rowCount() < kServiceRowCount + 3) return; // template not built yet

    m_updatingBillTable = true;
    t->blockSignals(true);

    double subtotal = 0.0;
    for (int r = 0; r < kServiceRowCount; ++r) {
        if (QTableWidgetItem *it = t->item(r, 2))
            subtotal += it->text().trimmed().toDouble();
    }

    double deposit = 0.0;
    if (QTableWidgetItem *dep = t->item(kRowDeposit, 2))
        deposit = dep->text().trimmed().toDouble();

    double discount = ui->billingDiscountEdit->text().trimmed().toDouble();

    double remaining = subtotal - deposit - discount;
    if (remaining < 0.0) remaining = 0.0;

    if (QTableWidgetItem *sub = t->item(kRowSubtotal, 2))
        sub->setText(QString::number(subtotal, 'f', 2));
    if (QTableWidgetItem *rem = t->item(kRowRemaining, 2))
        rem->setText(QString::number(remaining, 'f', 2));

    t->blockSignals(false);
    m_updatingBillTable = false;
}

void ReceptionistWindow::onBillingSearchClicked()
{
    QString query = ui->billingSearchEdit->text().trimmed();
    if (query.isEmpty()) {
        QMessageBox::information(this, "Search", "Enter a patient name or ID to search.");
        return;
    }

    patientMgr->reload();
    billingMgr->reload();

    // Match by exact ID first, then fall back to a case-insensitive name search.
    Patient found;
    bool matched = false;
    for (const auto &p : patientMgr->getAllPatients()) {
        if (p.id.compare(query, Qt::CaseInsensitive) == 0) {
            found = p; matched = true; break;
        }
    }
    if (!matched) {
        for (const auto &p : patientMgr->getAllPatients()) {
            if (p.name.contains(query, Qt::CaseInsensitive)) {
                found = p; matched = true; break;
            }
        }
    }

    if (!matched) {
        QMessageBox::warning(this, "Not Found", "No patient matches \"" + query + "\".");
        clearBillingPatientCard();
        populateBillingServiceTemplate();
        currentBillingPatientId.clear();
        currentBillId.clear();
        return;
    }

    currentBillingPatientId = found.id;
    populateBillingPatientCard(found);

    BillingRecord latest = billingMgr->getLatestBillForPatient(found.id);
    if (latest.billId.isEmpty()) {
        // No bill on file yet — start from a clean, pre-filled template.
        currentBillId.clear();
        ui->billingDiscountEdit->clear();
        populateBillingServiceTemplate();
    } else {
        currentBillId = latest.billId;
        loadBillIntoTable(latest);
    }
}

void ReceptionistWindow::onGenerateBillClicked()
{
    if (currentBillingPatientId.isEmpty()) {
        QMessageBox::warning(this, "No Patient Selected", "Search for a patient before generating a bill.");
        return;
    }

    QTableWidget *t = ui->billingSummaryTable;

    QVector<BillItem> items;
    for (int r = 0; r < kServiceRowCount; ++r) {
        QTableWidgetItem *it = t->item(r, 2);
        double amount = it ? it->text().trimmed().toDouble() : 0.0;
        if (amount > 0.0) {
            BillItem bi;
            bi.serviceCode = kServiceTemplate[r].code;
            bi.description = kServiceTemplate[r].description;
            bi.amount      = amount;
            items.append(bi);
        }
    }

    if (items.isEmpty()) {
        QMessageBox::warning(this, "No Charges Entered",
                             "Enter an amount for at least one service in the Current Bill Summary table before generating a bill.");
        return;
    }

    double deposit = 0.0;
    if (QTableWidgetItem *dep = t->item(kRowDeposit, 2))
        deposit = dep->text().trimmed().toDouble();

    double discount = ui->billingDiscountEdit->text().trimmed().toDouble();
    QString notes = ui->billingNotesEdit->text().trimmed();

    QString newBillId = billingMgr->generateBill(currentBillingPatientId, items, deposit, discount, notes);
    currentBillId = newBillId;

    BillingRecord newBill = billingMgr->searchBill(newBillId);

    // Reflect the authoritative stored Subtotal / Remaining Balance back
    // into the table (should already match what recomputeBillTotals showed).
    t->blockSignals(true);
    t->item(kRowSubtotal, 2)->setText(QString::number(newBill.subtotal, 'f', 2));
    t->item(kRowRemaining, 2)->setText(QString::number(newBill.remainingBalance, 'f', 2));
    t->blockSignals(false);

    ui->billingNotesEdit->clear();

    Patient p = patientMgr->searchPatient(currentBillingPatientId);
    QMessageBox::information(this, "Bill Generated",
                             QString("Bill %1 generated for %2.\nRemaining Balance: $%3")
                                 .arg(newBillId, p.name, QString::number(newBill.remainingBalance, 'f', 2)));
}

void ReceptionistWindow::onProcessPaymentClicked()
{
    if (currentBillId.isEmpty()) {
        QMessageBox::warning(this, "No Bill Selected",
                             "Search for a patient with an existing bill, or generate a new bill, before processing a payment.");
        return;
    }

    bool okAmount = false;
    double paymentAmount = ui->billingAmountToPayEdit->text().trimmed().toDouble(&okAmount);
    if (!okAmount || paymentAmount <= 0.0) {
        QMessageBox::warning(this, "Invalid Amount", "Enter a valid amount in \"Amount to Pay\" before processing payment.");
        return;
    }

    QString mode = selectedPaymentMode();
    QString notes = ui->billingNotesEdit->text().trimmed();

    if (!billingMgr->processPayment(currentBillId, paymentAmount, mode, notes)) {
        QMessageBox::critical(this, "Payment Failed", "Could not find bill " + currentBillId + " to apply payment.");
        return;
    }

    BillingRecord updated = billingMgr->searchBill(currentBillId);

    QTableWidget *t = ui->billingSummaryTable;
    t->blockSignals(true);
    t->item(kRowRemaining, 2)->setText(QString::number(updated.remainingBalance, 'f', 2));
    t->blockSignals(false);

    ui->billingAmountToPayEdit->clear();
    ui->billingCardExpiryEdit->clear();
    ui->billingCVCEdit->clear();
    ui->billingNotesEdit->clear();

    QMessageBox::information(this, "Payment Processed",
                             QString("Payment of $%1 (%2) applied to bill %3.\nRemaining Balance: $%4")
                                 .arg(QString::number(paymentAmount, 'f', 2), mode, currentBillId,
                                      QString::number(updated.remainingBalance, 'f', 2)));
}