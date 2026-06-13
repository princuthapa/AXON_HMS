#ifndef STAFFMANAGER_H
#define STAFFMANAGER_H

#include <QString>
#include <QVector>

// Simple struct to represent staff data details internally
struct StaffData {
    QString id;
    QString name;
    QString age;
    QString gender;
    QString phone;
    QString role;
};

class StaffManager
{
public:
    StaffManager();

    // Methods exactly from your class diagram:
    void addStaff(const StaffData &newStaff);
    void removeStaff(const QString &staffId);
    StaffData searchStaff(const QString &staffId);

private:
    QVector<StaffData> staffList; // Stores your current staff roster
};

#endif // STAFFMANAGER_H