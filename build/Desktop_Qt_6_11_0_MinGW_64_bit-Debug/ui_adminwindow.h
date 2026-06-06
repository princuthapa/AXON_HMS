/********************************************************************************
** Form generated from reading UI file 'adminwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.11.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/
#ifndef UI_ADMINWINDOW_H
#define UI_ADMINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Form
{
public:
    QWidget *sidebarWidget;
    QPushButton *pushButton;
    QPushButton *pushButton_2;
    QPushButton *pushButton_3;
    QPushButton *pushButton_4;

    void setupUi(QWidget *Form)
    {
        if (Form->objectName().isEmpty())
            Form->setObjectName("Form");
        Form->resize(1280, 720);

        sidebarWidget = new QWidget(Form);
        sidebarWidget->setObjectName("sidebarWidget");
        sidebarWidget->setGeometry(QRect(340, 150, 791, 431));

        pushButton = new QPushButton(Form);
        pushButton->setObjectName("pushButton");
        pushButton->setGeometry(QRect(30, 80, 111, 41));

        pushButton_2 = new QPushButton(Form);
        pushButton_2->setObjectName("pushButton_2");
        pushButton_2->setGeometry(QRect(30, 228, 111, 41));

        pushButton_3 = new QPushButton(Form);
        pushButton_3->setObjectName("pushButton_3");
        pushButton_3->setGeometry(QRect(30, 160, 111, 41));

        pushButton_4 = new QPushButton(Form);
        pushButton_4->setObjectName("pushButton_4");
        pushButton_4->setGeometry(QRect(29, 300, 111, 41));

        retranslateUi(Form);
        QMetaObject::connectSlotsByName(Form);
    } // setupUi

    void retranslateUi(QWidget *Form)
    {
        Form->setWindowTitle(QCoreApplication::translate("Form", "Form", nullptr));
        pushButton->setText(QCoreApplication::translate("Form", "Overview", nullptr));
        pushButton_2->setText(QCoreApplication::translate("Form", "Scheduling", nullptr));
        pushButton_3->setText(QCoreApplication::translate("Form", "Manage Staff", nullptr));
        pushButton_4->setText(QCoreApplication::translate("Form", "Log out", nullptr));
    } // retranslateUi
};

namespace Ui {
class Form: public Ui_Form {};
} // namespace Ui

QT_END_NAMESPACE
#endif // UI_ADMINWINDOW_H