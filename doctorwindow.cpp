#include "doctorwindow.h"
#include "ui_doctorwindow.h"
#include "mainwindow.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
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

static const QString kPlainLabelD =
    "border:none;background:transparent;background-color:transparent;";

static void applyStatusBadgeD(QLabel *label, const QString &status) {
    label->setText(status.toUpper());
    label->setAlignment(Qt::AlignCenter);
    label->setStyleSheet("font-weight:bold;font-size:11px;padding:4px 8px;border:none;"
                          "background-color:#e8f0fe;color:#1a4f8a;border-radius:10px;");
}

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

    //Clock
    timer= new QTimer(this);
    connect(timer, &QTimer::timeout, this, &doctorwindow::updateDateTime);
    updateDateTime();
    timer->start(1000);

    //SideBar Navigation
    connect(ui->overviewBtn, &QPushButton::clicked, this, &doctorwindow::switchPage);
    connect(ui->patientlistBtn, &QPushButton::clicked, this, &doctorwindow::switchPage);
    connect(ui->scheduleBtn, &QPushButton::clicked, this, &doctorwindow::switchPage);
    ui->stackedWidget->setCurrentIndex(0);

    //Logout Menu
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

    setupStatsSection();
    setupOverviewPage();
    setupPatientListPage();
    setupSchedulingPage();

    doctorBackend = new Doctor();

    this->setWindowTitle("AXON-HMS: Doctor Dashboard");
}

doctorwindow::~doctorwindow()
{
    delete doctorBackend;
    delete patientMgr;
    delete apptMgr;
    delete ui;
}

void doctorwindow::updateDateTime()
{
    QDateTime now=QDateTime::currentDateTime();
    QString text=now.toString("ddd MMM d yyyy, hh:mm AP");
    ui->label_clock->setText(text);
}

void doctorwindow::switchPage()
{
    QPushButton *btn = qobject_cast<QPushButton *>(sender());
    if(btn==ui->overviewBtn)
    {
        ui->stackedWidget->setCurrentIndex(0);
        patientMgr->reload();
        apptMgr->reload();
        setupOverviewPage();
    }

    else if(btn==ui->patientlistBtn)
    {
        ui->stackedWidget->setCurrentIndex(1);
        patientMgr->reload();
        refreshPatientList();
    }

    else if(btn==ui->scheduleBtn)
    {
        ui->stackedWidget->setCurrentIndex(2);
        apptMgr->reload();
        refreshSchedulingTable();
    }
}

void doctorwindow::logout()
{
    this->close();
    MainWindow *login =new MainWindow();
    login->show();
}

void doctorwindow::setupStatsSection()
{

    QLayout *oldLayout = ui->statsContainer->layout();
    if (oldLayout) {
        QLayoutItem *item;
        while ((item = oldLayout->takeAt(0)) != nullptr) {
            if(item->widget())
            {
                delete item->widget();
            }
            delete item;
        }
        delete oldLayout;
    }

    QHBoxLayout *newLayout = new QHBoxLayout(ui->statsContainer);
    newLayout->setSpacing(15);
    newLayout->setContentsMargins(0, 0, 0, 0);

    // Pull real numbers for THIS doctor from the shared patient database
    patientMgr->reload();
    QVector<Patient> myPatients;
    for (const auto &p : patientMgr->getAllPatients()) {
        if (p.assignedDoctor.trimmed().compare(currentUserName.trimmed(), Qt::CaseInsensitive) == 0)
            myPatients.append(p);
    }
    int totalPatients = myPatients.size();
    int admitted = 0, discharged = 0;
    for (const auto &p : myPatients) {
        if (p.status.trimmed().compare("Discharged", Qt::CaseInsensitive) == 0) discharged++;
        else admitted++;
    }

    // Create the stats data
    struct StatData {
        QString number;
        QString label;
        QString trend;
        bool isUp;
    };

    QList<StatData> stats = {
        {QString::number(totalPatients), "Total Patients", "", true},
        {QString::number(admitted),      "Currently Admitted", "", true},
        {QString::number(discharged),    "Discharged", "", false}
    };

    // Each stat card
    for (const StatData &data : stats) {
        QWidget *card = new QWidget();
        card->setObjectName("statCard");
        card->setStyleSheet(
            "QWidget#statCard {"
            "   background-color: #ffffff;"
            "   border: 1px solid #e9ecef;"
            "   border-radius: 12px;"
            "   padding: 15px;"
            "   min-width: 150px;"
            "   min-height: 100px;"
            "}"
            );

        QVBoxLayout *cardLayout = new QVBoxLayout(card);
        cardLayout->setAlignment(Qt::AlignCenter);
        cardLayout->setSpacing(5);

        // Number label
        QLabel *numberLabel = new QLabel(data.number);
        numberLabel->setStyleSheet(
            "font-size: 32px;"
            "font-weight: bold;"
            "color: #2c3e50;"
            );
        numberLabel->setAlignment(Qt::AlignCenter);

        // Label text
        QLabel *labelText = new QLabel(data.label);
        labelText->setStyleSheet(
            "font-size: 13px;"
            "color: #7f8c8d;"
            );
        labelText->setAlignment(Qt::AlignCenter);

        // Add to layout
        cardLayout->addWidget(numberLabel);
        cardLayout->addWidget(labelText);


        if (!data.trend.isEmpty()) {
            QLabel *trendLabel = new QLabel(data.trend);
            QString trendColor = data.isUp ? "#27ae60" : "#e74c3c";
            trendLabel->setStyleSheet(
                QString("font-size: 12px; color: %1; font-weight: 600;").arg(trendColor)
                );
            trendLabel->setAlignment(Qt::AlignCenter);
            cardLayout->addWidget(trendLabel);
        }

        // Add card to container
        ui->statsContainer->layout()->addWidget(card);
    }
}

void doctorwindow::setupOverviewPage()
{

    setupStatsSection();

    ui->lblPatientsTitle->setText("My Patients");
    ui->lblPatientsTitle->setStyleSheet(
        "font-size: 16px;"
        "font-weight: bold;"
        "color: #2c3e50;"
        "padding: 5px 0px 10px 0px;"
        );

    // Real patients assigned to this doctor (same patient_database.csv
    // Admin & Receptionist use — no more hard-coded mock names)
    patientMgr->reload();
    ui->patientListWidget->clear();
    for (const auto &p : patientMgr->getAllPatients()) {
        if (p.assignedDoctor.trimmed().compare(currentUserName.trimmed(), Qt::CaseInsensitive) != 0)
            continue;
        QString line = QString("%1 — %2 (%3)")
                           .arg(p.name, p.diagnosisTreatment, p.status);
        ui->patientListWidget->addItem(line);
    }
    if (ui->patientListWidget->count() == 0)
        ui->patientListWidget->addItem("No patients currently assigned.");

    ui->patientListWidget->setStyleSheet(
        "QListWidget {"
        "   background-color: #ffffff;"
        "   border: 1px solid #e9ecef;"
        "   border-radius: 8px;"
        "   padding: 5px;"
        "}"
        "QListWidget::item {"
        "   padding: 10px;"
        "   border-bottom: 1px solid #f0f0f0;"
        "}"
        "QListWidget::item:selected {"
        "   background-color: #e8f0fe;"
        "   color: #1a73e8;"
        "}"
        );

    ui->lblAppointmentsTtitle->setText("Today's Appointments");
    ui->lblAppointmentsTtitle->setStyleSheet(
        "font-size: 16px;"
        "font-weight: bold;"
        "color: #2c3e50;"
        "padding: 10px 0px;"
        );

    // The table
    ui->appointmentsTable->clearContents();
    ui->appointmentsTable->setRowCount(0);
    ui->appointmentsTable->setColumnCount(4);
    ui->appointmentsTable->setHorizontalHeaderLabels(
        QStringList() << "Time" << "Patient" << "Reason" << "Status"
        );

    ui->appointmentsTable->setColumnWidth(0, 90);
    ui->appointmentsTable->setColumnWidth(1, 130);
    ui->appointmentsTable->setColumnWidth(2, 140);
    ui->appointmentsTable->setColumnWidth(3, 90);

    // Real appointments for this doctor, filtered to today, from the
    // shared appointment_database.csv (replaces the TeamUp calendar embed)
    apptMgr->reload();
    QVector<Appointment> todaysForDoctor;
    for (const auto &a : apptMgr->getTodaysAppointments()) {
        if (a.doctorName.trimmed().compare(currentUserName.trimmed(), Qt::CaseInsensitive) == 0)
            todaysForDoctor.append(a);
    }

    for (int row = 0; row < todaysForDoctor.size(); row++) {
        const Appointment &a = todaysForDoctor[row];
        ui->appointmentsTable->insertRow(row);
        QTableWidgetItem *timeItem    = new QTableWidgetItem(a.time);
        QTableWidgetItem *patientItem = new QTableWidgetItem(a.patientName);
        QTableWidgetItem *reasonItem  = new QTableWidgetItem(a.reason);
        QTableWidgetItem *statusItem  = new QTableWidgetItem(a.status);

        QString status = a.status.trimmed();
        if (status.compare("Confirmed", Qt::CaseInsensitive) == 0) {
            statusItem->setForeground(QColor("#27ae60"));
        } else if (status.compare("Waiting", Qt::CaseInsensitive) == 0 ||
                   status.compare("Pending", Qt::CaseInsensitive) == 0) {
            statusItem->setBackground(QColor("#fff3cd"));
            statusItem->setForeground(QColor("#856404"));
        } else if (status.compare("Cancelled", Qt::CaseInsensitive) == 0) {
            statusItem->setForeground(QColor("#e74c3c"));
        }

        ui->appointmentsTable->setItem(row, 0, timeItem);
        ui->appointmentsTable->setItem(row, 1, patientItem);
        ui->appointmentsTable->setItem(row, 2, reasonItem);
        ui->appointmentsTable->setItem(row, 3, statusItem);
    }

    // Style the table
    ui->appointmentsTable->setStyleSheet(
        "QTableWidget {"
        "   background-color: #ffffff;"
        "   border: 1px solid #e9ecef;"
        "   border-radius: 8px;"
        "   gridline-color: #f0f0f0;"
        "}"
        "QTableWidget::item {"
        "   padding: 8px;"
        "}"
        "QHeaderView::section {"
        "   background-color: #f8f9fa;"
        "   padding: 8px;"
        "   border: none;"
        "   font-weight: bold;"
        "   color: #2c3e50;"
        "}"
        );

    // Hide row numbers and make read-only
    ui->appointmentsTable->verticalHeader()->setVisible(false);
    ui->appointmentsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    if (ui->appointmentsTable->rowCount() > 0)
        ui->appointmentsTable->selectRow(0);
}

//  Patient List page
// Builds a scrollable roster of every patient assigned to this doctor,
// read live from the same patient_database.csv Admin/Receptionist use.

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

    // Scrollable rows area
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
