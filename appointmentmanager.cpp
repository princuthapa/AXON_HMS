#include "appointmentmanager.h"
#include <QFile>
#include <QDebug>
#include <QDate>
#include <QTime>
#include <algorithm>

AppointmentManager::AppointmentManager() {
    loadAll();
}

void AppointmentManager::reload() {
    loadAll();
}


void AppointmentManager::loadAll() {
    appointmentList.clear();

    QFile file("appointment_database.csv");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {

        QFile res(":/database/appointment_database.csv");
        if (!res.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qDebug() << "AppointmentManager: appointment_database.csv not found anywhere.";
            return;
        }
        QTextStream in(&res);
        _parseStream(in);
        res.close();
        saveAll();
        return;
    }

    QTextStream in(&file);
    _parseStream(in);
    file.close();
}

void AppointmentManager::_parseStream(QTextStream &in) {
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty() || line.startsWith("#")) continue;

        QStringList fields = line.split(",");
        if (fields.size() >= 8) {
            Appointment a;
            a.id          = fields[0].trimmed();
            a.patientName = fields[1].trimmed();
            a.doctorName  = fields[2].trimmed();
            a.department  = fields[3].trimmed();
            a.date        = fields[4].trimmed();
            a.time        = fields[5].trimmed();
            a.reason      = fields[6].trimmed();
            a.status      = fields[7].trimmed();
            appointmentList.append(a);
        }
    }
}


void AppointmentManager::saveAll() {
    QFile file("appointment_database.csv");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "AppointmentManager: Cannot open appointment_database.csv for writing.";
        return;
    }

    QTextStream out(&file);
    out << "# appointment_id, patient_name, doctor_name, department, date, time, reason, status\n";
    for (const auto &a : appointmentList) {
        out << a.id << ","
            << a.patientName << ","
            << a.doctorName << ","
            << a.department << ","
            << a.date << ","
            << a.time << ","
            << a.reason << ","
            << a.status << "\n";
    }
    file.close();
}

void AppointmentManager::addAppointment(const Appointment &appt) {
    appointmentList.append(appt);
    saveAll();
}

void AppointmentManager::removeAppointment(const QString &appointmentId) {
    for (int i = 0; i < appointmentList.size(); ++i) {
        if (appointmentList[i].id == appointmentId) {
            appointmentList.removeAt(i);
            saveAll();
            return;
        }
    }
    qDebug() << "AppointmentManager: Appointment" << appointmentId << "not found for removal.";
}

void AppointmentManager::updateStatus(const QString &appointmentId, const QString &newStatus) {
    for (auto &a : appointmentList) {
        if (a.id == appointmentId) {
            a.status = newStatus;
            saveAll();
            return;
        }
    }
}

Appointment AppointmentManager::searchAppointment(const QString &appointmentId) {
    for (const auto &a : appointmentList) {
        if (a.id == appointmentId) return a;
    }
    return Appointment();
}

QVector<Appointment> AppointmentManager::getAllAppointments() const {
    return appointmentList;
}

QVector<Appointment> AppointmentManager::getAppointmentsForDoctor(const QString &doctorName) const {
    QVector<Appointment> out;
    const QString target = doctorName.trimmed().toLower();
    for (const auto &a : appointmentList) {
        if (a.doctorName.trimmed().toLower() == target)
            out.append(a);
    }
    // Sort by date then time-of-day so the schedule reads top-to-bottom
    std::sort(out.begin(), out.end(), [](const Appointment &l, const Appointment &r) {
        if (l.date != r.date) return l.date < r.date;
        QTime lt = QTime::fromString(l.time, "hh:mm AP");
        QTime rt = QTime::fromString(r.time, "hh:mm AP");
        if (lt.isValid() && rt.isValid()) return lt < rt;
        return l.time < r.time;
    });
    return out;
}

QVector<Appointment> AppointmentManager::getAppointmentsForDate(const QString &date) const {
    QVector<Appointment> out;
    for (const auto &a : appointmentList) {
        if (a.date == date) out.append(a);
    }
    return out;
}

QVector<Appointment> AppointmentManager::getTodaysAppointments() const {
    return getAppointmentsForDate(QDate::currentDate().toString("yyyy-MM-dd"));
}

int AppointmentManager::getTotalCount() const {
    return appointmentList.size();
}

QString AppointmentManager::generateNextId() const {
    int n = appointmentList.size() + 1;
    QString candidate;
    bool exists;
    do {
        candidate = QString("APT-%1").arg(n, 4, 10, QChar('0'));
        exists = false;
        for (const auto &a : appointmentList) {
            if (a.id == candidate) { exists = true; break; }
        }
        ++n;
    } while (exists);
    return candidate;
}
