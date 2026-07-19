#include "doctorwindow.h"
#include "ui_doctorwindow.h"
#include "mainwindow.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QListWidget>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QMenu>
#include <QAction>
#include <QPushButton>
#include <QDateTime>
#include <QFrame>
#include <QScrollArea>
#include <QComboBox>
#include <QDebug>

static const QString kPlainLabelD =
    "border:none;background:transparent;background-color:transparent;";

static void applyStatusBadgeD(QLabel *label, const QString &status) {
    label->setText(status.toUpper());
    label->setAlignment(Qt::AlignCenter);
    label->setStyleSheet("font-weight:bold;font-size:11px;padding:4px 8px;border:none;"
                         "background-color:#e8f0fe;color:#1a4f8a;border-radius:10px;");
}

// ===== CONSTRUCTOR =====
doctorwindow::doctorwindow(const QString &employeeName, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::doctorwindow),
    currentUserName(employeeName)
{
    ui->setupUi(this);
    if (ui->message)
        ui->message->setText(QString("Welcome, Dr. %1!").arg(currentUserName));

    // Shared backend — same CSVs Admin & Receptionist windows read/write.
    patientMgr = new PatientManager();
    apptMgr    = new AppointmentManager();

    // Clock
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &doctorwindow::updateDateTime);
    updateDateTime();
    timer->start(1000);

    // Sidebar Navigation
    connect(ui->overviewBtn, &QPushButton::clicked, this, &doctorwindow::switchPage);
    connect(ui->patientlistBtn, &QPushButton::clicked, this, &doctorwindow::switchPage);
    connect(ui->scheduleBtn, &QPushButton::clicked, this, &doctorwindow::switchPage);
    ui->stackedWidget->setCurrentIndex(0);

    // Logout Menu
    QMenu *logoutMenu = new QMenu(this);
    logoutMenu->addAction("  Logout  ", this, &doctorwindow::logout);
    ui->logoutBtn->setMenu(logoutMenu);

    logoutMenu->setStyleSheet(
        "QMenu {"
        "background-color: #ffffff ;"
        "color:#2c3e50;"
        "border: 1 px solid #dce1e6;"
        "border-radius: 10 px;"
        "padding: 8 px;"
        "width: 120 px;"
        "height:50 px;"
        "}"
        "QMenu::item {"
        "padding: 0 px;"
        "border-radius: 6 px;"
        "margin: 0 px;"
        "font-size: 13 px;"
        "color: 2c3e50;"
        "width: 100 px;"
        "height: 40 px;"
        "text-align:center;"
        "}"
        "QMenu::item:selected {"
        "background-color: #e8f0fe;"
        "color: #1a73e8;"
        "}"
        );

    // Apply light theme
    this->setStyleSheet(
        "QWidget {"
        "   background-color: #f8f9fc;"
        "   font-family: 'Segoe UI', Arial, sans-serif;"
        "}"
        "QListWidget {"
        "   background-color: #ffffff;"
        "   border: 1px solid #e9ecef;"
        "   border-radius: 10px;"
        "   padding: 5px;"
        "}"
        "QListWidget::item {"
        "   padding: 10px 8px;"
        "   border-bottom: 1px solid #f2f4f7;"
        "}"
        "QListWidget::item:selected {"
        "   background-color: #f0f4f9;"
        "}"
        "QTableWidget {"
        "   background-color: #ffffff;"
        "   border: 1px solid #e9ecef;"
        "   border-radius: 10px;"
        "   gridline-color: #f2f4f7;"
        "}"
        "QTableWidget::item {"
        "   padding: 8px;"
        "   color: #2c3e50;"
        "}"
        "QTableWidget::item:selected {"
        "   background-color: #e8f0fe;"
        "   color: #1a73e8;"
        "}"
        "QHeaderView::section {"
        "   background-color: #f8f9fc;"
        "   padding: 8px;"
        "   border: none;"
        "   font-weight: 600;"
        "   color: #5a6c7d;"
        "}"
        );

    // Setup UI
    setupStatsSection();
    populatePatientList();
    populateAppointmentsTable();
    setupOverviewLayout();  // MUST BE LAST!

    // Setup other pages
    setupPatientListPage();
    setupSchedulingPage();

    doctorBackend = new Doctor();
    this->setWindowTitle("AXON-HMS: Doctor Dashboard");
}

// ===== DESTRUCTOR =====
doctorwindow::~doctorwindow()
{
    delete doctorBackend;
    delete patientMgr;
    delete apptMgr;
    delete ui;
}

// ===== UPDATE CLOCK =====
void doctorwindow::updateDateTime()
{
    QDateTime now = QDateTime::currentDateTime();
    QString text = now.toString("ddd MMM d yyyy, hh:mm AP");
    ui->label_clock->setText(text);
}

// ===== SWITCH PAGE =====
void doctorwindow::switchPage()
{
    QPushButton *btn = qobject_cast<QPushButton *>(sender());
    if(btn == ui->overviewBtn) {
        ui->stackedWidget->setCurrentIndex(0);
        patientMgr->reload();
        apptMgr->reload();
        populatePatientList();
        populateAppointmentsTable();
        setupStatsSection();
    } else if(btn == ui->patientlistBtn) {
        ui->stackedWidget->setCurrentIndex(1);
        patientMgr->reload();
        refreshPatientList();
    } else if(btn == ui->scheduleBtn) {
        ui->stackedWidget->setCurrentIndex(2);
        apptMgr->reload();
        refreshSchedulingTable();
    }
}

// ===== LOGOUT =====
void doctorwindow::logout()
{
    this->close();
    MainWindow *login = new MainWindow();
    login->show();
}

// ===== SETUP STATS SECTION =====
void doctorwindow::setupStatsSection()
{
    QLayout *oldLayout = ui->statsContainer->layout();
    if (oldLayout) {
        QLayoutItem *item;
        while ((item = oldLayout->takeAt(0)) != nullptr) {
            if(item->widget()) {
                delete item->widget();
            }
            delete item;
        }
        delete oldLayout;
    }

    QHBoxLayout *newLayout = new QHBoxLayout(ui->statsContainer);
    newLayout->setSpacing(15);
    newLayout->setContentsMargins(0, 0, 0, 0);

    // Get real data from PatientManager
    patientMgr->reload();
    apptMgr->reload();

    QVector<Patient> myPatients;
    for (const auto &p : patientMgr->getAllPatients()) {
        if (p.assignedDoctor.trimmed().compare(currentUserName.trimmed(), Qt::CaseInsensitive) == 0)
            myPatients.append(p);
    }

    int totalPatients = myPatients.size();
    int admitted = 0, discharged = 0;
    for (const auto &p : myPatients) {
        if (p.status.trimmed().compare("Discharged", Qt::CaseInsensitive) == 0)
            discharged++;
        else
            admitted++;
    }

    // Get today's appointments for this doctor
    int todaysAppointments = 0;
    int pendingTreatments = 0;
    for (const auto &a : apptMgr->getTodaysAppointments()) {
        if (a.doctorName.trimmed().compare(currentUserName.trimmed(), Qt::CaseInsensitive) == 0) {
            todaysAppointments++;
            if (a.status.trimmed().compare("Pending", Qt::CaseInsensitive) == 0 ||
                a.status.trimmed().compare("Waiting", Qt::CaseInsensitive) == 0) {
                pendingTreatments++;
            }
        }
    }

    // 4 Stats with accent colors
    struct StatData {
        QString number;
        QString label;
        QString subtitle;
        QString accentColor;
    };

    QList<StatData> stats = {
        {QString::number(totalPatients), "Total Patients", "Under your care", "#1a73e8"},
        {QString::number(todaysAppointments), "Today's Appointments", "Scheduled", "#1e8449"},
        {QString::number(pendingTreatments), "Pending Treatments", "Need action", "#a04000"},
        {QString::number(discharged), "Completed Today", "Discharged", "#6c3483"}
    };

    for (const StatData &data : stats) {
        QWidget *card = new QWidget();
        card->setObjectName("statCard");
        card->setStyleSheet(
            QString(
                "QWidget#statCard {"
                "   background-color: #ffffff;"
                "   border: 1px solid #e9ecef;"
                "   border-left: 4px solid %1;"
                "   border-radius: 8px;"
                "   padding: 15px;"
                "   min-width: 150px;"
                "   min-height: 100px;"
                "}"
                ).arg(data.accentColor)
            );

        QVBoxLayout *cardLayout = new QVBoxLayout(card);
        cardLayout->setAlignment(Qt::AlignCenter);
        cardLayout->setSpacing(3);

        QLabel *numberLabel = new QLabel(data.number);
        numberLabel->setStyleSheet(
            "font-size: 28px;"
            "font-weight: bold;"
            "color: #2c3e50;"
            );
        numberLabel->setAlignment(Qt::AlignCenter);

        QLabel *labelText = new QLabel(data.label);
        labelText->setStyleSheet(
            "font-size: 13px;"
            "color: #7f8c8d;"
            );
        labelText->setAlignment(Qt::AlignCenter);

        QLabel *subtitleLabel = new QLabel(data.subtitle);
        subtitleLabel->setStyleSheet(
            "font-size: 11px;"
            "color: #95a5a6;"
            );
        subtitleLabel->setAlignment(Qt::AlignCenter);

        cardLayout->addWidget(numberLabel);
        cardLayout->addWidget(labelText);
        cardLayout->addWidget(subtitleLabel);

        ui->statsContainer->layout()->addWidget(card);
    }
}

// ===== POPULATE PATIENT LIST (Overview Page) =====
void doctorwindow::populatePatientList()
{
    patientMgr->reload();
    ui->patientListWidget->clear();

    for (const auto &p : patientMgr->getAllPatients()) {
        if (p.assignedDoctor.trimmed().compare(currentUserName.trimmed(), Qt::CaseInsensitive) != 0)
            continue;

        // Create custom widget with View button
        QWidget *itemWidget = new QWidget();
        QHBoxLayout *itemLayout = new QHBoxLayout(itemWidget);
        itemLayout->setContentsMargins(5, 5, 5, 5);
        itemLayout->setSpacing(10);

        QString displayText = QString("%1 — %2 (%3)")
                                  .arg(p.name, p.diagnosisTreatment, p.status);

        QLabel *infoLabel = new QLabel(displayText);
        infoLabel->setStyleSheet(
            "font-size: 13px;"
            "color: #2c3e50;"
            "font-weight: 500;"
            );
        infoLabel->setFixedWidth(250);

        QPushButton *viewBtn = new QPushButton("👁️ View");
        viewBtn->setObjectName("viewBtn");
        viewBtn->setStyleSheet(
            "QPushButton#viewBtn {"
            "   background-color: #e8f0fe;"
            "   color: #1a73e8;"
            "   border: none;"
            "   border-radius: 6px;"
            "   padding: 6px 20px;"
            "   font-size: 12px;"
            "   font-weight: 500;"
            "}"
            "QPushButton#viewBtn:hover {"
            "   background-color: #d2e3fc;"
            "}"
            );
        viewBtn->setProperty("patientId", p.id);

        connect(viewBtn, &QPushButton::clicked, this, [this, p]() {
            viewPatientDetails(p.id);
        });

        itemLayout->addWidget(infoLabel);
        itemLayout->addStretch();
        itemLayout->addWidget(viewBtn);

        QListWidgetItem *listItem = new QListWidgetItem(ui->patientListWidget);
        listItem->setSizeHint(itemWidget->sizeHint());
        ui->patientListWidget->setItemWidget(listItem, itemWidget);
    }

    if (ui->patientListWidget->count() == 0) {
        ui->patientListWidget->addItem("No patients currently assigned.");
    }
}

// ===== POPULATE APPOINTMENTS TABLE (Overview Page) =====
void doctorwindow::populateAppointmentsTable()
{
    apptMgr->reload();

    ui->appointmentsTable->clearContents();
    ui->appointmentsTable->setRowCount(0);
    ui->appointmentsTable->setColumnCount(4);
    ui->appointmentsTable->setHorizontalHeaderLabels(
        QStringList() << "Time" << "Patient" << "Reason" << "Status"
        );

    ui->appointmentsTable->setColumnWidth(0, 90);
    ui->appointmentsTable->setColumnWidth(1, 130);
    ui->appointmentsTable->setColumnWidth(2, 150);
    ui->appointmentsTable->setColumnWidth(3, 90);

    QVector<Appointment> todaysForDoctor;
    for (const auto &a : apptMgr->getTodaysAppointments()) {
        if (a.doctorName.trimmed().compare(currentUserName.trimmed(), Qt::CaseInsensitive) == 0)
            todaysForDoctor.append(a);
    }

    for (int row = 0; row < todaysForDoctor.size(); row++) {
        const Appointment &a = todaysForDoctor[row];
        ui->appointmentsTable->insertRow(row);

        QTableWidgetItem *timeItem = new QTableWidgetItem(a.time);
        QTableWidgetItem *patientItem = new QTableWidgetItem(a.patientName);
        QTableWidgetItem *reasonItem = new QTableWidgetItem(a.reason);
        QTableWidgetItem *statusItem = new QTableWidgetItem(a.status);

        timeItem->setTextAlignment(Qt::AlignCenter);
        patientItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        reasonItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        statusItem->setTextAlignment(Qt::AlignCenter);

        QString status = a.status.trimmed();
        if (status.compare("Confirmed", Qt::CaseInsensitive) == 0) {
            statusItem->setBackground(QColor("#eafaf1"));
            statusItem->setForeground(QColor("#1e8449"));
            statusItem->setText("Confirmed");
        } else if (status.compare("Waiting", Qt::CaseInsensitive) == 0 ||
                   status.compare("Pending", Qt::CaseInsensitive) == 0) {
            statusItem->setBackground(QColor("#fdf2e9"));
            statusItem->setForeground(QColor("#a04000"));
            statusItem->setText("Pending");
        } else if (status.compare("Now", Qt::CaseInsensitive) == 0) {
            statusItem->setBackground(QColor("#fef9e7"));
            statusItem->setForeground(QColor("#b7950b"));
            statusItem->setText("Now");
        } else if (status.compare("Cancelled", Qt::CaseInsensitive) == 0) {
            statusItem->setForeground(QColor("#e74c3c"));
            statusItem->setText("Cancelled");
        }

        ui->appointmentsTable->setItem(row, 0, timeItem);
        ui->appointmentsTable->setItem(row, 1, patientItem);
        ui->appointmentsTable->setItem(row, 2, reasonItem);
        ui->appointmentsTable->setItem(row, 3, statusItem);
    }

    ui->appointmentsTable->verticalHeader()->setVisible(false);
    ui->appointmentsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->appointmentsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->appointmentsTable->setSelectionMode(QAbstractItemView::SingleSelection);

    if (ui->appointmentsTable->rowCount() > 0) {
        ui->appointmentsTable->selectRow(0);
    }
}

// ===== SETUP OVERVIEW LAYOUT (GRID) =====
void doctorwindow::setupOverviewLayout()
{
    QWidget *overviewPage = ui->pageOverview;
    if (!overviewPage) return;

    // Remove existing layout if any
    QLayout *oldLayout = overviewPage->layout();
    if (oldLayout) {
        delete oldLayout;
    }

    // Create grid layout
    QGridLayout *gridLayout = new QGridLayout(overviewPage);
    gridLayout->setSpacing(15);
    gridLayout->setContentsMargins(20, 10, 20, 10);

    // Row 0: Stats (spanning 2 columns)
    if (ui->statsContainer->parent() != overviewPage) {
        ui->statsContainer->setParent(overviewPage);
    }
    gridLayout->addWidget(ui->statsContainer, 0, 0, 1, 2);

    // Row 1: Titles
    if (ui->lblPatientsTitle->parent() != overviewPage) {
        ui->lblPatientsTitle->setParent(overviewPage);
    }
    if (ui->lblAppointmentsTtitle->parent() != overviewPage) {
        ui->lblAppointmentsTtitle->setParent(overviewPage);
    }

    ui->lblPatientsTitle->setText("My Patients");
    ui->lblPatientsTitle->setStyleSheet(
        "font-size: 16px;"
        "font-weight: bold;"
        "color: #2c3e50;"
        "padding: 10px 0px 5px 0px;"
        );
    gridLayout->addWidget(ui->lblPatientsTitle, 1, 0);

    ui->lblAppointmentsTtitle->setText("Today's Appointments");
    ui->lblAppointmentsTtitle->setStyleSheet(
        "font-size: 16px;"
        "font-weight: bold;"
        "color: #2c3e50;"
        "padding: 10px 0px 5px 0px;"
        );
    gridLayout->addWidget(ui->lblAppointmentsTtitle, 1, 1);

    // Row 2: Patient List and Appointments Table
    if (ui->patientListWidget->parent() != overviewPage) {
        ui->patientListWidget->setParent(overviewPage);
    }
    if (ui->appointmentsTable->parent() != overviewPage) {
        ui->appointmentsTable->setParent(overviewPage);
    }

    gridLayout->addWidget(ui->patientListWidget, 2, 0);
    gridLayout->addWidget(ui->appointmentsTable, 2, 1);

    // Set column stretches
    gridLayout->setColumnStretch(0, 1);
    gridLayout->setColumnStretch(1, 1);
    gridLayout->setRowStretch(2, 1);
}

// ===== VIEW PATIENT DETAILS =====
void doctorwindow::viewPatientDetails(const QString &patientId)
{
    Patient p = patientMgr->searchPatient(patientId);
    if (p.id.isEmpty()) {
        ui->message->setText("Patient not found.");
        return;
    }

    ui->message->setText(QString("Viewing details for: %1 (ID: %2) — Diagnosis: %3")
                             .arg(p.name, p.id, p.diagnosisTreatment));
}

// ===== PATIENT LIST PAGE =====
void doctorwindow::setupPatientListPage()
{
    QWidget *page = ui->pagePatient;
    if (!page) return;

    if (page->layout()) {
        QLayoutItem *item;
        while ((item = page->layout()->takeAt(0))) {
            if (item->widget()) item->widget()->deleteLater();
            delete item;
        }
        delete page->layout();
    }

    QVBoxLayout *pageLayout = new QVBoxLayout(page);
    pageLayout->setContentsMargins(20, 16, 20, 16);
    pageLayout->setSpacing(12);

    QLabel *title = new QLabel("My Patients");
    title->setStyleSheet("font-size:20px;font-weight:700;color:#0f172a;border:none;background:transparent;");
    pageLayout->addWidget(title);

    patientCountLabel = new QLabel();
    patientCountLabel->setStyleSheet("font-size:13px;color:#64748b;border:none;background:transparent;");
    pageLayout->addWidget(patientCountLabel);

    // Column header bar
    QWidget *colHeader = new QWidget();
    colHeader->setStyleSheet("background-color:#f8fafc;border-bottom:2px solid #e2e8f0;");
    QHBoxLayout *chLayout = new QHBoxLayout(colHeader);
    chLayout->setContentsMargins(16, 8, 16, 8);
    chLayout->setSpacing(10);
    const QString chStyle = "font-weight:700;font-size:11px;color:#94a3b8;" + kPlainLabelD;
    auto makeCol = [&](const QString &txt, int s) {
        QLabel *l = new QLabel(txt); l->setStyleSheet(chStyle);
        chLayout->addWidget(l, s);
    };
    makeCol("PATIENT ID", 1); makeCol("NAME", 2); makeCol("AGE", 1);
    makeCol("GENDER", 1); makeCol("DIAGNOSIS/PROBLEM", 3); makeCol("STATUS", 1);
    pageLayout->addWidget(colHeader);

    QScrollArea *scroll = new QScrollArea();
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setStyleSheet("QScrollArea{border:none;background:transparent;}");

    patientRowContainer = new QWidget();
    patientRowContainer->setStyleSheet("background:transparent;");
    patientRowsLayout = new QVBoxLayout(patientRowContainer);
    patientRowsLayout->setContentsMargins(0, 0, 0, 0);
    patientRowsLayout->setSpacing(0);
    patientRowsLayout->addStretch();

    scroll->setWidget(patientRowContainer);
    pageLayout->addWidget(scroll, 1);

    refreshPatientList();
}

void doctorwindow::refreshPatientList()
{
    if (!patientRowsLayout) return;

    while (patientRowsLayout->count() > 1) {
        QLayoutItem *item = patientRowsLayout->takeAt(0);
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }

    int count = 0;
    for (const auto &p : patientMgr->getAllPatients()) {
        if (p.assignedDoctor.trimmed().compare(currentUserName.trimmed(), Qt::CaseInsensitive) != 0)
            continue;
        addPatientRow(p);
        count++;
    }

    if (patientCountLabel)
        patientCountLabel->setText(QString("Total Patients Assigned: <b>%1</b>").arg(count));
}

void doctorwindow::addPatientRow(const Patient &p)
{
    QFrame *row = new QFrame();
    row->setObjectName("docPatientRow");
    row->setStyleSheet(
        "QFrame#docPatientRow{background-color:#ffffff;border-bottom:1px solid #f1f5f9;}"
        "QFrame#docPatientRow:hover{background-color:#f8fafc;}");

    QHBoxLayout *rl = new QHBoxLayout(row);
    rl->setContentsMargins(16, 10, 16, 10);
    rl->setSpacing(10);

    QLabel *lblId       = new QLabel(p.id);
    QLabel *lblName     = new QLabel(p.name);
    QLabel *lblAge      = new QLabel(p.age);
    QLabel *lblGender   = new QLabel(p.gender);
    QLabel *lblProblem  = new QLabel(p.diagnosisTreatment);
    QLabel *lblStatus   = new QLabel();

    lblId->setStyleSheet("font-weight:bold;color:#64748b;" + kPlainLabelD);
    lblName->setStyleSheet("font-weight:bold;color:#1e293b;" + kPlainLabelD);
    lblAge->setStyleSheet("color:#334155;" + kPlainLabelD);
    lblGender->setStyleSheet("color:#334155;" + kPlainLabelD);
    lblProblem->setStyleSheet("color:#334155;" + kPlainLabelD);
    applyStatusBadgeD(lblStatus, p.status);

    rl->addWidget(lblId,      1);
    rl->addWidget(lblName,    2);
    rl->addWidget(lblAge,     1);
    rl->addWidget(lblGender,  1);
    rl->addWidget(lblProblem, 3);
    rl->addWidget(lblStatus,  1);
    row->setLayout(rl);

    patientRowsLayout->insertWidget(patientRowsLayout->count() - 1, row);
}

// ===== SCHEDULING PAGE =====
void doctorwindow::setupSchedulingPage()
{
    QWidget *page = ui->pageScheduling;
    if (!page) return;

    if (page->layout()) {
        QLayoutItem *item;
        while ((item = page->layout()->takeAt(0))) {
            if (item->widget()) item->widget()->deleteLater();
            delete item;
        }
        delete page->layout();
    }

    QVBoxLayout *pageLayout = new QVBoxLayout(page);
    pageLayout->setContentsMargins(20, 16, 20, 16);
    pageLayout->setSpacing(12);

    QLabel *title = new QLabel("My Schedule");
    title->setStyleSheet("font-size:20px;font-weight:700;color:#0f172a;border:none;background:transparent;");
    pageLayout->addWidget(title);

    scheduleCountLabel = new QLabel();
    scheduleCountLabel->setStyleSheet("font-size:13px;color:#64748b;border:none;background:transparent;");
    pageLayout->addWidget(scheduleCountLabel);

    QWidget *colHeader = new QWidget();
    colHeader->setStyleSheet("background-color:#f8fafc;border-bottom:2px solid #e2e8f0;");
    QHBoxLayout *chLayout = new QHBoxLayout(colHeader);
    chLayout->setContentsMargins(16, 8, 16, 8);
    chLayout->setSpacing(10);
    const QString chStyle = "font-weight:700;font-size:11px;color:#94a3b8;" + kPlainLabelD;
    auto makeCol = [&](const QString &txt, int s) {
        QLabel *l = new QLabel(txt); l->setStyleSheet(chStyle);
        chLayout->addWidget(l, s);
    };
    makeCol("DATE", 2); makeCol("TIME", 1); makeCol("PATIENT", 2);
    makeCol("DEPARTMENT", 2); makeCol("REASON", 3); makeCol("STATUS", 1);
    pageLayout->addWidget(colHeader);

    QScrollArea *scroll = new QScrollArea();
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setStyleSheet("QScrollArea{border:none;background:transparent;}");

    scheduleRowContainer = new QWidget();
    scheduleRowContainer->setStyleSheet("background:transparent;");
    scheduleRowsLayout = new QVBoxLayout(scheduleRowContainer);
    scheduleRowsLayout->setContentsMargins(0, 0, 0, 0);
    scheduleRowsLayout->setSpacing(0);
    scheduleRowsLayout->addStretch();

    scroll->setWidget(scheduleRowContainer);
    pageLayout->addWidget(scroll, 1);

    refreshSchedulingTable();
}

void doctorwindow::refreshSchedulingTable()
{
    if (!scheduleRowsLayout) return;

    while (scheduleRowsLayout->count() > 1) {
        QLayoutItem *item = scheduleRowsLayout->takeAt(0);
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }

    QVector<Appointment> mine = apptMgr->getAppointmentsForDoctor(currentUserName);
    for (const auto &a : mine)
        addScheduleRow(a);

    if (scheduleCountLabel)
        scheduleCountLabel->setText(QString("Total Appointments: <b>%1</b>").arg(mine.size()));
}

void doctorwindow::addScheduleRow(const Appointment &a)
{
    QFrame *row = new QFrame();
    row->setObjectName("docScheduleRow");
    row->setStyleSheet(
        "QFrame#docScheduleRow{background-color:#ffffff;border-bottom:1px solid #f1f5f9;}"
        "QFrame#docScheduleRow:hover{background-color:#f8fafc;}");

    QHBoxLayout *rl = new QHBoxLayout(row);
    rl->setContentsMargins(16, 10, 16, 10);
    rl->setSpacing(10);

    QLabel *lblDate   = new QLabel(a.date);
    QLabel *lblTime   = new QLabel(a.time);
    QLabel *lblPat    = new QLabel(a.patientName);
    QLabel *lblDept   = new QLabel(a.department);
    QLabel *lblReason = new QLabel(a.reason);
    QLabel *lblStatus = new QLabel();

    lblDate->setStyleSheet("font-weight:bold;color:#64748b;" + kPlainLabelD);
    lblTime->setStyleSheet("color:#334155;" + kPlainLabelD);
    lblPat->setStyleSheet("font-weight:bold;color:#1e293b;" + kPlainLabelD);
    lblDept->setStyleSheet("color:#334155;" + kPlainLabelD);
    lblReason->setStyleSheet("color:#334155;" + kPlainLabelD);
    applyStatusBadgeD(lblStatus, a.status);

    rl->addWidget(lblDate,   2);
    rl->addWidget(lblTime,   1);
    rl->addWidget(lblPat,    2);
    rl->addWidget(lblDept,   2);
    rl->addWidget(lblReason, 3);
    rl->addWidget(lblStatus, 1);
    row->setLayout(rl);

    scheduleRowsLayout->insertWidget(scheduleRowsLayout->count() - 1, row);
}