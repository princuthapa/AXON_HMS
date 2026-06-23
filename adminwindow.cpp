#include "mainwindow.h"
#include "adminwindow.h"
#include "ui_adminwindow.h"
#include <QDesktopServices>
#include <QUrl>
#include <QDateTime>
#include <QTimer>
#include <QMenu>
#include <QAction>
#include <QPoint>
#include <QButtonGroup>
#include <QScrollArea>
#include <QScrollBar>

// Required native elements
#include <QtCharts/QChartView>
#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>
#include <QtCharts/QLineSeries>
#include <QtCharts/QChart>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>
#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QFormLayout>
#include <QFrame>
#include <QSpacerItem>


class EditPatientDialog : public QDialog {
public:
    QLineEdit *nameEdit;
    QComboBox *genderBox;
    QLineEdit *problemEdit;
    QLineEdit *doctorEdit;
    QComboBox *statusBox;

    EditPatientDialog(const QString &id, const QString &name, const QString &gender,
                      const QString &problem, const QString &doctor, const QString &status,
                      QWidget *parent = nullptr) : QDialog(parent)
    {
        setWindowTitle("Modify Patient Record — " + id);
        setMinimumWidth(380);
        setStyleSheet(
            "QDialog { background-color: #ffffff; border-radius: 8px; }"
            "QLabel { font-weight: bold; color: #334155; font-size: 12px; border:none; background:transparent; }"
            "QLineEdit, QComboBox { padding: 6px; border: 1px solid #cbd5e1; border-radius: 6px;"
            "  background: #f8fafc; color: #0f172a; }"
            "QLineEdit:focus, QComboBox:focus { border: 1px solid #6366f1; background: #ffffff; }"
            "QPushButton { padding: 6px 14px; font-weight: bold; border-radius: 6px; font-size: 12px; }"
        );

        QFormLayout *form = new QFormLayout(this);
        form->setContentsMargins(24, 24, 24, 24);
        form->setSpacing(14);

        nameEdit    = new QLineEdit(name, this);
        genderBox   = new QComboBox(this);
        genderBox->addItems({"Male", "Female", "Other"});
        genderBox->setCurrentText(gender);
        problemEdit = new QLineEdit(problem, this);
        doctorEdit  = new QLineEdit(doctor, this);
        statusBox   = new QComboBox(this);
        statusBox->addItems({"Admitted", "Discharged", "Checked In", "No Show", "Completed"});
        statusBox->setCurrentText(status.trimmed());

        form->addRow("Patient Name:",       nameEdit);
        form->addRow("Gender:",             genderBox);
        form->addRow("Diagnosis/Problem:",  problemEdit);
        form->addRow("Assigned Doctor:",    doctorEdit);
        form->addRow("Status:",             statusBox);

        QHBoxLayout *btns = new QHBoxLayout();
        QPushButton *cancel = new QPushButton("Cancel", this);
        QPushButton *save   = new QPushButton("Save Changes", this);
        cancel->setStyleSheet("background-color:#f1f5f9;color:#475569;border:1px solid #e2e8f0;");
        save->setStyleSheet("background-color:#0284c7;color:#ffffff;border:none;");
        btns->addStretch();
        btns->addWidget(cancel);
        btns->addWidget(save);
        form->addRow(btns);

        connect(cancel, &QPushButton::clicked, this, &QDialog::reject);
        connect(save,   &QPushButton::clicked, this, &QDialog::accept);
    }
};


class AddStaffDialog : public QDialog {
public:
    QLineEdit *usernameEdit;
    QLineEdit *passwordEdit;
    QComboBox *roleBox;
    QLineEdit *idEdit;
    QLineEdit *nameEdit;
    QLineEdit *ageEdit;
    QComboBox *genderBox;
    QLineEdit *phoneEdit;

    explicit AddStaffDialog(QWidget *parent = nullptr) : QDialog(parent)
    {
        setWindowTitle("Add New Staff Member");
        setMinimumWidth(400);
        setStyleSheet(
            "QDialog { background-color: #ffffff; }"
            "QLabel { font-weight: 600; color: #334155; font-size: 12px; border:none; background:transparent; }"
            "QLineEdit, QComboBox { padding: 7px 10px; border: 1px solid #cbd5e1; border-radius: 8px;"
            "  background: #f8fafc; color: #0f172a; font-size: 12px; }"
            "QLineEdit:focus, QComboBox:focus { border: 1px solid #0284c7; background: #fff; }"
            "QPushButton { padding: 8px 16px; font-weight: bold; border-radius: 8px; font-size: 12px; }"
        );

        QFormLayout *form = new QFormLayout(this);
        form->setContentsMargins(28, 28, 28, 28);
        form->setSpacing(14);

        // Credentials section header
        QLabel *credHeader = new QLabel("── Login Credentials ──", this);
        credHeader->setStyleSheet("color:#0284c7; font-size:11px; font-weight:700; border:none; background:transparent;");
        form->addRow(credHeader);

        usernameEdit = new QLineEdit(this);
        usernameEdit->setPlaceholderText("e.g. drsmith");
        passwordEdit = new QLineEdit(this);
        passwordEdit->setPlaceholderText("Set initial password");
        passwordEdit->setEchoMode(QLineEdit::Password);

        roleBox = new QComboBox(this);
        roleBox->addItems({"Admin", "Doctor", "Receptionist"});

        form->addRow("Username:",  usernameEdit);
        form->addRow("Password:",  passwordEdit);
        form->addRow("Role:",      roleBox);

        // Profile section header
        QLabel *profileHeader = new QLabel("── Staff Profile ──", this);
        profileHeader->setStyleSheet("color:#0284c7; font-size:11px; font-weight:700; border:none; background:transparent;");
        form->addRow(profileHeader);

        idEdit    = new QLineEdit(this);
        idEdit->setPlaceholderText("e.g. STF_004");
        nameEdit  = new QLineEdit(this);
        nameEdit->setPlaceholderText("Full Name");
        ageEdit   = new QLineEdit(this);
        ageEdit->setPlaceholderText("Age");
        genderBox = new QComboBox(this);
        genderBox->addItems({"Male", "Female", "Other"});
        phoneEdit = new QLineEdit(this);
        phoneEdit->setPlaceholderText("Phone Number");

        form->addRow("Staff ID:",  idEdit);
        form->addRow("Full Name:", nameEdit);
        form->addRow("Age:",       ageEdit);
        form->addRow("Gender:",    genderBox);
        form->addRow("Phone:",     phoneEdit);

        // Buttons
        QHBoxLayout *btns = new QHBoxLayout();
        QPushButton *cancel = new QPushButton("Cancel", this);
        QPushButton *addBtn = new QPushButton("Add Staff", this);
        cancel->setStyleSheet("background:#f1f5f9;color:#475569;border:1px solid #e2e8f0;");
        addBtn->setStyleSheet("background:#0284c7;color:#ffffff;border:none;");
        btns->addStretch();
        btns->addWidget(cancel);
        btns->addWidget(addBtn);
        form->addRow(btns);

        connect(cancel, &QPushButton::clicked, this, &QDialog::reject);
        connect(addBtn, &QPushButton::clicked, this, [=]() {
            if (usernameEdit->text().trimmed().isEmpty() ||
                passwordEdit->text().trimmed().isEmpty() ||
                idEdit->text().trimmed().isEmpty()       ||
                nameEdit->text().trimmed().isEmpty()) {
                QMessageBox::warning(this, "Incomplete", "Username, Password, Staff ID, and Full Name are required.");
                return;
            }
            accept();
        });
    }
};


class EditStaffDialog : public QDialog {
public:
    QLineEdit *usernameEdit;
    QLineEdit *passwordEdit;
    QComboBox *roleBox;
    QLineEdit *nameEdit;
    QLineEdit *ageEdit;
    QComboBox *genderBox;
    QLineEdit *phoneEdit;

    EditStaffDialog(const StaffData &s, QWidget *parent = nullptr) : QDialog(parent)
    {
        setWindowTitle("Edit Staff — " + s.id);
        setMinimumWidth(380);
        setStyleSheet(
            "QDialog { background-color: #ffffff; }"
            "QLabel { font-weight: 600; color: #334155; font-size: 12px; border:none; background:transparent; }"
            "QLineEdit, QComboBox { padding: 7px 10px; border: 1px solid #cbd5e1; border-radius: 8px;"
            "  background: #f8fafc; color: #0f172a; }"
            "QLineEdit:focus, QComboBox:focus { border: 1px solid #0284c7; background: #fff; }"
            "QPushButton { padding: 8px 16px; font-weight: bold; border-radius: 8px; }"
        );

        QFormLayout *form = new QFormLayout(this);
        form->setContentsMargins(24, 24, 24, 24);
        form->setSpacing(12);

        usernameEdit = new QLineEdit(s.username, this);
        passwordEdit = new QLineEdit(s.password, this);
        passwordEdit->setEchoMode(QLineEdit::Password);
        roleBox      = new QComboBox(this);
        roleBox->addItems({"Admin", "Doctor", "Receptionist"});
        roleBox->setCurrentText(s.role);
        nameEdit  = new QLineEdit(s.name,   this);
        ageEdit   = new QLineEdit(s.age,    this);
        genderBox = new QComboBox(this);
        genderBox->addItems({"Male", "Female", "Other"});
        genderBox->setCurrentText(s.gender);
        phoneEdit = new QLineEdit(s.phone,  this);

        form->addRow("Username:", usernameEdit);
        form->addRow("Password:", passwordEdit);
        form->addRow("Role:",     roleBox);
        form->addRow("Name:",     nameEdit);
        form->addRow("Age:",      ageEdit);
        form->addRow("Gender:",   genderBox);
        form->addRow("Phone:",    phoneEdit);

        QHBoxLayout *btns = new QHBoxLayout();
        QPushButton *cancel = new QPushButton("Cancel", this);
        QPushButton *save   = new QPushButton("Save", this);
        cancel->setStyleSheet("background:#f1f5f9;color:#475569;border:1px solid #e2e8f0;");
        save->setStyleSheet("background:#0284c7;color:#ffffff;border:none;");
        btns->addStretch();
        btns->addWidget(cancel);
        btns->addWidget(save);
        form->addRow(btns);

        connect(cancel, &QPushButton::clicked, this, &QDialog::reject);
        connect(save,   &QPushButton::clicked, this, &QDialog::accept);
    }
};

// charts and line graphs

static void embedChart(QWidget *container, QChart *chart) {
    if (!container || !chart) return;
    chart->setBackgroundVisible(false);
    chart->setMargins(QMargins(0, 0, 0, 0));
    chart->legend()->hide();

    QChartView *view = new QChartView(chart);
    view->setRenderHint(QPainter::Antialiasing);

    if (!container->layout()) {
        QVBoxLayout *l = new QVBoxLayout(container);
        l->setContentsMargins(0, 0, 0, 0);
        container->setLayout(l);
    } else {
        QLayoutItem *item;
        while ((item = container->layout()->takeAt(0)) != nullptr) {
            if (item->widget()) delete item->widget();
            delete item;
        }
    }
    container->layout()->addWidget(view);
}

// Returns a styled status badge label
static void applyStatusBadge(QLabel *label, const QString &status) {
    label->setText(status.toUpper());
    label->setAlignment(Qt::AlignCenter);
    QString norm = status.toLower().trimmed();

    if (norm == "admitted" || norm == "checked in" || norm == "present") {
        label->setStyleSheet(
            "background-color:#dcfce7;color:#15803d;border-radius:6px;"
            "font-weight:bold;font-size:11px;padding:4px 8px;border:none;");
    } else if (norm == "discharged" || norm == "no show" || norm == "absent") {
        label->setStyleSheet(
            "background-color:#fee2e2;color:#b91c1c;border-radius:6px;"
            "font-weight:bold;font-size:11px;padding:4px 8px;border:none;");
    } else {
        label->setStyleSheet(
            "background-color:#e0f2fe;color:#0369a1;border-radius:6px;"
            "font-weight:bold;font-size:11px;padding:4px 8px;border:none;");
    }
}

// Role badge colouring
static void applyRoleBadge(QLabel *label, const QString &role) {
    label->setText(role.toUpper());
    label->setAlignment(Qt::AlignCenter);
    QString r = role.toLower().trimmed();
    if (r == "admin") {
        label->setStyleSheet(
            "background-color:#fef9c3;color:#854d0e;border-radius:6px;"
            "font-weight:bold;font-size:11px;padding:4px 8px;border:none;");
    } else if (r == "doctor") {
        label->setStyleSheet(
            "background-color:#dbeafe;color:#1e40af;border-radius:6px;"
            "font-weight:bold;font-size:11px;padding:4px 8px;border:none;");
    } else {
        label->setStyleSheet(
            "background-color:#f3e8ff;color:#7e22ce;border-radius:6px;"
            "font-weight:bold;font-size:11px;padding:4px 8px;border:none;");
    }
}

static const QString kPlainLabel =
    "border:none;background:transparent;background-color:transparent;";

// core

adminwindow::adminwindow(const QString &employeeName, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::AXON_ADMIN)
    , currentAdminName(employeeName)
{
    ui->setupUi(this);

    // Sidebar exclusive button group
    QButtonGroup *sidebarGroup = new QButtonGroup(this);
    sidebarGroup->addButton(ui->btnOverview);
    sidebarGroup->addButton(ui->btnStaffManager);
    sidebarGroup->addButton(ui->btnScheduling);
    sidebarGroup->setExclusive(true);

    // Initialise backend objects
    staffMgr    = new StaffManager();
    patientMgr  = new PatientManager();
    adminBackend = nullptr; // Admin class is pure logic; not needed for UI

    // Live clock
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &adminwindow::updateDateTime);
    timer->start(1000);
    updateDateTime();

    // Logo
    QPixmap logoPixmap(":/images/axonimg.png");
    if (ui->lblLogo && !logoPixmap.isNull())
        ui->lblLogo->setPixmap(logoPixmap.scaled(90, 90, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    // Welcome message
    if (ui->message)
        ui->message->setText(QString("Welcome back, %1!").arg(currentAdminName));

    // Start on overview
    if (ui->stackedWidget) ui->stackedWidget->setCurrentIndex(0);
    ui->btnOverview->setChecked(true);

    // Build overview charts and patient table
    initDashboardGraphs();
    setupPatientHeader();
    loadPatientRowsFromBackend();

    // Build the staff manager page
    setupStaffPage();
}

adminwindow::~adminwindow()
{
    delete ui;
    delete staffMgr;
    delete patientMgr;
    if (adminBackend) delete adminBackend;
}


// CLOCK

void adminwindow::updateDateTime()
{
    if (ui->lblDateTime)
        ui->lblDateTime->setText(
            QDateTime::currentDateTime().toString("ddd MMM d yyyy,  hh:mm AP"));
}


// MENU / NAV BUTTONS

void adminwindow::on_btnMenu_clicked()
{
    QMenu menu(this);
    QAction *logoutAction = menu.addAction("Logout");
    QPoint pos = ui->btnMenu->mapToGlobal(QPoint(0, ui->btnMenu->height()));
    if (menu.exec(pos) == logoutAction) {
        MainWindow *loginScreen = new MainWindow();
        loginScreen->show();
        this->close();
    }
}

void adminwindow::on_btnOverview_clicked()     { if (ui->stackedWidget) ui->stackedWidget->setCurrentIndex(0); }
void adminwindow::on_btnStaffManager_clicked() { if (ui->stackedWidget) ui->stackedWidget->setCurrentIndex(1); }
void adminwindow::on_btnScheduling_clicked()   { QDesktopServices::openUrl(QUrl("https://calendar.google.com/calendar/u/0/r")); }


// OVERVIEW — CHARTS

void adminwindow::initDashboardGraphs()
{
    int totalStaff   = staffMgr->getTotalCount();
    int totalPatients = patientMgr->getTotalCount();

    // Update stat labels with live data
    if (ui->lblValueStaff)
        ui->lblValueStaff->setText(QString("%1 / 350").arg(totalStaff));
    if (ui->lblValuePatients)
        ui->lblValuePatients->setText(QString::number(totalPatients));

    // 1. STAFF PIE
    QPieSeries *staffSeries = new QPieSeries();
    staffSeries->setPieSize(1.0);
    staffSeries->append("Active",    totalStaff)->setBrush(QColor(0x166534));
    staffSeries->append("Remaining", qMax(0, 350 - totalStaff))->setBrush(QColor(0xbbf7d0));
    QChart *staffChart = new QChart();
    staffChart->addSeries(staffSeries);
    if (ui->widgetGraphStaff) embedChart(ui->widgetGraphStaff, staffChart);
    if (ui->lblSubtextStaff)
        ui->lblSubtextStaff->setText("<span style='color:#166534;'>●</span> Active &nbsp;"
                                     "<span style='color:#bbf7d0;'>●</span> Inactive");

    // 2. ACTIVE DOCTORS donut
    QPieSeries *docSeries = new QPieSeries();
    docSeries->setHoleSize(0.75);
    docSeries->setPieSize(0.95);
    docSeries->append("On Shift", 42)->setBrush(QColor(0x0284c7));
    docSeries->append("Vacant",    8)->setBrush(QColor(0xf1f5f9));
    QChart *docChart = new QChart();
    docChart->addSeries(docSeries);
    if (ui->widgetGraphDoctors) embedChart(ui->widgetGraphDoctors, docChart);
    if (ui->lblSubtextDoctors)
        ui->lblSubtextDoctors->setText("<span style='color:#0284c7;'>●</span> On Shift &nbsp;"
                                       "<span style='color:#f1f5f9;'>●</span> Vacant");

    // 3. PATIENTS gender breakdown (from actual data)
    int male = 0, female = 0, other = 0;
    for (const auto &p : patientMgr->getAllPatients()) {
        QString g = p.gender.toLower();
        if (g == "male")        male++;
        else if (g == "female") female++;
        else                    other++;
    }
    QPieSeries *patSeries = new QPieSeries();
    patSeries->setHoleSize(0.35);
    if (male)   patSeries->append("Male",   male)->setBrush(QColor(0x6366f1));
    if (female) patSeries->append("Female", female)->setBrush(QColor(0xec4899));
    if (other)  patSeries->append("Other",  other)->setBrush(QColor(0xf59e0b));
    QChart *patChart = new QChart();
    patChart->addSeries(patSeries);
    if (ui->widgetGraphPatients) embedChart(ui->widgetGraphPatients, patChart);
    if (ui->lblSubtextPatients)
        ui->lblSubtextPatients->setText(
            "<span style='color:#6366f1;'>●</span> Male &nbsp;"
            "<span style='color:#ec4899;'>●</span> Female &nbsp;"
            "<span style='color:#f59e0b;'>●</span> Other");

    // 4. REVENUE TREND (mock)
    QLineSeries *revSeries = new QLineSeries();
    revSeries->append(0, 95000); revSeries->append(1, 110000);
    revSeries->append(2, 125000); revSeries->append(3, 142500);
    revSeries->setPen(QPen(QColor(0x0d9488), 4));
    QChart *revChart = new QChart();
    revChart->addSeries(revSeries);
    revChart->createDefaultAxes();
    if (ui->widgetGraphRevenue) embedChart(ui->widgetGraphRevenue, revChart);
    if (ui->lblSubtextRevenue)
        ui->lblSubtextRevenue->setText("<span style='color:#0d9488;'>●</span> Revenue &nbsp;"
                                       "<span style='color:#880808;'>●</span> Expense");
}


// patient table

void adminwindow::setupPatientHeader()
{
    if (!ui->webContainer) return;

    QWidget *headerWidget = new QWidget();
    QHBoxLayout *hl = new QHBoxLayout(headerWidget);
    hl->setContentsMargins(15, 5, 15, 5);
    hl->setSpacing(10);

    const QString hStyle =
        "font-weight:bold;font-size:11px;color:#94a3b8;"
        "border:none;background:transparent;background-color:transparent;";

    auto makeH = [&](const QString &txt, int stretch) {
        QLabel *l = new QLabel(txt);
        l->setStyleSheet(hStyle);
        hl->addWidget(l, stretch);
    };
    makeH("PATIENT ID", 1); makeH("NAME", 2); makeH("GENDER", 1);
    makeH("PROBLEM / DIAGNOSIS", 2); makeH("ASSIGNED DOCTOR", 2);
    makeH("STATUS", 1); makeH("ACTION", 1);

    headerWidget->setLayout(hl);

    QVBoxLayout *cardLayout = qobject_cast<QVBoxLayout*>(ui->webContainer->layout());
    if (!cardLayout) {
        cardLayout = new QVBoxLayout(ui->webContainer);
        cardLayout->setContentsMargins(15, 15, 15, 15);
        cardLayout->setSpacing(8);
        ui->webContainer->setLayout(cardLayout);
    }
    cardLayout->insertWidget(0, headerWidget);
}

void adminwindow::loadPatientRowsFromBackend()
{
    auto patients = patientMgr->getAllPatients();
    for (const auto &p : patients) {
        addPatientRow(p.id, p.name, p.gender, p.diagnosisTreatment,
                      p.assignedDoctor, p.status);
    }
    // If database is empty, show placeholder rows so the UI isn't blank
    if (patients.isEmpty()) {
        addPatientRow("PT-0001", "Aditya Poudel",   "Male",   "Cardiac Checkup",     "Dr. Rijal",   "Admitted");
        addPatientRow("PT-0002", "Mira Gurung",     "Female", "Migraine Treatment",   "Dr. Subedi",  "Discharged");
        addPatientRow("PT-0003", "Hari Rana",       "Male",   "Fracture Follow-up",   "Dr. Baral",   "Admitted");
        addPatientRow("PT-0004", "Sita Wagle",      "Female", "General Physical",     "Dr. Acharya", "Completed");
    }
}

void adminwindow::addPatientRow(const QString &id, const QString &name, const QString &gender,
                                const QString &problem, const QString &doctor, const QString &status)
{
    if (!ui->webContainer) return;

    QFrame *row = new QFrame();
    row->setObjectName("patientRow");
    row->setStyleSheet(
        "QFrame#patientRow{background-color:#ffffff;border-bottom:1px solid #f1f5f9;}"
        "QFrame#patientRow:hover{background-color:#f8fafc;}");

    QHBoxLayout *rl = new QHBoxLayout(row);
    rl->setContentsMargins(15, 10, 15, 10);
    rl->setSpacing(10);

    QLabel *lblId      = new QLabel(id);
    QLabel *lblName    = new QLabel(name);
    QLabel *lblGender  = new QLabel(gender);
    QLabel *lblProblem = new QLabel(problem);
    QLabel *lblDoctor  = new QLabel(doctor);
    QLabel *lblStatus  = new QLabel();
    QPushButton *btnEdit = new QPushButton("Edit");

    lblId->setStyleSheet("font-weight:bold;color:#64748b;" + kPlainLabel);
    lblName->setStyleSheet("font-weight:bold;color:#1e293b;" + kPlainLabel);
    lblGender->setStyleSheet("color:#334155;" + kPlainLabel);
    lblProblem->setStyleSheet("color:#334155;" + kPlainLabel);
    lblDoctor->setStyleSheet("color:#334155;" + kPlainLabel);
    applyStatusBadge(lblStatus, status);

    btnEdit->setStyleSheet(
        "QPushButton{background-color:#f1f5f9;color:#0f172a;border:1px solid #e2e8f0;"
        "border-radius:6px;font-weight:bold;font-size:12px;padding:4px 12px;max-width:60px;}"
        "QPushButton:hover{background-color:#e2e8f0;}"
        "QPushButton:pressed{background-color:#cbd5e1;}");

    connect(btnEdit, &QPushButton::clicked, this, [=]() {
        EditPatientDialog dlg(lblId->text(), lblName->text(), lblGender->text(),
                              lblProblem->text(), lblDoctor->text(), lblStatus->text(), this);
        if (dlg.exec() == QDialog::Accepted) {
            lblName->setText(dlg.nameEdit->text());
            lblGender->setText(dlg.genderBox->currentText());
            lblProblem->setText(dlg.problemEdit->text());
            lblDoctor->setText(dlg.doctorEdit->text());
            applyStatusBadge(lblStatus, dlg.statusBox->currentText());

            // Persist change to backend
            Patient updated = patientMgr->searchPatient(lblId->text());
            updated.name               = dlg.nameEdit->text();
            updated.gender             = dlg.genderBox->currentText();
            updated.diagnosisTreatment = dlg.problemEdit->text();
            updated.assignedDoctor     = dlg.doctorEdit->text();
            updated.status             = dlg.statusBox->currentText();
            patientMgr->removePatient(lblId->text());
            patientMgr->addPatient(updated);
        }
    });

    rl->addWidget(lblId,      1);
    rl->addWidget(lblName,    2);
    rl->addWidget(lblGender,  1);
    rl->addWidget(lblProblem, 2);
    rl->addWidget(lblDoctor,  2);
    rl->addWidget(lblStatus,  1);
    rl->addWidget(btnEdit,    1);
    row->setLayout(rl);

    QVBoxLayout *cl = qobject_cast<QVBoxLayout*>(ui->webContainer->layout());
    if (cl) cl->addWidget(row);
}


// STAFF MANAGER PAGE


void adminwindow::setupStaffPage()
{
    // page_2 in the stacked widget is the staff manager page.

    QWidget *page2 = ui->stackedWidget->widget(1);
    if (!page2) return;

    // Clear any existing layout
    if (page2->layout()) {
        QLayoutItem *item;
        while ((item = page2->layout()->takeAt(0))) {
            if (item->widget()) item->widget()->deleteLater();
            delete item;
        }
        delete page2->layout();
    }

    QVBoxLayout *pageLayout = new QVBoxLayout(page2);
    pageLayout->setContentsMargins(20, 16, 20, 16);
    pageLayout->setSpacing(14);

    // ── Header row ──────────────────────────────────────────────────────────
    QHBoxLayout *headerRow = new QHBoxLayout();
    QLabel *title = new QLabel("Staff Manager");
    title->setStyleSheet("font-size:22px;font-weight:700;color:#0f172a;border:none;background:transparent;");

    QPushButton *btnAdd = new QPushButton("＋  Add Staff");
    btnAdd->setFixedHeight(38);
    btnAdd->setStyleSheet(
        "QPushButton{background-color:#0284c7;color:#ffffff;border:none;"
        "border-radius:8px;font-weight:700;font-size:13px;padding:0 18px;}"
        "QPushButton:hover{background-color:#0369a1;}"
        "QPushButton:pressed{background-color:#075985;}");

    headerRow->addWidget(title);
    headerRow->addStretch();
    headerRow->addWidget(btnAdd);
    pageLayout->addLayout(headerRow);


    staffCountLabel = new QLabel();
    staffCountLabel->setStyleSheet("font-size:13px;color:#64748b;border:none;background:transparent;");
    updateStaffCountLabel();
    pageLayout->addWidget(staffCountLabel);


    QWidget *colHeader = new QWidget();
    colHeader->setStyleSheet("background-color:#f8fafc;border-bottom:2px solid #e2e8f0;"
                             "border-radius:0px;");
    QHBoxLayout *chLayout = new QHBoxLayout(colHeader);
    chLayout->setContentsMargins(16, 8, 16, 8);
    chLayout->setSpacing(10);

    const QString chStyle =
        "font-weight:700;font-size:11px;color:#94a3b8;"
        "border:none;background:transparent;background-color:transparent;";
    auto makeCol = [&](const QString &txt, int s) {
        QLabel *l = new QLabel(txt); l->setStyleSheet(chStyle);
        chLayout->addWidget(l, s);
    };
    makeCol("STAFF ID",  1); makeCol("FULL NAME", 2); makeCol("ROLE", 1);
    makeCol("AGE",       1); makeCol("GENDER",    1); makeCol("PHONE",    2);
    makeCol("USERNAME",  1); makeCol("ACTIONS",   1);
    pageLayout->addWidget(colHeader);


    staffScrollArea = new QScrollArea();
    staffScrollArea->setWidgetResizable(true);
    staffScrollArea->setFrameShape(QFrame::NoFrame);
    staffScrollArea->setStyleSheet("QScrollArea{border:none;background:transparent;}");

    staffRowContainer = new QWidget();
    staffRowContainer->setStyleSheet("background:transparent;");
    staffRowsLayout = new QVBoxLayout(staffRowContainer);
    staffRowsLayout->setContentsMargins(0, 0, 0, 0);
    staffRowsLayout->setSpacing(0);
    staffRowsLayout->addStretch();

    staffScrollArea->setWidget(staffRowContainer);
    pageLayout->addWidget(staffScrollArea, 1);

    // Populate from backend
    refreshStaffTable();

    // Connect Add button
    connect(btnAdd, &QPushButton::clicked, this, &adminwindow::onAddStaffClicked);
}

void adminwindow::updateStaffCountLabel()
{
    if (!staffCountLabel) return;
    int total  = staffMgr->getTotalCount();
    int docs   = 0, admins = 0, recs = 0;
    for (const auto &s : staffMgr->getAllStaff()) {
        if (s.role == "Doctor")       docs++;
        else if (s.role == "Admin")   admins++;
        else                          recs++;
    }
    staffCountLabel->setText(
        QString("Total Staff: <b>%1</b> &nbsp;|&nbsp; "
                "Doctors: <b>%2</b> &nbsp;|&nbsp; "
                "Admins: <b>%3</b> &nbsp;|&nbsp; "
                "Receptionists: <b>%4</b>")
        .arg(total).arg(docs).arg(admins).arg(recs));
}

void adminwindow::refreshStaffTable()
{
    // Remove all existing rows (leave the stretch at the end)
    while (staffRowsLayout->count() > 1) {
        QLayoutItem *item = staffRowsLayout->takeAt(0);
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }

    for (const auto &s : staffMgr->getAllStaff()) {
        addStaffRow(s);
    }
    updateStaffCountLabel();
}

void adminwindow::addStaffRow(const StaffData &s)
{
    QFrame *row = new QFrame();
    row->setObjectName("staffRow");
    row->setStyleSheet(
        "QFrame#staffRow{background-color:#ffffff;border-bottom:1px solid #f1f5f9;}"
        "QFrame#staffRow:hover{background-color:#f8fafc;}");

    QHBoxLayout *rl = new QHBoxLayout(row);
    rl->setContentsMargins(16, 10, 16, 10);
    rl->setSpacing(10);

    QLabel *lblId     = new QLabel(s.id);
    QLabel *lblName   = new QLabel(s.name);
    QLabel *lblRole   = new QLabel();
    QLabel *lblAge    = new QLabel(s.age);
    QLabel *lblGender = new QLabel(s.gender);
    QLabel *lblPhone  = new QLabel(s.phone);
    QLabel *lblUser   = new QLabel(s.username);

    lblId->setStyleSheet("font-weight:bold;color:#64748b;" + kPlainLabel);
    lblName->setStyleSheet("font-weight:bold;color:#1e293b;" + kPlainLabel);
    lblAge->setStyleSheet("color:#334155;" + kPlainLabel);
    lblGender->setStyleSheet("color:#334155;" + kPlainLabel);
    lblPhone->setStyleSheet("color:#334155;" + kPlainLabel);
    lblUser->setStyleSheet("font-family:monospace;color:#0369a1;" + kPlainLabel);
    applyRoleBadge(lblRole, s.role);

    // Actions: Edit + Remove
    QWidget *actionWidget = new QWidget();
    actionWidget->setStyleSheet("background:transparent;border:none;");
    QHBoxLayout *al = new QHBoxLayout(actionWidget);
    al->setContentsMargins(0, 0, 0, 0);
    al->setSpacing(6);

    QPushButton *btnEdit   = new QPushButton("Edit");
    QPushButton *btnRemove = new QPushButton("Remove");
    btnEdit->setFixedSize(52, 28);
    btnRemove->setFixedSize(68, 28);
    btnEdit->setStyleSheet(
        "QPushButton{background:#f1f5f9;color:#0f172a;border:1px solid #e2e8f0;"
        "border-radius:6px;font-weight:bold;font-size:11px;}"
        "QPushButton:hover{background:#e2e8f0;}");
    btnRemove->setStyleSheet(
        "QPushButton{background:#fee2e2;color:#b91c1c;border:1px solid #fecaca;"
        "border-radius:6px;font-weight:bold;font-size:11px;}"
        "QPushButton:hover{background:#fecaca;}");
    al->addWidget(btnEdit);
    al->addWidget(btnRemove);
    actionWidget->setLayout(al);

    QString staffIdCopy = s.id; // captured for lambdas

    connect(btnEdit, &QPushButton::clicked, this, [=]() {
        StaffData current = staffMgr->searchStaff(staffIdCopy);
        EditStaffDialog dlg(current, this);
        if (dlg.exec() == QDialog::Accepted) {
            current.username = dlg.usernameEdit->text().trimmed();
            current.password = dlg.passwordEdit->text().trimmed();
            current.role     = dlg.roleBox->currentText();
            current.name     = dlg.nameEdit->text().trimmed();
            current.age      = dlg.ageEdit->text().trimmed();
            current.gender   = dlg.genderBox->currentText();
            current.phone    = dlg.phoneEdit->text().trimmed();

            if (staffMgr->updateStaff(current)) {
                refreshStaffTable();
            }
        }
    });

    connect(btnRemove, &QPushButton::clicked, this, [=]() {
        StaffData s2 = staffMgr->searchStaff(staffIdCopy);
        auto reply = QMessageBox::question(this,
            "Confirm Removal",
            QString("Remove staff member <b>%1</b> (%2)?<br>"
                    "This will also delete their login credentials.")
            .arg(s2.name, s2.id),
            QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            staffMgr->removeStaff(staffIdCopy);
            refreshStaffTable();
        }
    });

    rl->addWidget(lblId,     1);
    rl->addWidget(lblName,   2);
    rl->addWidget(lblRole,   1);
    rl->addWidget(lblAge,    1);
    rl->addWidget(lblGender, 1);
    rl->addWidget(lblPhone,  2);
    rl->addWidget(lblUser,   1);
    rl->addWidget(actionWidget, 1);
    row->setLayout(rl);

    // Insert before the trailing stretch
    staffRowsLayout->insertWidget(staffRowsLayout->count() - 1, row);
}

void adminwindow::onAddStaffClicked()
{
    AddStaffDialog dlg(this);
    if (dlg.exec() != QDialog::Accepted) return;

    StaffData newStaff;
    newStaff.username = dlg.usernameEdit->text().trimmed();
    newStaff.password = dlg.passwordEdit->text().trimmed();
    newStaff.role     = dlg.roleBox->currentText();
    newStaff.id       = dlg.idEdit->text().trimmed();
    newStaff.name     = dlg.nameEdit->text().trimmed();
    newStaff.age      = dlg.ageEdit->text().trimmed();
    newStaff.gender   = dlg.genderBox->currentText();
    newStaff.phone    = dlg.phoneEdit->text().trimmed();

    // Duplicate username check
    for (const auto &s : staffMgr->getAllStaff()) {
        if (s.username == newStaff.username) {
            QMessageBox::warning(this, "Duplicate Username",
                                 "A staff member with that username already exists.");
            return;
        }
        if (s.id == newStaff.id) {
            QMessageBox::warning(this, "Duplicate Staff ID",
                                 "A staff member with that ID already exists.");
            return;
        }
    }

    staffMgr->addStaff(newStaff);
    refreshStaffTable();

    QMessageBox::information(this, "Staff Added",
        QString("Staff member <b>%1</b> added successfully.<br>"
                "Login: <b>%2</b> / <b>%3</b>")
        .arg(newStaff.name, newStaff.username, newStaff.password));
}
