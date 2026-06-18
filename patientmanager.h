#ifndef PATIENTMANAGER_H
#define PATIENTMANAGER_H

#include <QVector>
#include "patient.h"

// Manages the patient roster. Mirrors StaffManager's file-backed design
// so the two manager classes behave consistently. Sits between Doctor /
// Receptionist and the actual patient data
class PatientManager
{
public:
    PatientManager();

    // Methods matching the diagram
    void addPatient(const Patient &newPatient);
    void removePatient(const QString &patientId);
    Patient searchPatient(const QString &patientId);
    void diagnose(const QString &patientId, const QString &diagnosis);
    void treat(const QString &patientId, const QString &treatment);
    void discharge(const QString &patientId);

private:
    QVector<Patient> patientList;

    void saveAll(); // rewrites patient_database.csv from patientList
};

#endif // PATIENTMANAGER_H
