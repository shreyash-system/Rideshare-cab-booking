#ifndef CAB_H
#define CAB_H
#include <string>
using namespace std;

class Cab {
protected:
    string cabType;
    double baseFare;
    double ratePerKm;
public:
    Cab() {}
    Cab(string t, double b, double r) : cabType(t), baseFare(b), ratePerKm(r) {}
    virtual double calculateFare(double distance) { return baseFare + ratePerKm * distance; }
    string getCabType() { return cabType; }
    double getBaseFare() { return baseFare; }
    double getRatePerKm() { return ratePerKm; }
};

class Mini : public Cab {
public:
    Mini() : Cab("Mini", 50, 10) {}
};

class Prime : public Cab {
public:
    Prime() : Cab("Prime", 80, 15) {}
};

class SUV : public Cab {
public:
    SUV() : Cab("SUV", 100, 20) {}
};

#endif
