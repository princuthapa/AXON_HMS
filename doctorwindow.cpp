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
#include <QWebEngineView>

doctorwindow::doctorwindow(const QString &employeeName, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::doctorwindow),
    currentUserName(employeeName)
{
    ui->setupUi(this);
    if (ui->message)
        ui->message->setText(QString("Welcome, Dr. %1!").arg(currentUserName));

    //Clock
    timer= new QTimer(this);
    connect(timer, &QTimer::timeout, this, &doctorwindow::updateDateTime);
    updateDateTime();
    timer->start(1000);

    //SideBar Navigation
    connect(ui->overviewBtn, &QPushButton::clicked, this, &doctorwindow::switchPage);
    connect(ui->patientlistBtn, &QPushButton::clicked, this, &doctorwindow::switchPage);
    // connect(ui->scheduleBtn, &QPushButton::clicked, this, &doctorwindow::switchPage);
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

    doctorBackend = new Doctor();

    this->setWindowTitle("AXON-HMS: Doctor Dashboard");
}

doctorwindow::~doctorwindow()
{
    delete doctorBackend;
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
    }

    else if(btn==ui->patientlistBtn)
    {
        ui->stackedWidget->setCurrentIndex(1);
    }

    else if(btn==ui->scheduleBtn)
    {
        ui->stackedWidget->setCurrentIndex(2);
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

    // Create the stats data
    struct StatData {
        QString number;
        QString label;
        QString trend;
        bool isUp;
    };

    QList<StatData> stats = {
        {"990", "Total Patients", "", true},
        {"67", "New Patients", "↓ 39% vs last month", false},
        {"27", "Returning Patients", "↓ 4% vs last month", false}
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

    ui->lblPatientsTitle->setText("Patients Today");
    ui->lblPatientsTitle->setStyleSheet(
        "font-size: 16px;"
        "font-weight: bold;"
        "color: #2c3e50;"
        "padding: 5px 0px 10px 0px;"
        );

    ui->patientListWidget->clear();
    ui->patientListWidget->addItem("Sarah Hostem - Bronchitis (10:30 am)");
    ui->patientListWidget->addItem("John Smith - Flu (11:00 am)");
    ui->patientListWidget->addItem("Maria Rivera - Checkup (1:00 pm)");
    ui->patientListWidget->addItem("David Chen - Follow-up (2:30 pm)");

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

    // The tabel
    ui->appointmentsTable->setColumnCount(5);
    ui->appointmentsTable->setHorizontalHeaderLabels(
        QStringList() << "Time" << "Patient" << "Reason" << "Amount" << "Status"
        );


    ui->appointmentsTable->setColumnWidth(0, 80);
    ui->appointmentsTable->setColumnWidth(1, 120);
    ui->appointmentsTable->setColumnWidth(2, 150);
    ui->appointmentsTable->setColumnWidth(3, 70);
    ui->appointmentsTable->setColumnWidth(4, 90);

    QStringList times = {"10:00 AM", "10:30 AM", "11:00 AM", "1:00 PM"};
    QStringList patients = {"Shawn Hampton", "Polly Paul", "John Doe", "Maria Rivera"};
    QStringList reasons = {"Emergency", "USG Consultation", "Laboratory Screening", "General Checkup"};
    QStringList amounts = {"$30", "$50", "$70", "$25"};
    QStringList statuses = {"Now", "Confirmed", "Confirmed", "Pending"};

    for (int row = 0; row < 4; row++) {
        ui->appointmentsTable->insertRow(row);
        QTableWidgetItem *timeItem = new QTableWidgetItem(times[row]);
        QTableWidgetItem *patientItem = new QTableWidgetItem(patients[row]);
        QTableWidgetItem *reasonItem = new QTableWidgetItem(reasons[row]);
        QTableWidgetItem *amountItem = new QTableWidgetItem(amounts[row]);
        QTableWidgetItem *statusItem = new QTableWidgetItem(statuses[row]);

        // Color-code status column
        QString status = statuses[row];
        if (status == "Now") {
            statusItem->setBackground(QColor("#fff3cd"));
            statusItem->setForeground(QColor("#856404"));
        } else if (status == "Confirmed") {
            statusItem->setForeground(QColor("#27ae60"));
        } else if (status == "Pending") {
            statusItem->setForeground(QColor("#f39c12"));
        }

        // Add to table
        ui->appointmentsTable->setItem(row, 0, timeItem);
        ui->appointmentsTable->setItem(row, 1, patientItem);
        ui->appointmentsTable->setItem(row, 2, reasonItem);
        ui->appointmentsTable->setItem(row, 3, amountItem);
        ui->appointmentsTable->setItem(row, 4, statusItem);
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
ui->appointmentsTable->selectRow(0);

}
void doctorwindow::on_scheduleBtn_clicked()
{


        ui->stackedWidget->setGeometry(165, 11, 1104, 698);

        QWebEngineView *webView = ui->stackedWidget->findChild<QWebEngineView*>();
        if (!webView) {
            webView = new QWebEngineView(ui->stackedWidget);
            webView->setUrl(QUrl("https://teamup.com/c/vvud1m/axon-hms"));
            ui->stackedWidget->addWidget(webView);
        } else {
            webView->setUrl(QUrl("https://teamup.com/c/vvud1m/axon-hms"));
        }
        ui->stackedWidget->setCurrentWidget(webView);

}

