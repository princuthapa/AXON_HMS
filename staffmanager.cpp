#include "staffmanager.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>

StaffManager::StaffManager() {
    loadAll();
}

void StaffManager::loadAll() {
    staffList.clear();

    QFile file("staff_database.csv");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        // Fallback: try the embedded resource (read-only, used as seed)
        QFile res(":/database/staff_database.csv");
        if (!res.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qDebug() << "StaffManager: staff_database.csv not found anywhere.";
            return;
        }
        QTextStream in(&res);
        _parseStream(in);
        res.close();
        saveAll(); // write out to working directory so future edits persist
        return;
    }

    QTextStream in(&file);
    _parseStream(in);
    file.close();
}

void StaffManager::_parseStream(QTextStream &in) {
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty() || line.startsWith("#")) continue;

        QStringList f = line.split(",");
        // Format: username,password,Role,staff_id,full_name,age,gender,phone
        if (f.size() >= 8) {
            StaffData s;
            s.username = f[0].trimmed();
            s.password = f[1].trimmed();
            s.role     = f[2].trimmed();
            s.id       = f[3].trimmed();
            s.name     = f[4].trimmed();
            s.age      = f[5].trimmed();
            s.gender   = f[6].trimmed();
            s.phone    = f[7].trimmed();
            staffList.append(s);
        }
    }
}

void StaffManager::saveAll() {
    QFile file("staff_database.csv");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "StaffManager: Cannot open staff_database.csv for writing.";
        return;
    }
    QTextStream out(&file);
    out << "# username,password,Role,staff_id,full_name,age,gender,phone\n";
    for (const auto &s : staffList) {
        out << s.username << ","
            << s.password << ","
            << s.role     << ","
            << s.id       << ","
            << s.name     << ","
            << s.age      << ","
            << s.gender   << ","
            << s.phone    << "\n";
    }
    file.close();
}

// +addStaff() -> Appends a new staff record
void StaffManager::addStaff(const StaffData &newStaff) {
    staffList.append(newStaff);
    saveAll();
}

// +removeStaff() -> Removes by staff ID and rewrites the database
void StaffManager::removeStaff(const QString &staffId) {
    for (int i = 0; i < staffList.size(); ++i) {
        if (staffList[i].id == staffId) {
            staffList.removeAt(i);
            saveAll();
            return;
        }
    }
    qDebug() << "StaffManager: Staff" << staffId << "not found for removal.";
}

// +searchStaff() -> Returns matched staff by ID
StaffData StaffManager::searchStaff(const QString &staffId) {
    for (const auto &s : staffList) {
        if (s.id == staffId) return s;
    }
    return StaffData();
}

bool StaffManager::updateStaff(const StaffData &updated) {
    for (auto &s : staffList) {
        if (s.id == updated.id) {
            s = updated;
            saveAll();
            return true;
        }
    }
    return false;
}

QVector<StaffData> StaffManager::getAllStaff() const {
    return staffList;
}

int StaffManager::getTotalCount() const {
    return staffList.size();
}
