#ifndef DOCTOR_H
#define DOCTOR_H

#include "person.h"
#include "patientmanager.h"


class Doctor : public Person
{
public:
    Doctor();
    Doctor(const QString &id, const QString &name, const QString &age,
           const QString &gender, const QString &phone);
    ~Doctor();


    void runMainInterface();


    void viewPatient();
    void diagnose();
    void treat();
    void viewAppointments();

private:

    PatientManager *patientMgr;
};

#endif // DOCTOR_H
