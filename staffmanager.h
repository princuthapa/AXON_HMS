#ifndef STAFFMANAGER_H
#define STAFFMANAGER_H

#include <QString>
#include <QVector>
#include <QTextStream>

// Full staff record — includes auth credentials plus profile data.
// CSV format: username,password,Role,staff_id,full_name,age,gender,phone
struct StaffData {
    QString username;
    QString password;
    QString role;
    QString id;
    QString name;
    QString age;
    QString gender;
    QString phone;
};

class StaffManager
{
public:
    StaffManager();

    // Methods from the class diagram
    void addStaff(const StaffData &newStaff);
    void removeStaff(const QString &staffId);
    StaffData searchStaff(const QString &staffId);

    // Extra utilities needed by the Admin window
    QVector<StaffData> getAllStaff() const;
    bool updateStaff(const StaffData &updated);
    int  getTotalCount() const;

private:
    QVector<StaffData> staffList;
    void saveAll();
    void loadAll();
    void _parseStream(QTextStream &in);
};

#endif // STAFFMANAGER_H
