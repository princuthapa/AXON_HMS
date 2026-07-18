#ifndef RECEPTIONISTWINDOW_H
#define RECEPTIONISTWINDOW_H

#include <QWidget>
#include <QTimer>
#include <QDateTime>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include "patientmanager.h"
#include "staffmanager.h"
#include "appointmentmanager.h"

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
    void onClearFormClicked();
    void onSubmitRegistrationClicked();
    void updateDateTime();
    void onBookAppointmentClicked();
private:
    Ui::ReceptionistWindow *ui;


    PatientManager     *patientMgr;
    StaffManager       *staffMgr;
    AppointmentManager *apptMgr;


    void setupConnections();
    void refreshDashboardData();
    void populateAppointmentsTable();
    void populateRecentPatientsTable();

    void populateDoctorDropdowns();     // fills doctorbox / doctorComboBox from StaffManager
    void refreshScheduleTable();        // fills the Scheduling page's tableWidget from AppointmentManager
    void refreshRegisteredPatientsList();// fills listRecentPatientsReg from PatientManager
    void addPatientListItem(const Patient &p); // builds one custom list row widget

    QTimer *dateTimeTimer;
};

#endif // RECEPTIONISTWINDOW_H
