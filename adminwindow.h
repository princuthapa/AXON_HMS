#ifndef ADMINWINDOW_H
#define ADMINWINDOW_H

#include <QWidget> // <-- Changed from QMainWindow to match your UI file structure
#include "admin.h"
#include "staffmanager.h"

namespace Ui {
class Form; // <-- Your .ui file objectName is "Form", so we use Form here
}

class adminwindow : public QWidget // <-- Changed from QMainWindow to QWidget
{
    Q_OBJECT

public:
    explicit adminwindow(const QString &employeeName, QWidget *parent = nullptr);
    ~adminwindow();

private:
    Ui::Form *ui; // <-- Links specifically to the UI Form pointer template
    Admin *adminBackend;
    StaffManager *staffMgr;
    QString currentAdminName;
};

#endif // ADMINWINDOW_H