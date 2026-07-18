#ifndef APPOINTMENTMANAGER_H
#define APPOINTMENTMANAGER_H

#include <QVector>
#include <QString>
#include <QTextStream>


struct Appointment {
    QString id;
    QString patientName;
    QString doctorName;
    QString department;
    QString date;         // yyyy-MM-dd
    QString time;         //  "09:00 AM"
    QString reason;
    QString status;       // Confirmed / Waiting / In-Progress / Completed / Cancelled
};


class AppointmentManager
{
public:
    AppointmentManager();

    // CRUD - create read update delete
    void addAppointment(const Appointment &appt);
    void removeAppointment(const QString &appointmentId);
    void updateStatus(const QString &appointmentId, const QString &newStatus);
    Appointment searchAppointment(const QString &appointmentId);


    QVector<Appointment> getAllAppointments() const;
    QVector<Appointment> getAppointmentsForDoctor(const QString &doctorName) const;
    QVector<Appointment> getAppointmentsForDate(const QString &date) const;
    QVector<Appointment> getTodaysAppointments() const;

    int getTotalCount() const;
    QString generateNextId() const;


    void reload();

private:
    QVector<Appointment> appointmentList;
    void saveAll();
    void loadAll();
    void _parseStream(QTextStream &in);
};

#endif // APPOINTMENTMANAGER_H
