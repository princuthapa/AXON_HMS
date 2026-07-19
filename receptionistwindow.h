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
#include "billingmanager.h"

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
    void onBookAppointmentClicked(); // Handles click event

    // Billing page
    void onBillingSearchClicked();
    void onGenerateBillClicked();
    void onProcessPaymentClicked();

private:
    Ui::ReceptionistWindow *ui;

    // Shared backend — same CSV files Admin & Doctor windows use
    PatientManager     *patientMgr;
    StaffManager       *staffMgr;
    AppointmentManager *apptMgr;
    BillingManager      *billingMgr;

    // Helper functions to keep code clean
    void setupConnections();
    void refreshDashboardData();
    void populateAppointmentsTable();
    void populateRecentPatientsTable();

    void populateDoctorDropdowns();     // fills doctorbox / doctorComboBox from StaffManager
    void refreshScheduleTable();        // fills the Scheduling page's tableWidget from AppointmentManager
    void refreshRegisteredPatientsList();// fills listRecentPatientsReg from PatientManager
    void addPatientListItem(const Patient &p); // builds one custom list row widget

    // Billing page helpers
    QString currentBillingPatientId;   // patient currently loaded on the Billing page
    QString currentBillId;             // bill currently displayed in Current Bill Summary (may be empty)

    void populateBillingPatientCard(const Patient &p);
    void clearBillingPatientCard();
    void populateBillSummaryTable(const BillingRecord &bill);
    void clearBillSummaryTable();
    QString selectedPaymentMode() const; // reads the checked Payment Mode radio button

    QTimer *dateTimeTimer;
};

#endif // RECEPTIONISTWINDOW_H