#ifndef RECEPTIONIST_H
#define RECEPTIONIST_H

#include "person.h"
#include "patientmanager.h"
#include "billingmanager.h"

// Receptionist "isa" Person, and "uses" both PatientManager and
// BillingManager (dependency arrows on the diagram).
class Receptionist : public Person
{
public:
    Receptionist();
    Receptionist(const QString &id, const QString &name, const QString &age,
                 const QString &gender, const QString &phone);
    ~Receptionist();

    // The core execution method that drives the main interface menu loop
    void runMainInterface();

    // The sub-methods matching the class diagram
    void registerPatient();
    void scheduleAppt();
    void billing();
    void allocateBed();
    void discharge();

private:
    // Dependency links to the manager classes
    PatientManager *patientMgr;
    BillingManager *billingMgr;
};

#endif // RECEPTIONIST_H
