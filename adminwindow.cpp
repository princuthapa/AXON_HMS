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

// ============================================================================
// SLEEK MODAL DIALOG FOR MODIFYING FIELDS (POP-UP)
// ============================================================================
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
        setWindowTitle("Modify Patient Record - " + id);
        setMinimumWidth(360);

        // Modal Window Styling
        setStyleSheet(
            "QDialog { background-color: #ffffff; border-radius: 8px; }"
            "QLabel { font-weight: bold; color: #334155; font-size: 12px; }"
            "QLineEdit, QComboBox { padding: 6px; border: 1px solid #cbd5e1; border-radius: 4px; background: #f8fafc; color: #0f172a; }"
            "QLineEdit:focus, QComboBox:focus { border: 1px solid #6366f1; background: #ffffff; }"
            "QPushButton { padding: 6px 14px; font-weight: bold; border-radius: 4px; font-size: 12px; }"
            );

        QFormLayout *formLayout = new QFormLayout(this);
        formLayout->setContentsMargins(20, 20, 20, 20);
        formLayout->setSpacing(12);

        // Input Fields
        nameEdit = new QLineEdit(name, this);

        genderBox = new QComboBox(this);
        genderBox->addItems({"Male", "Female", "Other"});
        genderBox->setCurrentText(gender);

        problemEdit = new QLineEdit(problem, this);
        doctorEdit = new QLineEdit(doctor, this);

        statusBox = new QComboBox(this);
        statusBox->addItems({"Checked In", "No Show", "Completed"});

        // Cleaned up match selection loop safely matching baseline strings without 'toTitleCase'
        QString currentStatusCleaned = status.trimmed().toLower();
        if (currentStatusCleaned == "present") {
            statusBox->setCurrentText("Checked In");
        } else if (currentStatusCleaned == "absent" || currentStatusCleaned == "not present") {
            statusBox->setCurrentText("No Show");
        } else {
            statusBox->setCurrentText(status.trimmed());
        }

        formLayout->addRow("Patient Name:", nameEdit);
        formLayout->addRow("Gender:", genderBox);
        formLayout->addRow("Diagnosis/Problem:", problemEdit);
        formLayout->addRow("Assigned Doctor:", doctorEdit);
        formLayout->addRow("Attendance Status:", statusBox);

        // Dialog Action Buttons
        QHBoxLayout *btnLayout = new QHBoxLayout();
        QPushButton *btnCancel = new QPushButton("Cancel", this);
        QPushButton *btnSave = new QPushButton("Save Changes", this);

        btnCancel->setStyleSheet("background-color: #f1f5f9; color: #475569; border: 1px solid #e2e8f0;");
        btnSave->setStyleSheet("background-color: #0284c7; color: #ffffff; border: none;");

        btnLayout->addStretch();
        btnLayout->addWidget(btnCancel);
        btnLayout->addWidget(btnSave);
        formLayout->addRow(btnLayout);

        connect(btnCancel, &QPushButton::clicked, this, &QDialog::reject);
        connect(btnSave, &QPushButton::clicked, this, &QDialog::accept);
    }
};

// ============================================================================
// MAIN CORE IMPLEMENTATION
// ============================================================================

void embedChart(QWidget* container, QChart* chart) {
    if (!container || !chart) return;

    chart->setBackgroundVisible(false);
    chart->setMargins(QMargins(0, 0, 0, 0));
    chart->legend()->hide();

    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    if (container->layout() == nullptr) {
        QVBoxLayout *layout = new QVBoxLayout(container);
        layout->setContentsMargins(0, 0, 0, 0);
        container->setLayout(layout);
    } else {
        QLayoutItem *item;
        while ((item = container->layout()->takeAt(0)) != nullptr) {
            if (item->widget()) delete item->widget();
            delete item;
        }
    }
    container->layout()->addWidget(chartView);
}

adminwindow::adminwindow(const QString &employeeName, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AXON_ADMIN),
    currentAdminName(employeeName)
{
    ui->setupUi(this);

    QButtonGroup* sidebarGroup = new QButtonGroup(this);
    sidebarGroup->addButton(ui->btnOverview);
    sidebarGroup->addButton(ui->btnStaffManager);
    sidebarGroup->addButton(ui->btnScheduling);
    sidebarGroup->setExclusive(true);

    adminBackend = nullptr;
    staffMgr = nullptr;

    if (ui->stackedWidget) ui->stackedWidget->setCurrentIndex(0);

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &adminwindow::updateDateTime);
    timer->start(1000);
    updateDateTime();

    QPixmap logoPixmap(":/images/axonimg.png");
    if (ui->lblLogo && !logoPixmap.isNull()) {
        ui->lblLogo->setPixmap(logoPixmap.scaled(90, 90, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }

    initDashboardGraphs();
    setupPatientHeader();

    // UI Placeholders / Pure frontend mock data states
    addPatientRow("#PT-0021", "Brandon Vance", "Male", "Cardiac Checkup", "Dr. Ross", "Checked In");
    addPatientRow("#PT-0022", "Clara Thompson", "Female", "Migraine Treatment", "Dr. House", "No Show");
    addPatientRow("#PT-0023", "David Miller", "Male", "Fracture Follow-up", "Dr. Strange", "No Show");
    addPatientRow("#PT-0024", "Emma Watson", "Female", "General Physical", "Dr. Ross", "Checked In");
}

adminwindow::~adminwindow()
{
    delete ui;
    if (adminBackend) delete adminBackend;
    if (staffMgr) delete staffMgr;
}

void adminwindow::updateDateTime()
{
    if (ui->lblDateTime) {
        QDateTime current = QDateTime::currentDateTime();
        ui->lblDateTime->setText(current.toString("ddd MMM d yyyy, hh:mm AP"));
    }
}

void adminwindow::on_btnMenu_clicked()
{
    QMenu menu(this);
    QAction *logoutAction = menu.addAction("Logout");
    QPoint popupPos = ui->btnMenu->mapToGlobal(QPoint(0, ui->btnMenu->height()));
    if (menu.exec(popupPos) == logoutAction) {
        MainWindow *loginScreen = new MainWindow();
        loginScreen->show();
        this->close();
    }
}

void adminwindow::on_btnOverview_clicked() { if (ui->stackedWidget) ui->stackedWidget->setCurrentIndex(0); }
void adminwindow::on_btnStaffManager_clicked() { if (ui->stackedWidget) ui->stackedWidget->setCurrentIndex(1); }
void adminwindow::on_btnScheduling_clicked() { QDesktopServices::openUrl(QUrl("https://calendar.google.com/calendar/u/0/r")); }

void adminwindow::initDashboardGraphs()
{
    ui->btnOverview->setChecked(true);
    on_btnOverview_clicked();

    // 1. STAFF PIE CHART
    QPieSeries *staffSeries = new QPieSeries();
    staffSeries->setPieSize(1.0);
    staffSeries->append("Active", 312)->setBrush(QColor(0x166534));
    staffSeries->append("Remaining", 38)->setBrush(QColor(0xbbf7d0));

    QChart *staffChart = new QChart();
    staffChart->addSeries(staffSeries);
    if (ui->widgetGraphStaff) embedChart(ui->widgetGraphStaff, staffChart);
    if (ui->lblSubtextStaff) {
        ui->lblSubtextStaff->setText("<span style='color:#166534;'>●</span> Active &nbsp;<span style='color:#bbf7d0;'>●</span> Inactive &nbsp;");
    }

    // 2. ACTIVE DOCTORS - Capacity Gauge Donut
    QPieSeries *docGaugeSeries = new QPieSeries();
    docGaugeSeries->setHoleSize(0.75);
    docGaugeSeries->setPieSize(0.95);

    QPieSlice *onShift = docGaugeSeries->append("On Shift", 42);
    onShift->setBrush(QColor(0x0284c7));

    QPieSlice *offShift = docGaugeSeries->append("Vacant", 8);
    offShift->setBrush(QColor(0xf1f5f9));

    QChart *docChart = new QChart();
    docChart->addSeries(docGaugeSeries);

    if (ui->widgetGraphDoctors) {
        embedChart(ui->widgetGraphDoctors, docChart);
        if (ui->lblSubtextDoctors) {
            ui->lblSubtextDoctors->setText("<span style='color:#0284c7;'>●</span> On Shift &nbsp;<span style='color:#f1f5f9;'>●</span> Vacant &nbsp;");
        }
    }

    // 3. PATIENTS SEGMENTED PIE
    QPieSeries *patSeries = new QPieSeries();
    patSeries->setHoleSize(0.35);
    patSeries->append("Male", 450)->setBrush(QColor(0x6366f1));
    patSeries->append("Female", 520)->setBrush(QColor(0xec4899));
    patSeries->append("Child", 165)->setBrush(QColor(0xf59e0b));
    patSeries->append("Elderly", 110)->setBrush(QColor(0x10b981));

    QChart *patChart = new QChart();
    patChart->addSeries(patSeries);
    if (ui->widgetGraphPatients) embedChart(ui->widgetGraphPatients, patChart);
    if (ui->lblSubtextPatients) {
        ui->lblSubtextPatients->setText("<span style='color:#6366f1;'>●</span> Male &nbsp;<span style='color:#ec4899;'>●</span> Female &nbsp;<span style='color:#f59e0b;'>●</span> Child &nbsp;<span style='color:#10b981;'>●</span> Elderly &nbsp;");
    }

    // 4. REVENUE TREND LINE
    QLineSeries *revSeries = new QLineSeries();
    revSeries->append(0, 95000); revSeries->append(1, 110000);
    revSeries->append(2, 125000); revSeries->append(3, 142500);
    revSeries->setPen(QPen(QColor(0x0d9488), 4));

    QChart *revChart = new QChart();
    revChart->addSeries(revSeries);
    revChart->createDefaultAxes();
    if (ui->widgetGraphRevenue) embedChart(ui->widgetGraphRevenue, revChart);

    if (ui->lblSubtextRevenue) {
        ui->lblSubtextRevenue->setText("<span style='color:#0d9488;'>●</span> Revenue &nbsp;<span style='color:#880808;'>●</span> Expense &nbsp;");
    }
}

void adminwindow::setupPatientHeader()
{
    if (!ui->webContainer) return;

    QWidget *headerWidget = new QWidget();
    headerWidget->setObjectName("patientHeaderRow");

    QHBoxLayout *headerLayout = new QHBoxLayout(headerWidget);
    headerLayout->setContentsMargins(15, 5, 15, 5);
    headerLayout->setSpacing(10);

    QLabel *hId = new QLabel("PATIENT ID");
    QLabel *hName = new QLabel("NAME");
    QLabel *hGender = new QLabel("GENDER");
    QLabel *hProblem = new QLabel("PROBLEM / DIAGNOSIS");
    QLabel *hDoctor = new QLabel("ASSIGNED DOCTOR");
    QLabel *hStatus = new QLabel("STATUS");
    QLabel *hAction = new QLabel("ACTION");

    QString headerStyle = "font-weight: bold; font-size: 11px; color: #94a3b8; border: none; background: transparent; background-color: transparent;";
    hId->setStyleSheet(headerStyle);
    hName->setStyleSheet(headerStyle);
    hGender->setStyleSheet(headerStyle);
    hProblem->setStyleSheet(headerStyle);
    hDoctor->setStyleSheet(headerStyle);
    hStatus->setStyleSheet(headerStyle);
    hAction->setStyleSheet(headerStyle);
    hAction->setAlignment(Qt::AlignCenter);

    headerLayout->addWidget(hId, 1);
    headerLayout->addWidget(hName, 2);
    headerLayout->addWidget(hGender, 1);
    headerLayout->addWidget(hProblem, 2);
    headerLayout->addWidget(hDoctor, 2);
    headerLayout->addWidget(hStatus, 1);
    headerLayout->addWidget(hAction, 1);

    headerWidget->setLayout(headerLayout);

    // Safeguard lookup mapping target container layouts properly
    QVBoxLayout *cardLayout = qobject_cast<QVBoxLayout*>(ui->webContainer->layout());
    if (!cardLayout) {
        cardLayout = new QVBoxLayout(ui->webContainer);
        cardLayout->setContentsMargins(15, 15, 15, 15);
        cardLayout->setSpacing(10);
    }
    cardLayout->insertWidget(0, headerWidget);
}

void adminwindow::addPatientRow(QString id, QString name, QString gender, QString problem, QString doctor, QString status)
{
    if (!ui->webContainer) return;

    QFrame *rowFrame = new QFrame();
    rowFrame->setObjectName("patientRow");
    rowFrame->setStyleSheet("QFrame#patientRow { background-color: #ffffff; border-bottom: 1px solid #f1f5f9; } QFrame#patientRow:hover { background-color: #f8fafc; }");

    QHBoxLayout *rowLayout = new QHBoxLayout(rowFrame);
    rowLayout->setContentsMargins(15, 10, 15, 10);
    rowLayout->setSpacing(10);

    QLabel *lblId = new QLabel(id);
    QLabel *lblName = new QLabel(name);
    QLabel *lblGender = new QLabel(gender);
    QLabel *lblProblem = new QLabel(problem);
    QLabel *lblDoctor = new QLabel(doctor);
    QLabel *lblStatus = new QLabel();
    QPushButton *btnEdit = new QPushButton("Edit");

    QString plainLabelStyle = "border: none; background: transparent; background-color: transparent;";
    lblId->setStyleSheet("font-weight: bold; color: #64748b; " + plainLabelStyle);
    lblName->setStyleSheet("font-weight: bold; color: #1e293b; " + plainLabelStyle);
    lblGender->setStyleSheet("color: #334155; " + plainLabelStyle);
    lblProblem->setStyleSheet("color: #334155; " + plainLabelStyle);
    lblDoctor->setStyleSheet("color: #334155; " + plainLabelStyle);

    auto updateStatusBadge = [](QLabel* label, const QString& currentStatus) {
        label->setText(currentStatus.toUpper());
        label->setAlignment(Qt::AlignCenter);

        QString normStatus = currentStatus.toLower().trimmed();
        if (normStatus == "present" || normStatus == "checked in") {
            label->setStyleSheet("background-color: #dcfce7; color: #15803d; border-radius: 6px; font-weight: bold; font-size: 11px; max-width: 95px; padding: 4px 8px; border: none;");
        } else if (normStatus == "absent" || normStatus == "not present" || normStatus == "no show") {
            label->setStyleSheet("background-color: #fee2e2; color: #b91c1c; border-radius: 6px; font-weight: bold; font-size: 11px; max-width: 95px; padding: 4px 8px; border: none;");
        } else {
            label->setStyleSheet("background-color: #e0f2fe; color: #0369a1; border-radius: 6px; font-weight: bold; font-size: 11px; max-width: 95px; padding: 4px 8px; border: none;");
        }
    };

    updateStatusBadge(lblStatus, status);

    btnEdit->setStyleSheet(
        "QPushButton { background-color: #f1f5f9; color: #0f172a; border: 1px solid #e2e8f0; border-radius: 6px; font-weight: bold; font-size: 12px; padding: 4px 12px; max-width: 60px; }"
        "QPushButton:hover { background-color: #e2e8f0; }"
        "QPushButton:pressed { background-color: #cbd5e1; }"
        );

    // INTERACTIVE LIVE UI-ONLY MUTATION LOOP
    connect(btnEdit, &QPushButton::clicked, this, [=]() {
        EditPatientDialog dialog(lblId->text(), lblName->text(), lblGender->text(), lblProblem->text(), lblDoctor->text(), lblStatus->text(), this);

        if (dialog.exec() == QDialog::Accepted) {
            // Instantly modifies the UI values in memory
            lblName->setText(dialog.nameEdit->text());
            lblGender->setText(dialog.genderBox->currentText());
            lblProblem->setText(dialog.problemEdit->text());
            lblDoctor->setText(dialog.doctorEdit->text());

            updateStatusBadge(lblStatus, dialog.statusBox->currentText());
        }
    });

    rowLayout->addWidget(lblId, 1);
    rowLayout->addWidget(lblName, 2);
    rowLayout->addWidget(lblGender, 1);
    rowLayout->addWidget(lblProblem, 2);
    rowLayout->addWidget(lblDoctor, 2);
    rowLayout->addWidget(lblStatus, 1);
    rowLayout->addWidget(btnEdit, 1);

    rowFrame->setLayout(rowLayout);

    QVBoxLayout *containerLayout = qobject_cast<QVBoxLayout*>(ui->webContainer->layout());
    if (containerLayout) {
        containerLayout->addWidget(rowFrame);
    }
}