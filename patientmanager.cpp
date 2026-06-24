#include "patientmanager.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>

PatientManager::PatientManager() {
    loadAll();
}

// Loads all patient records from the CSV database into memory
void PatientManager::loadAll() {
    patientList.clear();

    QFile file("patient_database.csv");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "PatientManager: patient_database.csv not found, starting fresh.";
        return;
    }

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty() || line.startsWith("#")) continue;

        QStringList fields = line.split(",");
        if (fields.size() >= 8) {
            Patient p;
            p.id                 = fields[0].trimmed();
            p.name               = fields[1].trimmed();
            p.age                = fields[2].trimmed();
            p.gender             = fields[3].trimmed();
            p.diagnosisTreatment = fields[4].trimmed();
            p.assignedDoctor     = fields[5].trimmed();
            p.status             = fields[6].trimmed();
            p.bedNumber          = fields[7].trimmed();
            patientList.append(p);
        }
    }
    file.close();
}

// Rewrites the entire CSV from the in-memory vector (used after any mutation)
void PatientManager::saveAll() {
    QFile file("patient_database.csv");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "PatientManager: Cannot open patient_database.csv for writing.";
        return;
    }

    QTextStream out(&file);
    out << "# patient_id, name, age, gender, diagnosis/treatment, assigned_doctor, status, bed_number\n";
    for (const auto &p : patientList) {
        out << p.id << ","
            << p.name << ","
            << p.age << ","
            << p.gender << ","
            << p.diagnosisTreatment << ","
            << p.assignedDoctor << ","
            << p.status << ","
            << p.bedNumber << "\n";
    }
    file.close();
}

// +addPatient() -> Appends a new patient record to the file database
void PatientManager::addPatient(const Patient &newPatient) {
    patientList.append(newPatient);

    QFile file("patient_database.csv");
    if (file.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&file);
        out << newPatient.id << ","
            << newPatient.name << ","
            << newPatient.age << ","
            << newPatient.gender << ","
            << newPatient.diagnosisTreatment << ","
            << newPatient.assignedDoctor << ","
            << newPatient.status << ","
            << newPatient.bedNumber << "\n";
        file.close();
    }
}

// +removePatient() -> Deletes a record from memory and rewrites the database
void PatientManager::removePatient(const QString &patientId) {
    for (int i = 0; i < patientList.size(); ++i) {
        if (patientList[i].id == patientId) {
            patientList.removeAt(i);
            saveAll();
            return;
        }
    }
    qDebug() << "PatientManager: Patient" << patientId << "not found for removal.";
}

// +searchPatient() -> Returns the matched patient object by ID
Patient PatientManager::searchPatient(const QString &patientId) {
    for (const auto &p : patientList) {
        if (p.id == patientId) {
            return p;
        }
    }
    return Patient(); // Returns empty struct if not found
}

// +diagnose() -> Updates diagnosis field and persists
void PatientManager::diagnose(const QString &patientId, const QString &diagnosis) {
    for (auto &p : patientList) {
        if (p.id == patientId) {
            p.diagnosisTreatment = diagnosis;
            saveAll();
            return;
        }
    }
}

// +treat() -> Updates treatment info (appends to diagnosis field) and persists
void PatientManager::treat(const QString &patientId, const QString &treatment) {
    for (auto &p : patientList) {
        if (p.id == patientId) {
            p.diagnosisTreatment = treatment;
            saveAll();
            return;
        }
    }
}

// +discharge() -> Sets patient status to Discharged and persists
void PatientManager::discharge(const QString &patientId) {
    for (auto &p : patientList) {
        if (p.id == patientId) {
            p.status = "Discharged";
            saveAll();
            return;
        }
    }
}

QVector<Patient> PatientManager::getAllPatients() const {
    return patientList;
}

int PatientManager::getTotalCount() const {
    return patientList.size();
}
