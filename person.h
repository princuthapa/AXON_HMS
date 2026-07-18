#ifndef PERSON_H
#define PERSON_H

#include <QString>


class Person
{
public:
    Person();
    Person(const QString &id, const QString &name, const QString &age,
           const QString &gender, const QString &phone);
    virtual ~Person();

    QString getId() const;
    QString getName() const;
    QString getAge() const;
    QString getGender() const;
    QString getPhone() const;

    void setId(const QString &id);
    void setName(const QString &name);
    void setAge(const QString &age);
    void setGender(const QString &gender);
    void setPhone(const QString &phone);

protected:
    QString id;
    QString name;
    QString age;
    QString gender;
    QString phone;
};

#endif // PERSON_H
