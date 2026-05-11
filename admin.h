#ifndef ADMIN_H
#define ADMIN_H
#include "ui_adminwindow.h"
#include <QMainWindow>
#include <QTimer>
#include <QVector>
#include <QString>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QHBoxLayout>

QT_BEGIN_NAMESPACE
namespace Ui { class admin; }
QT_END_NAMESPACE

// ─── Data structures ───────────────────────────────────────────────
struct StaffRecord {
    QString employeeId;
    QString name;
    QString specialty;
    QString department;
    QString shift;
    QString currentPatient;
    QString status;
};

struct BedRecord {
    QString department;
    int     bedsOccupied;
    int     totalBeds;
};

// ─── Admin Dashboard window ─────────────────────────────────────────
class admin : public QMainWindow
{
    Q_OBJECT

public:
    explicit admin(QWidget *loginWindow = nullptr, QWidget *parent = nullptr);
    ~admin();

private slots:
    void updateClock();
    void onSearchTextChanged(const QString &text);
    void onPrevPage();
    void onNextPage();
    void onPageButtonClicked(int page);
    void onAddStaffClicked();
    void onNavDashboard();
    void onNavPatient();
    void onNavSchedule();
    void onNavMedRecords();
    void onNavSettings();
    void onNavLogout();

private:
    void loadStaffCSV(const QString &filePath);
    void loadBedCSV(const QString &filePath);
    void buildBedDeptCards();
    void populateTable(const QVector<StaffRecord> &records);
    void applyPage(int page);
    void refreshPaginationBar();
    void buildPageButtons();

    QLabel  *makeStatusBadge(const QString &status, QWidget *parent = nullptr);
    QString  bedStatusColor(double pct) const;
    QString  bedStatusLabel(double pct) const;

    Ui::admin *ui;

    // Pointer back to the login window so logout can show it again
    QWidget *m_loginWindow = nullptr;

    QVector<StaffRecord> m_allStaff;
    QVector<StaffRecord> m_filteredStaff;
    QVector<BedRecord>   m_bedRecords;

    int m_currentPage = 1;
    int m_rowsPerPage = 6;

    QTimer *m_clockTimer = nullptr;
};

#endif // ADMIN_H