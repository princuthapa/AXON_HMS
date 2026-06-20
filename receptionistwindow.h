#ifndef RECEPTIONISTWINDOW_H
#define RECEPTIONISTWINDOW_H

#include <QWidget>
#include "receptionist.h"

namespace Ui {
class ReceptionistForm;
}

class receptionistwindow : public QWidget
{
    Q_OBJECT

public:
    explicit receptionistwindow(QWidget *parent = nullptr);
    ~receptionistwindow();

private:
    Ui::ReceptionistForm *ui;
    Receptionist *receptionistBackend;
};

#endif // RECEPTIONISTWINDOW_H
