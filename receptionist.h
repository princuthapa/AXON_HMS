#ifndef RECEPTIONIST_H
#define RECEPTIONIST_H

#include "person.h"
#include "patientmanager.h"
#include "billingmanager.h"


class Receptionist : public Person
{
public:
    Receptionist();
    Receptionist(const QString &id, const QString &name, const QString &age,
                 const QString &gender, const QString &phone);
    ~Receptionist();


    void runMainInterface();


    void registerPatient();
    void scheduleAppt();
    void billing();
    void allocateBed();
    void discharge();

private:

    PatientManager *patientMgr;
    BillingManager *billingMgr;
};

#endif // RECEPTIONIST_H
