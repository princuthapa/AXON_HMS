#ifndef PATIENTMANAGER_H
#define PATIENTMANAGER_H

#include <QVector>
#include <QString>
#include "patient.h"

// Manages the patient roster with file-backed CSV persistence.
// Sits between Doctor / Receptionist and the actual patient data.
class PatientManager
{
public:
    PatientManager();

    // CRUD — matching the class diagram
    void addPatient(const Patient &newPatient);
    void removePatient(const QString &patientId);
    Patient searchPatient(const QString &patientId);

    // Clinical operations
    void diagnose(const QString &patientId, const QString &diagnosis);
    void treat(const QString &patientId, const QString &treatment);
    void discharge(const QString &patientId);

    // Utility accessors used by the Admin window
    QVector<Patient> getAllPatients() const;
    int getTotalCount() const;

private:
    QVector<Patient> patientList;
    void saveAll(); // rewrites patient_database.csv from patientList
    void loadAll(); // reads patient_database.csv into patientList
};

#endif // PATIENTMANAGER_H
