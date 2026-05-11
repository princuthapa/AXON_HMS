

#include "admin.h"
#include "ui_adminwindow.h"   // generated from Forms/adminwindow.ui

#include <QDateTime>
#include <QFile>
#include <QTextStream>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QApplication>

// ════════════════════════════════════════════════════════════════════
//  Constructor / Destructor
// ════════════════════════════════════════════════════════════════════
admin::admin(QWidget *loginWindow, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::admin)
    , m_loginWindow(loginWindow)
{
    ui->setupUi(this);

    // ── Table column widths ──────────────────────────────────────
    QHeaderView *hh = ui->staffTable->horizontalHeader();
    hh->setSectionResizeMode(QHeaderView::Interactive);
    hh->setDefaultSectionSize(120);
    ui->staffTable->setColumnWidth(0, 110);   // Employee ID
    ui->staffTable->setColumnWidth(1, 140);   // Name
    ui->staffTable->setColumnWidth(2, 140);   // Specialty
    ui->staffTable->setColumnWidth(3, 120);   // Department
    ui->staffTable->setColumnWidth(4, 80);    // Shift
    ui->staffTable->setColumnWidth(5, 120);   // Current Patient
    ui->staffTable->setColumnWidth(6, 110);   // Status
    ui->staffTable->setColumnWidth(7, 80);    // Actions
    ui->staffTable->verticalHeader()->setVisible(false);
    ui->staffTable->verticalHeader()->setDefaultSectionSize(54);

    // ── Clock ────────────────────────────────────────────────────
    m_clockTimer = new QTimer(this);
    connect(m_clockTimer, &QTimer::timeout, this, &admin::updateClock);
    m_clockTimer->start(1000);
    updateClock();

    // ── Load CSV data ────────────────────────────────────────────
    // Try Qt resources first (:/database/), fall back to working dir
    QString staffPath  = QFile::exists(":/database/staff.csv")
                            ? ":/database/staff.csv"  : "staff.csv";
    QString bedPath    = QFile::exists(":/database/bedstats.csv")
                          ? ":/database/bedstats.csv" : "bedstats.csv";

    loadStaffCSV(staffPath);
    loadBedCSV(bedPath);

    // ── Build UI from data ───────────────────────────────────────
    buildBedDeptCards();
    m_filteredStaff = m_allStaff;
    applyPage(1);

    ui->staffCountLabel->setText(QString::number(m_allStaff.size()));

    // ── Signal / Slot wiring ─────────────────────────────────────
    connect(ui->staffSearchInput, &QLineEdit::textChanged,
            this, &admin::onSearchTextChanged);
    connect(ui->prevPageBtn,      &QPushButton::clicked, this, &admin::onPrevPage);
    connect(ui->nextPageBtn,      &QPushButton::clicked, this, &admin::onNextPage);
    connect(ui->addStaffBtn,      &QPushButton::clicked, this, &admin::onAddStaffClicked);
    connect(ui->navDashboardBtn,  &QPushButton::clicked, this, &admin::onNavDashboard);
    connect(ui->navPatientBtn,    &QPushButton::clicked, this, &admin::onNavPatient);
    connect(ui->navScheduleBtn,   &QPushButton::clicked, this, &admin::onNavSchedule);
    connect(ui->navMedRecordsBtn, &QPushButton::clicked, this, &admin::onNavMedRecords);
    connect(ui->navSettingsBtn,   &QPushButton::clicked, this, &admin::onNavSettings);
    connect(ui->navLogoutBtn,     &QPushButton::clicked, this, &admin::onNavLogout);
}

admin::~admin()
{
    delete ui;
}

// ════════════════════════════════════════════════════════════════════
//  CSV Loaders
// ════════════════════════════════════════════════════════════════════
void admin::loadStaffCSV(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "File Error",
                             QString("Cannot open %1\nAdd staff.csv to resources (:/database/) "
                                     "or place it next to the executable.").arg(filePath));
        return;
    }

    QTextStream in(&file);
    bool firstLine = true;
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;
        if (firstLine) { firstLine = false; continue; }   // skip header

        QStringList cols = line.split(',');
        if (cols.size() < 7) continue;

        StaffRecord r;
        r.employeeId     = cols[0].trimmed();
        r.name           = cols[1].trimmed();
        r.specialty      = cols[2].trimmed();
        r.department     = cols[3].trimmed();
        r.shift          = cols[4].trimmed();
        r.currentPatient = cols[5].trimmed();
        r.status         = cols[6].trimmed();
        m_allStaff.append(r);
    }
    file.close();
}

void admin::loadBedCSV(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "File Error",
                             QString("Cannot open %1\nAdd bedstats.csv to resources (:/database/) "
                                     "or place it next to the executable.").arg(filePath));
        return;
    }

    QTextStream in(&file);
    bool firstLine = true;
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;
        if (firstLine) { firstLine = false; continue; }

        QStringList cols = line.split(',');
        if (cols.size() < 3) continue;

        BedRecord b;
        b.department   = cols[0].trimmed();
        b.bedsOccupied = cols[1].trimmed().toInt();
        b.totalBeds    = cols[2].trimmed().toInt();
        m_bedRecords.append(b);
    }
    file.close();
}

// ════════════════════════════════════════════════════════════════════
//  Bed Occupancy Cards  — all 6 in one horizontal row
// ════════════════════════════════════════════════════════════════════
void admin::buildBedDeptCards()
{
    QFrame *container = ui->bedDeptContainer;
    qDeleteAll(container->findChildren<QWidget*>(QString(), Qt::FindDirectChildrenOnly));

    // Total cards = dept records + 1 overall card
    const int totalCards = m_bedRecords.size() + 1;
    const int containerW = 780;
    const int gapX       = 8;
    const int cardW      = (containerW - (totalCards - 1) * gapX) / totalCards;
    const int cardH      = 110;

    int totalOccupied = 0, totalBeds = 0;
    for (const BedRecord &b : qAsConst(m_bedRecords)) {
        totalOccupied += b.bedsOccupied;
        totalBeds     += b.totalBeds;
    }

    auto makePBar = [&](QWidget *parent, double pct, int x, int y, int w) -> QProgressBar* {
        auto *pb = new QProgressBar(parent);
        pb->setRange(0, 100);
        pb->setValue(static_cast<int>(pct));
        pb->setTextVisible(false);
        pb->setGeometry(x, y, w, 5);
        pb->setStyleSheet(QString(
                              "QProgressBar { background:rgba(255,255,255,0.12); border-radius:3px; border:none; }"
                              "QProgressBar::chunk { background:%1; border-radius:3px; }").arg(bedStatusColor(pct)));
        return pb;
    };

    // ── Department cards ─────────────────────────────────────────
    for (int i = 0; i < m_bedRecords.size(); ++i) {
        const BedRecord &b = m_bedRecords[i];
        double pct = (b.totalBeds > 0) ? (100.0 * b.bedsOccupied / b.totalBeds) : 0.0;
        int x = i * (cardW + gapX);

        QFrame *card = new QFrame(container);
        card->setGeometry(x, 0, cardW, cardH);
        card->setStyleSheet(
            "QFrame { background:rgba(15,40,75,0.45); border:1px solid rgba(255,255,255,0.18); border-radius:10px; }");

        // Department name + percentage on same row
        QLabel *deptLabel = new QLabel(b.department, card);
        deptLabel->setGeometry(8, 8, cardW - 44, 18);
        deptLabel->setStyleSheet("QLabel { color:rgba(200,220,240,0.80); font-size:11px; "
                                 "font-family:'Segoe UI'; background:transparent; border:none; }");

        QString pctColor = bedStatusColor(pct);
        QLabel *pctLabel = new QLabel(QString("%1%").arg(qRound(pct)), card);
        pctLabel->setGeometry(cardW - 40, 5, 36, 22);
        pctLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        pctLabel->setStyleSheet(QString("QLabel { color:%1; font-size:13px; font-weight:700; "
                                        "font-family:'Segoe UI'; background:transparent; border:none; }").arg(pctColor));

        // Progress bar
        makePBar(card, pct, 8, 32, cardW - 16);

        // Beds filled
        QLabel *fillLabel = new QLabel(
            QString("%1/%2 Filled").arg(b.bedsOccupied).arg(b.totalBeds), card);
        fillLabel->setGeometry(8, 44, cardW - 16, 14);
        fillLabel->setStyleSheet("QLabel { color:rgba(200,220,240,0.55); font-size:9px; "
                                 "font-family:'Segoe UI'; background:transparent; border:none; }");

        // Status badge
        QString statusLbl = bedStatusLabel(pct);
        QString statusBg  = (pct >= 90) ? "#3d0b0b" : (pct >= 70) ? "#3a2800" :
                                                     (pct >= 50) ? "#1a2800" : "#0b2d1a";
        QLabel *badge = new QLabel(statusLbl, card);
        badge->setGeometry(8, 64, cardW - 16, 20);
        badge->setAlignment(Qt::AlignCenter);
        badge->setStyleSheet(QString("QLabel { background:%1; color:%2; font-size:9px; font-weight:700; "
                                     "font-family:'Segoe UI'; border-radius:4px; padding:1px 4px; }")
                                 .arg(statusBg).arg(pctColor));
    }

    // ── Overall hospital card (last slot) ────────────────────────
    {
        int x = m_bedRecords.size() * (cardW + gapX);
        double overallPct = (totalBeds > 0) ? (100.0 * totalOccupied / totalBeds) : 0.0;
        QString oColor    = bedStatusColor(overallPct);

        QFrame *card = new QFrame(container);
        card->setGeometry(x, 0, cardW, cardH);
        card->setStyleSheet(
            "QFrame { background:rgba(15,40,75,0.45); border:1px solid rgba(255,255,255,0.18); border-radius:10px; }");

        QLabel *overallTitle = new QLabel("OVERALL", card);
        overallTitle->setGeometry(0, 6, cardW, 14);
        overallTitle->setAlignment(Qt::AlignCenter);
        overallTitle->setStyleSheet("QLabel { color:#00c8c8; font-size:9px; font-weight:700; "
                                    "letter-spacing:1px; font-family:'Segoe UI'; background:transparent; border:none; }");

        QLabel *bigPct = new QLabel(QString("%1%").arg(qRound(overallPct)), card);
        bigPct->setGeometry(0, 22, cardW, 36);
        bigPct->setAlignment(Qt::AlignCenter);
        bigPct->setStyleSheet(QString("QLabel { color:%1; font-size:26px; font-weight:800; "
                                      "font-family:'Segoe UI'; background:transparent; border:none; }").arg(oColor));

        QLabel *totalFill = new QLabel(
            QString("%1/%2 Beds").arg(totalOccupied).arg(totalBeds), card);
        totalFill->setGeometry(0, 60, cardW, 14);
        totalFill->setAlignment(Qt::AlignCenter);
        totalFill->setStyleSheet("QLabel { color:rgba(200,220,240,0.55); font-size:9px; "
                                 "font-family:'Segoe UI'; background:transparent; border:none; }");

        // Progress bar for overall
        makePBar(card, overallPct, 8, 80, cardW - 16);
    }
}

// ════════════════════════════════════════════════════════════════════
//  Staff Table
// ════════════════════════════════════════════════════════════════════
void admin::populateTable(const QVector<StaffRecord> &records)
{
    ui->staffTable->setRowCount(0);

    for (int row = 0; row < records.size(); ++row) {
        const StaffRecord &r = records[row];
        ui->staffTable->insertRow(row);

        // Column 0: Employee ID
        QTableWidgetItem *idItem = new QTableWidgetItem(QString("#%1").arg(r.employeeId));
        idItem->setForeground(QColor("#00c8c8"));
        idItem->setFont([]{ QFont f("Segoe UI", 11, QFont::Bold); return f; }());
        ui->staffTable->setItem(row, 0, idItem);

        // Columns 1-5: plain text
        auto *nameItem  = new QTableWidgetItem(r.name);
        auto *specItem  = new QTableWidgetItem(r.specialty);
        auto *deptItem  = new QTableWidgetItem(r.department);
        auto *shiftItem = new QTableWidgetItem(r.shift);
        auto *patItem   = new QTableWidgetItem(r.currentPatient);

        for (auto *it : {nameItem, specItem, deptItem, shiftItem, patItem})
            it->setForeground(QColor("#e2e8f0"));

        ui->staffTable->setItem(row, 1, nameItem);
        ui->staffTable->setItem(row, 2, specItem);
        ui->staffTable->setItem(row, 3, deptItem);
        ui->staffTable->setItem(row, 4, shiftItem);
        ui->staffTable->setItem(row, 5, patItem);

        // Column 6: Status badge widget
        QWidget *badgeWidget = new QWidget();
        badgeWidget->setStyleSheet("background:transparent;");
        QHBoxLayout *badgeLayout = new QHBoxLayout(badgeWidget);
        badgeLayout->setContentsMargins(6, 0, 6, 0);
        badgeLayout->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        badgeLayout->addWidget(makeStatusBadge(r.status));
        ui->staffTable->setCellWidget(row, 6, badgeWidget);

        // Column 7: Actions button
        QWidget *actWidget = new QWidget();
        actWidget->setStyleSheet("background:transparent;");
        QHBoxLayout *actLayout = new QHBoxLayout(actWidget);
        actLayout->setContentsMargins(6, 0, 6, 0);
        actLayout->setAlignment(Qt::AlignCenter);

        QPushButton *actBtn = new QPushButton("•••");
        actBtn->setFixedSize(32, 28);
        actBtn->setStyleSheet(
            "QPushButton { background:rgba(255,255,255,0.12); border:1px solid rgba(255,255,255,0.20); border-radius:6px; "
            "color:rgba(200,220,240,0.80); font-size:14px; }"
            "QPushButton:hover { background:rgba(255,255,255,0.22); color:#fff; }");
        actBtn->setCursor(Qt::PointingHandCursor);

        QString empId = r.employeeId;
        connect(actBtn, &QPushButton::clicked, this, [this, empId](){
            QMessageBox::information(this, "Staff Actions",
                                     QString("Actions for employee %1\n(Edit / Delete / View Profile)").arg(empId));
        });
        actLayout->addWidget(actBtn);
        ui->staffTable->setCellWidget(row, 7, actWidget);
    }
}

void admin::applyPage(int page)
{
    m_currentPage = page;
    int total = m_filteredStaff.size();
    int start = (page - 1) * m_rowsPerPage;
    int end   = qMin(start + m_rowsPerPage, total);

    populateTable(m_filteredStaff.mid(start, end - start));
    refreshPaginationBar();
    buildPageButtons();
}

void admin::refreshPaginationBar()
{
    int total = m_filteredStaff.size();
    int start = (m_currentPage - 1) * m_rowsPerPage + 1;
    int end   = qMin(m_currentPage * m_rowsPerPage, total);
    if (total == 0) start = 0;

    ui->paginationInfoLabel->setText(
        QString("Showing %1–%2 of %3 entries").arg(start).arg(end).arg(total));

    int totalPages = (total + m_rowsPerPage - 1) / m_rowsPerPage;
    ui->prevPageBtn->setEnabled(m_currentPage > 1);
    ui->nextPageBtn->setEnabled(m_currentPage < totalPages);
}

void admin::buildPageButtons()
{
    QFrame *frame = ui->pageButtonsFrame;
    qDeleteAll(frame->findChildren<QWidget*>(QString(), Qt::FindDirectChildrenOnly));

    int total      = m_filteredStaff.size();
    int totalPages = (total + m_rowsPerPage - 1) / m_rowsPerPage;

    QHBoxLayout *lay = new QHBoxLayout(frame);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(4);

    for (int p = 1; p <= totalPages; ++p) {
        QPushButton *btn = new QPushButton(QString::number(p), frame);
        btn->setFixedSize(30, 30);
        bool active = (p == m_currentPage);
        btn->setStyleSheet(active
                               ? "QPushButton { background:#0aafaf; border:none; border-radius:6px; "
                                 "color:#fff; font-size:12px; font-weight:700; font-family:'Segoe UI'; }"
                               : "QPushButton { background:rgba(255,255,255,0.12); border:1px solid rgba(255,255,255,0.20); border-radius:6px; "
                                 "color:rgba(255,255,255,0.75); font-size:12px; font-family:'Segoe UI'; }"
                                 "QPushButton:hover { background:rgba(255,255,255,0.22); color:#fff; }");
        btn->setCursor(Qt::PointingHandCursor);
        connect(btn, &QPushButton::clicked, this, [this, p](){ onPageButtonClicked(p); });
        lay->addWidget(btn);
    }
    frame->setLayout(lay);
}

// ════════════════════════════════════════════════════════════════════
//  Helpers
// ════════════════════════════════════════════════════════════════════
QLabel *admin::makeStatusBadge(const QString &status, QWidget *parent)
{
    QLabel *lbl = new QLabel(status, parent);
    lbl->setAlignment(Qt::AlignCenter);

    QString bg, color;
    if      (status == "On Duty")        { bg = "#0b2d1a"; color = "#22c55e"; }
    else if (status == "Emergency Call") { bg = "#3d0b0b"; color = "#ef4444"; }
    else if (status == "Off Duty")       { bg = "#1a2233"; color = "#94a3b8"; }
    else if (status == "On Break")       { bg = "#2a1f00"; color = "#f59e0b"; }
    else                                 { bg = "#1a2233"; color = "#e2e8f0"; }

    lbl->setStyleSheet(QString(
                           "QLabel { background:%1; color:%2; font-size:11px; font-weight:700; "
                           "font-family:'Segoe UI'; border-radius:6px; padding:3px 8px; }")
                           .arg(bg).arg(color));
    return lbl;
}

QString admin::bedStatusColor(double pct) const
{
    if (pct >= 85) return "#ef4444";
    if (pct >= 70) return "#f59e0b";
    if (pct >= 50) return "#22d3ee";
    return "#22c55e";
}

QString admin::bedStatusLabel(double pct) const
{
    if (pct >= 85) return "CRITICAL";
    if (pct >= 70) return "HIGH";
    if (pct >= 50) return "WARNING";
    return "NORMAL";
}

// ════════════════════════════════════════════════════════════════════
//  Slots
// ════════════════════════════════════════════════════════════════════
void admin::updateClock()
{
    QDateTime now = QDateTime::currentDateTime();
    ui->clockTimeLabel->setText(now.toString("hh:mm AP"));
    ui->clockDateLabel->setText(now.toString("MMM d, yyyy").toUpper());
}

void admin::onSearchTextChanged(const QString &text)
{
    if (text.trimmed().isEmpty()) {
        m_filteredStaff = m_allStaff;
    } else {
        QString q = text.trimmed().toLower();
        m_filteredStaff.clear();
        for (const StaffRecord &r : qAsConst(m_allStaff)) {
            if (r.name.toLower().contains(q)       ||
                r.employeeId.toLower().contains(q)  ||
                r.specialty.toLower().contains(q)   ||
                r.department.toLower().contains(q)  ||
                r.status.toLower().contains(q)) {
                m_filteredStaff.append(r);
            }
        }
    }
    applyPage(1);
}

void admin::onPrevPage()
{
    if (m_currentPage > 1) applyPage(m_currentPage - 1);
}

void admin::onNextPage()
{
    int totalPages = (m_filteredStaff.size() + m_rowsPerPage - 1) / m_rowsPerPage;
    if (m_currentPage < totalPages) applyPage(m_currentPage + 1);
}

void admin::onPageButtonClicked(int page) { applyPage(page); }

void admin::onAddStaffClicked()
{
    QMessageBox::information(this, "Add New Staff",
                             "Add New Staff dialog — connect your AddStaffDialog here.");
}

// ── Navigation stubs ────────────────────────────────────────────────
void admin::onNavDashboard()  { /* already on dashboard */ }
void admin::onNavPatient()    { QMessageBox::information(this, "Navigation", "Patient List — coming soon."); }
void admin::onNavSchedule()   { QMessageBox::information(this, "Navigation", "Schedule — coming soon."); }
void admin::onNavMedRecords() { QMessageBox::information(this, "Navigation", "Medical Records — coming soon."); }
void admin::onNavSettings()   { QMessageBox::information(this, "Navigation", "Settings — coming soon."); }

void admin::onNavLogout()
{
    int ret = QMessageBox::question(this, "Logout",
                                    "Are you sure you want to log out?",
                                    QMessageBox::Yes | QMessageBox::No);

    if (ret == QMessageBox::Yes) {
        // Show the login window again, then close this window
        if (m_loginWindow) {
            m_loginWindow->show();
        }
        this->close();   // WA_DeleteOnClose will free memory
    }
}