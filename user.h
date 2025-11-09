#ifndef USER_H
#define USER_H
#include <string>
using namespace std;

class User {
protected:
    string name;
    string phone;
public:
    User() {}
    User(string n, string p) : name(n), phone(p) {}
    string getName() { return name; }
    string getPhone() { return phone; }
    virtual ~User() {} // Virtual destructor for proper cleanup
};

class Rider : public User {
public:
    Rider() {}
    Rider(string n, string p) : User(n, p) {}
};

class Driver : public User {
protected:
    string vehicleNum;
    string cabType;
public:
    Driver() {}
    Driver(string n, string p, string v, string c) : User(n, p), vehicleNum(v), cabType(c) {}
    string getVehicleNum() { return vehicleNum; }
    string getCabType() { return cabType; }
};

#endif
