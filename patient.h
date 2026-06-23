#ifndef PATIENT_H
#define PATIENT_H

#include <QString>

// Lightweight data record for a patient, matching the
// "Patient (struct)" block in the AXON class diagram.
struct Patient {
    QString id;
    QString name;
    QString age;
    QString gender;
    QString diagnosisTreatment; // combined "diagnosis/treatment" field from the diagram
    QString assignedDoctor;
    QString status;             // Admitted / Discharged
    QString bedNumber;
};

#endif // PATIENT_H
