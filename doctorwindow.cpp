#include "doctorwindow.h"
#include "ui_doctorwindow.h"

doctorwindow::doctorwindow(const QString &employeeName, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DoctorForm),
    currentUserName(employeeName) // Initialize your member variable
{
    ui->setupUi(this);

    doctorBackend = new Doctor();

    this->setWindowTitle("AXON-HMS: Doctor Dashboard");
    ui->welcomeDynamic->setText("Welcome back !, Dr. " + currentUserName);

}

doctorwindow::~doctorwindow()
{
    delete doctorBackend;
    delete ui;
}
