#include "staffmanager.h"
#include <QFile>
#include <QTextStream>

StaffManager::StaffManager() {
    // Dynamically load existing staff from the file when the application starts
    QFile file("staff_database.csv");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine().trimmed();
            if (line.isEmpty()) continue;

            QStringList fields = line.split(",");
            if (fields.size() >= 4) {
                StaffData staff;
                staff.id = fields[0].trimmed();
                staff.name = fields[1].trimmed();
                staff.role = fields[2].trimmed();
                staff.phone = fields[3].trimmed();
                staffList.append(staff);
            }
        }
        file.close();
    }
}

// +addStaff() -> Appends a new staff record to the file database
void StaffManager::addStaff(const StaffData &newStaff) {
    staffList.append(newStaff);

    QFile file("staff_database.csv");
    if (file.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&file);
        out << newStaff.id << ","
            << newStaff.name << ","
            << newStaff.role << ","
            << newStaff.phone << "\n";
        file.close();
    }
}

// +removeStaff() -> Deletes a record from memory and rewrites the database
void StaffManager::removeStaff(const QString &staffId) {
    bool found = false;
    for (int i = 0; i < staffList.size(); ++i) {
        if (staffList[i].id == staffId) {
            staffList.removeAt(i);
            found = true;
            break;
        }
    }

    if (found) {
        // Rewrite the updated vector back to the CSV database file
        QFile file("staff_database.csv");
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            for (const auto &staff : staffList) {
                out << staff.id << "," << staff.name << ","
                    << staff.role << "," << staff.phone << "\n";
            }
            file.close();
        }
    }
}

// +searchStaff() -> Returns the matched staff object by ID
StaffData StaffManager::searchStaff(const QString &staffId) {
    for (const auto &staff : staffList) {
        if (staff.id == staffId) {
            return staff;
        }
    }
    return StaffData(); // Returns an empty data structure if not found
}