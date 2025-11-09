#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <vector>
#include <tuple>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include "user.h"
using namespace std;

class Database {
private:
    string usersFile = "data/users.csv";
    string driversFile = "data/drivers.csv";
    string ridesFile = "data/rides.csv";

public:
    Database() {
        // Create data directory if it doesn't exist
        system("mkdir data 2>nul");
        
        // Initialize CSV files with headers if they don't exist
        initializeFiles();
        
        cout << "[OK] Database initialized (CSV-based)!" << endl;
    }

    ~Database() {}

private:
    void initializeFiles() {
        // Initialize users.csv
        ifstream testUsers(usersFile);
        if (!testUsers.good()) {
            ofstream file(usersFile);
            file << "username,fullname,phone,emergency_contact,emergency_phone,payment_method\n";
            file.close();
        }
        testUsers.close();

        // Initialize drivers.csv
        ifstream testDrivers(driversFile);
        if (!testDrivers.good()) {
            ofstream file(driversFile);
            file << "id,name,phone,vehicle_number,cab_type\n";
            file.close();
        }
        testDrivers.close();

        // Initialize rides.csv
        ifstream testRides(ridesFile);
        if (!testRides.good()) {
            ofstream file(ridesFile);
            file << "ride_id,rider_name,rider_phone,pickup,drop_location,cab_type,distance,fare,driver_name,vehicle_number,passenger_count,status,payment_method,scheduled_date,scheduled_time\n";
            file.close();
        }
        testRides.close();
    }

    vector<string> split(const string& str, char delimiter) {
        vector<string> tokens;
        string token;
        stringstream ss(str);
        while (getline(ss, token, delimiter)) {
            tokens.push_back(token);
        }
        return tokens;
    }

public:
    bool insertUser(string username, string fullname, string phone) {
        ofstream file(usersFile, ios::app);
        if (file.is_open()) {
            file << username << "," << fullname << "," << phone << ",,,Cash\n";
            file.close();
            return true;
        }
        return false;
    }

    bool getUserByUsername(string username, string& fullname, string& phone, 
                          string& emergencyContact, string& emergencyPhone, 
                          string& paymentMethod) {
        ifstream file(usersFile);
        string line;
        getline(file, line); // Skip header
        
        while (getline(file, line)) {
            vector<string> fields = split(line, ',');
            if (fields.size() >= 6 && fields[0] == username) {
                fullname = fields[1];
                phone = fields[2];
                emergencyContact = fields[3];
                emergencyPhone = fields[4];
                paymentMethod = fields[5];
                file.close();
                return true;
            }
        }
        file.close();
        return false;
    }

    bool updateUserEmergencyContact(string username, string ec, string ep) {
        ifstream file(usersFile);
        vector<string> lines;
        string line;
        
        // Read all lines
        while (getline(file, line)) {
            lines.push_back(line);
        }
        file.close();

        // Update the user's emergency contact
        ofstream outFile(usersFile);
        for (size_t i = 0; i < lines.size(); i++) {
            if (i == 0) {
                outFile << lines[i] << "\n";
                continue;
            }
            
            vector<string> fields = split(lines[i], ',');
            if (fields.size() >= 6 && fields[0] == username) {
                fields[3] = ec;
                fields[4] = ep;
                outFile << fields[0] << "," << fields[1] << "," << fields[2] << "," 
                        << fields[3] << "," << fields[4] << "," << fields[5] << "\n";
            } else {
                outFile << lines[i] << "\n";
            }
        }
        outFile.close();
        return true;
    }

    bool updateUserPaymentMethod(string username, string pm) {
        ifstream file(usersFile);
        vector<string> lines;
        string line;
        
        while (getline(file, line)) {
            lines.push_back(line);
        }
        file.close();

        ofstream outFile(usersFile);
        for (size_t i = 0; i < lines.size(); i++) {
            if (i == 0) {
                outFile << lines[i] << "\n";
                continue;
            }
            
            vector<string> fields = split(lines[i], ',');
            if (fields.size() >= 6 && fields[0] == username) {
                fields[5] = pm;
                outFile << fields[0] << "," << fields[1] << "," << fields[2] << "," 
                        << fields[3] << "," << fields[4] << "," << fields[5] << "\n";
            } else {
                outFile << lines[i] << "\n";
            }
        }
        outFile.close();
        return true;
    }

    bool insertDriver(string name, string phone, string vehicleNumber, string cabType) {
        // Get next ID
        int nextId = 1;
        ifstream readFile(driversFile);
        string line;
        getline(readFile, line); // Skip header
        while (getline(readFile, line)) {
            nextId++;
        }
        readFile.close();

        ofstream file(driversFile, ios::app);
        if (file.is_open()) {
            file << nextId << "," << name << "," << phone << "," << vehicleNumber << "," << cabType << "\n";
            file.close();
            return true;
        }
        return false;
    }

    vector<Driver> getAllDrivers() {
        vector<Driver> drivers;
        ifstream file(driversFile);
        string line;
        getline(file, line); // Skip header
        
        while (getline(file, line)) {
            vector<string> fields = split(line, ',');
            if (fields.size() >= 5) {
                drivers.push_back(Driver(fields[1], fields[2], fields[3], fields[4]));
            }
        }
        file.close();
        return drivers;
    }

    int insertRide(string riderName, string riderPhone, string pickup, string drop,
                   string cabType, double distance, double fare, string driverName,
                   string vehicleNumber, int passengerCount, string status,
                   string paymentMethod, string scheduledDate, string scheduledTime) {
        // Get next ride ID
        int nextId = 1;
        ifstream readFile(ridesFile);
        string line;
        getline(readFile, line); // Skip header
        while (getline(readFile, line)) {
            if (line.empty()) continue;
            vector<string> fields = split(line, ',');
            if (fields.size() > 0) {
                try {
                    int id = stoi(fields[0]);
                    if (id >= nextId) nextId = id + 1;
                } catch (...) {}
            }
        }
        readFile.close();

        ofstream file(ridesFile, ios::app);
        if (file.is_open()) {
            file << nextId << "," << riderName << "," << riderPhone << "," << pickup << "," 
                 << drop << "," << cabType << "," << distance << "," << fare << "," 
                 << driverName << "," << vehicleNumber << "," << passengerCount << "," 
                 << status << "," << paymentMethod << "," << scheduledDate << "," 
                 << scheduledTime << "\n";
            file.close();
            return nextId;
        }
        return -1;
    }

    vector<tuple<int, string, string, string, string, int, double>> getActiveRides() {
        vector<tuple<int, string, string, string, string, int, double>> rides;
        ifstream file(ridesFile);
        string line;
        
        // Skip header
        getline(file, line);
        
        // Read all rides
        while (getline(file, line)) {
            if (line.empty()) continue;
            
            vector<string> fields = split(line, ',');
            
            // Debug output
            cout << "[DEBUG] Reading line: " << line << endl;
            cout << "[DEBUG] Fields count: " << fields.size() << endl;
            
            if (fields.size() >= 12) {
                try {
                    string status = fields[11];
                    cout << "[DEBUG] Status: '" << status << "'" << endl;
                    
                    if (status == "Active") {
                        rides.push_back(make_tuple(
                            stoi(fields[0]),  // ride_id
                            fields[3],         // pickup
                            fields[4],         // drop_location
                            fields[5],         // cab_type
                            fields[1],         // rider_name
                            stoi(fields[10]),  // passenger_count
                            stod(fields[7])    // fare
                        ));
                    }
                } catch (...) {
                    cout << "[DEBUG] Error parsing line" << endl;
                }
            }
        }
        file.close();
        
        cout << "[DEBUG] Total active rides found: " << rides.size() << endl;
        return rides;
    }

    bool updateRideStatus(int rideID, string status) {
        ifstream file(ridesFile);
        vector<string> lines;
        string line;
        
        while (getline(file, line)) {
            lines.push_back(line);
        }
        file.close();

        ofstream outFile(ridesFile);
        for (size_t i = 0; i < lines.size(); i++) {
            if (i == 0) {
                outFile << lines[i] << "\n";
                continue;
            }
            
            vector<string> fields = split(lines[i], ',');
            if (fields.size() >= 12 && stoi(fields[0]) == rideID) {
                fields[11] = status;
                outFile << fields[0];
                for (size_t j = 1; j < fields.size(); j++) {
                    outFile << "," << fields[j];
                }
                outFile << "\n";
            } else {
                outFile << lines[i] << "\n";
            }
        }
        outFile.close();
        return true;
    }

    bool addPassengerToRide(int rideID, string newRiderName) {
        ifstream file(ridesFile);
        vector<string> lines;
        string line;
        
        while (getline(file, line)) {
            lines.push_back(line);
        }
        file.close();

        ofstream outFile(ridesFile);
        for (size_t i = 0; i < lines.size(); i++) {
            if (i == 0) {
                outFile << lines[i] << "\n";
                continue;
            }
            
            vector<string> fields = split(lines[i], ',');
            if (fields.size() >= 12 && stoi(fields[0]) == rideID) {
                fields[1] = fields[1] + " & " + newRiderName;
                fields[10] = to_string(stoi(fields[10]) + 1);
                outFile << fields[0];
                for (size_t j = 1; j < fields.size(); j++) {
                    outFile << "," << fields[j];
                }
                outFile << "\n";
            } else {
                outFile << lines[i] << "\n";
            }
        }
        outFile.close();
        return true;
    }

    vector<tuple<int, string, string, string, double, int, double, string>> getRidesByPhone(string phone) {
        vector<tuple<int, string, string, string, double, int, double, string>> rides;
        ifstream file(ridesFile);
        string line;
        getline(file, line); // Skip header
        
        while (getline(file, line)) {
            if (line.empty()) continue;
            
            vector<string> fields = split(line, ',');
            if (fields.size() >= 15 && fields[2] == phone) {
                rides.push_back(make_tuple(
                    stoi(fields[0]),   // ride_id
                    fields[3],          // pickup
                    fields[4],          // drop_location
                    fields[5],          // cab_type
                    stod(fields[6]),    // distance
                    stoi(fields[10]),   // passenger_count
                    stod(fields[7]),    // fare
                    fields[11]          // status
                ));
            }
        }
        file.close();
        return rides;
    }

    double getFarePerPerson(int rideID) {
        ifstream file(ridesFile);
        string line;
        getline(file, line); // Skip header
        
        while (getline(file, line)) {
            if (line.empty()) continue;
            
            vector<string> fields = split(line, ',');
            if (fields.size() >= 12 && stoi(fields[0]) == rideID) {
                double fare = stod(fields[7]);
                int count = stoi(fields[10]);
                file.close();
                return fare / count;
            }
        }
        file.close();
        return 0.0;
    }
};

#endif
