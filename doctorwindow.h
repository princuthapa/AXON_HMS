#ifndef DOCTORWINDOW_H
#define DOCTORWINDOW_H

#include <QWidget> // Matches the root widget type generated from doctorwindow.ui ("Form")
#include <QTimer>
#include <QMainWindow>
#include "doctor.h"

namespace Ui
{
class doctorwindow;
};

class doctorwindow : public QWidget
{
    Q_OBJECT

public:
    explicit doctorwindow(const QString &employeeName, QWidget *parent = nullptr);
    ~doctorwindow();

private slots:
    void updateDateTime();
    void switchPage();
    void logout();
    void setupOverviewPage();
    void setupStatsSection();

    void on_scheduleBtn_clicked();

private:
    Ui::doctorwindow *ui;
    Doctor *doctorBackend;
    QString currentUserName;
    QTimer *timer;

};

#endif // DOCTORWINDOW_H
