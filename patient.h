#ifndef PATIENT_H
#define PATIENT_H

#include <QString>

// Lightweight data record for a patient, matching the
// "Patient (struct)" block in the AXON class diagram.
struct Patient {
    QString id;
    QString name;
    QString gender;
    QString diagnosisTreatment; // the diagram's combined "diagnosis/treatment" field
};

#endif // PATIENT_H
