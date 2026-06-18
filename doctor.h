#ifndef DOCTOR_H
#define DOCTOR_H

#include "person.h"
#include "patientmanager.h"

// Doctor "isa" Person, and "uses" PatientManager (dependency arrow on
// the diagram) to look up and update patient records.
class Doctor : public Person
{
public:
    Doctor();
    Doctor(const QString &id, const QString &name, const QString &age,
           const QString &gender, const QString &phone);
    ~Doctor();

    // The core execution method that drives the main interface menu loop
    void runMainInterface();


    void viewPatient();
    void diagnose();
    void treat();
    void viewAppointments();

private:
    // Dependency link to the manager class
    PatientManager *patientMgr;
};

#endif // DOCTOR_H
