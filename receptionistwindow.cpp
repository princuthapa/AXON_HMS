#include "receptionistwindow.h"
#include "ui_receptionistwindow.h"

receptionistwindow::receptionistwindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ReceptionistForm)
{
    ui->setupUi(this);

    receptionistBackend = new Receptionist();

    this->setWindowTitle("AXON-HMS: Receptionist Dashboard");
}

receptionistwindow::~receptionistwindow()
{
    delete receptionistBackend;
    delete ui;
}
