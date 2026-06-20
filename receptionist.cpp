#include "receptionist.h"

Receptionist::Receptionist() : Person() {
    patientMgr = new PatientManager();
    billingMgr = new BillingManager();
}

Receptionist::Receptionist(const QString &id, const QString &name, const QString &age,
                           const QString &gender, const QString &phone)
    : Person(id, name, age, gender, phone)
{
    patientMgr = new PatientManager();
    billingMgr = new BillingManager();
}

Receptionist::~Receptionist() {
    delete patientMgr;
    delete billingMgr;
}

void Receptionist::runMainInterface() {}
void Receptionist::registerPatient() {}
void Receptionist::scheduleAppt() {}
void Receptionist::billing() {}
void Receptionist::allocateBed() {}
void Receptionist::discharge() {}
