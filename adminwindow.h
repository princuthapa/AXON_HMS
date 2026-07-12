#ifndef ADMINWINDOW_H
#define ADMINWINDOW_H

#include <QWidget>
#include <QTimer>
#include <QDateTime>
#include <QMenu>
#include <QAction>
#include <QLabel>
#include <QScrollArea>
#include <QVBoxLayout>
#include "admin.h"
#include "staffmanager.h"
#include "patientmanager.h"
#include <QtCharts/QChartView>
#include <QtCharts/QChart>
#include <QTimer>
#include <QDateTime>


namespace Ui {
class AXON_ADMIN;
}

class adminwindow : public QWidget
{
    Q_OBJECT

public:
    explicit adminwindow(const QString &employeeName, QWidget *parent = nullptr);
    ~adminwindow();

signals:
    void logoutRequested();

private slots:
    void initDashboardGraphs();
    void updateDateTime();
    void on_btnMenu_clicked();
    void on_btnOverview_clicked();
    void on_btnStaffManager_clicked();
    void on_btnScheduling_clicked();
    void onAddStaffClicked();


private:
    Ui::AXON_ADMIN *ui;
    Admin          *adminBackend;
    StaffManager   *staffMgr;
    PatientManager *patientMgr;
    QTimer         *timer;
    QString         currentAdminName;

    // ── Overview / patient table helpers ─────────────────────────────────────
    void setupPatientHeader();
    void loadPatientRowsFromBackend();
    void addPatientRow(const QString &id,     const QString &name,
                       const QString &gender, const QString &problem,
                       const QString &doctor, const QString &status);

    // ── Staff Manager page ────────────────────────────────────────────────────
    void setupStaffPage();
    void refreshStaffTable();
    void addStaffRow(const StaffData &s);
    void updateStaffCountLabel();

    // Staff page widgets (built programmatically inside page_2)
    QScrollArea *staffScrollArea   = nullptr;
    QWidget     *staffRowContainer = nullptr;
    QVBoxLayout *staffRowsLayout   = nullptr;
    QLabel      *staffCountLabel   = nullptr;
};

#endif // ADMINWINDOW_H
