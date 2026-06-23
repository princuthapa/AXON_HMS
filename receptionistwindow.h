#ifndef RECEPTIONISTWINDOW_H
#define RECEPTIONISTWINDOW_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class ReceptionistWindow; }
QT_END_NAMESPACE

class ReceptionistWindow : public QWidget
{
    Q_OBJECT

public:
    ReceptionistWindow(QWidget *parent = nullptr);
    ~ReceptionistWindow();

private slots:
    // Sidebar Navigation Buttons
    void onDashboardClicked();
    void onRegisterPatientClicked();
    void onScheduleClicked();
    void onBillingClicked();

    // Action Buttons
    void onViewAllAppointmentsClicked();

private:
    Ui::ReceptionistWindow *ui;

    // Helper functions to keep code clean
    void setupConnections();
    void refreshDashboardData();
    void populateAppointmentsTable();
    void populateRecentPatientsTable();
};

#endif // RECEPTIONISTWINDOW_H