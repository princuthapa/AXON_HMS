#ifndef DOCTORWINDOW_H
#define DOCTORWINDOW_H

#include <QWidget> // Matches the root widget type generated from doctorwindow.ui ("Form")
#include "doctor.h"

namespace Ui {
class DoctorForm; // doctorwindow.ui's <class> tag is "DoctorForm"
}

class doctorwindow : public QWidget
{
    Q_OBJECT

public:
    explicit doctorwindow(QWidget *parent = nullptr);
    ~doctorwindow();

private:
    Ui::DoctorForm *ui;
    Doctor *doctorBackend;
};

#endif // DOCTORWINDOW_H
