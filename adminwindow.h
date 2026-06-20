#ifndef ADMINWINDOW_H
#define ADMINWINDOW_H

#include <QWidget>
#include <QTimer>
#include <QDateTime>
#include <QMenu>
#include <QAction>
#include "admin.h"
#include "staffmanager.h"

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
    void logoutRequested(); // Signal to notify MainWindow to show the login screen again

private slots:
    void updateDateTime();       // Triggers every second to update the clock
    void on_btnMenu_clicked();    // Opens the 3-dots dropdown menu
    void on_btnOverview_clicked();
    void on_btnManageStaff_clicked();
    void on_btnScheduling_clicked();

private:
    Ui::AXON_ADMIN *ui;
    Admin *adminBackend;
    StaffManager *staffMgr;
    QTimer *timer;               // For real-time updates
    QString currentAdminName;
};

#endif // ADMINWINDOW_H