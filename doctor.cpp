#include "doctor.h"

Doctor::Doctor() : Person() {
    patientMgr = new PatientManager();
}

Doctor::Doctor(const QString &id, const QString &name, const QString &age,
               const QString &gender, const QString &phone)
    : Person(id, name, age, gender, phone)
{
    patientMgr = new PatientManager();
}

Doctor::~Doctor() {
    delete patientMgr;
}

void Doctor::runMainInterface() {}
void Doctor::viewPatient() {}
void Doctor::diagnose() {}
void Doctor::treat() {}
void Doctor::viewAppointments() {}
