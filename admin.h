#ifndef ADMIN_H
#define ADMIN_H

#include <QString>
#include "staffmanager.h"

class Admin
{
public:
    Admin();
    ~Admin();

    // The core execution method that drives the main interface menu loop
    void runMainInterface();

    // The sub-methods matching your class diagram
    void manageStaff();
    void getOverview();
    void scheduling();

private:
    // Core attributes from your diagram
    QString id;
    QString name;
    QString age;
    QString gender;
    QString phone;

    // Aggregation link to the manager class
    StaffManager *staffMgr;
};

#endif // ADMIN_H