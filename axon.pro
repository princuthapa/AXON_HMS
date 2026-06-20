QT += core gui widgets quickwidgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    admin.cpp \
    adminwindow.cpp \
    billingmanager.cpp \
    doctor.cpp \
    doctorwindow.cpp \
    main.cpp \
    mainwindow.cpp \
    patientmanager.cpp \
    person.cpp \
    receptionist.cpp \
    receptionistwindow.cpp \
    staffmanager.cpp

HEADERS += \
    admin.h \
    adminwindow.h \
    billingmanager.h \
    doctor.h \
    doctorwindow.h \
    mainwindow.h \
    patient.h \
    patientmanager.h \
    person.h \
    receptionist.h \
    receptionistwindow.h \
    staffmanager.h

FORMS += \
    mainwindow.ui \
    adminwindow.ui \
    doctorwindow.ui \
    receptionistwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc

DISTFILES += \
    readme.md
