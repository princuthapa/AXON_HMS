#ifndef BILLINGMANAGER_H
#define BILLINGMANAGER_H

#include <QString>


class BillingManager
{
public:
    BillingManager();


    QString generateBill(const QString &patientId, double amount);
    bool processPayment(const QString &patientId, double amount);
};

#endif // BILLINGMANAGER_H
