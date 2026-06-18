#include "doctorwindow.h"
#include "ui_doctorwindow.h"

doctorwindow::doctorwindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DoctorForm)
{
    ui->setupUi(this);

    doctorBackend = new Doctor();

    this->setWindowTitle("AXON-HMS: Doctor Dashboard");
}

doctorwindow::~doctorwindow()
{
    delete doctorBackend;
    delete ui;
}
