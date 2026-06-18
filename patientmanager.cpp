#include "patientmanager.h"
#include <QFile>
#include <QTextStream>

PatientManager::PatientManager() {}
void PatientManager::addPatient(const Patient &newPatient) {
    Q_UNUSED(newPatient);
}
void PatientManager::removePatient(const QString &patientId) {
    Q_UNUSED(patientId);
}
// Patient PatientManager::searchPatient(const QString &patientId) {
//     Q_UNUSED(patientId);
// }
// void PatientManager::diagnose(const QString &patientId, const QString &diagnosis) {

// }
void PatientManager::treat(const QString &patientId, const QString &treatment) {
    Q_UNUSED(patientId);
    Q_UNUSED(treatment);
}
void PatientManager::discharge(const QString &patientId) {
    Q_UNUSED(patientId);
}
void PatientManager::saveAll() {}