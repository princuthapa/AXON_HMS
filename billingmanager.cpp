#include "billingmanager.h"
#include <QDebug>

BillingManager::BillingManager() {
    // Pure backend initialization
}


QString BillingManager::generateBill(const QString &patientId, double amount) {
    QString bill = QString("Bill for patient %1: $%2")
    .arg(patientId)
        .arg(amount, 0, 'f', 2);
    qDebug() << bill;
    return bill;
}

// +processPayment() -> Records that a payment was made
bool BillingManager::processPayment(const QString &patientId, double amount) {
    qDebug() << "Processed payment of $" << amount << "for patient" << patientId;
    return true;
}
