#include "adminwindow.h"
#include "ui_adminwindow.h"

adminwindow::adminwindow(const QString &employeeName, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Form),// Connects perfectly to your form!
    currentAdminName(employeeName)
{
    ui->setupUi(this);

    adminBackend = new Admin();
    staffMgr = new StaffManager();

    this->setWindowTitle("AXON-HMS: Admin Dashboard");
}

adminwindow::~adminwindow()
{
    delete adminBackend;
    delete staffMgr;
    delete ui;
}
