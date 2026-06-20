#ifndef BILLINGMANAGER_H
#define BILLINGMANAGER_H

#include <QString>

// Handles billing for a patient's stay. Referenced by Receptionist and
// PatientManager (dependency arrows on the diagram).
class BillingManager
{
public:
    BillingManager();

    // Methods matching the diagram
    QString generateBill(const QString &patientId, double amount);
    bool processPayment(const QString &patientId, double amount);
};

#endif // BILLINGMANAGER_H
