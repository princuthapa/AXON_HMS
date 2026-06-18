#include "person.h"

Person::Person()
    : id(""), name(""), age(""), gender(""), phone("")
{
}

Person::Person(const QString &id, const QString &name, const QString &age,
               const QString &gender, const QString &phone)
    : id(id), name(name), age(age), gender(gender), phone(phone)
{
}

Person::~Person()
{
}

QString Person::getId() const { return id; }
QString Person::getName() const { return name; }
QString Person::getAge() const { return age; }
QString Person::getGender() const { return gender; }
QString Person::getPhone() const { return phone; }

void Person::setId(const QString &id) { this->id = id; }
void Person::setName(const QString &name) { this->name = name; }
void Person::setAge(const QString &age) { this->age = age; }
void Person::setGender(const QString &gender) { this->gender = gender; }
void Person::setPhone(const QString &phone) { this->phone = phone; }
