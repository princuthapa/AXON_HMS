#ifndef PATIENT_H
#define PATIENT_H

#include <QString>


struct Patient {
    QString id;
    QString name;
    QString age;
    QString gender;
    QString diagnosisTreatment;
    QString assignedDoctor;
    QString status;
    QString bedNumber;
};

#endif // PATIENT_H
