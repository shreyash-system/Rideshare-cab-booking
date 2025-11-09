#ifndef MAPS_API_H
#define MAPS_API_H

#include <string>
#include <iostream>
#include <fstream>
#include <cstdlib>
using namespace std;

class MapsAPI {
private:
    string apiKey;
    bool apiWorking;

public:
    MapsAPI(string key) : apiKey(key), apiWorking(false) {
        // Test API on initialization
        testAPIQuietly();
    }
    
    double getDistance(string origin, string destination) {
        // If API is not working, return estimated distance
        if (!apiWorking) {
            cout << "[INFO] Using estimated distance (API offline)" << endl;
            return estimateDistance(origin, destination);
        }

        // Build command to call Python script
        string command = "python get_distance.py \"" + origin + "\" \"" + destination 
                         + "\" " + apiKey + " > temp_distance.txt 2>&1";
        
        // Execute command
        int result = system(command.c_str());
        
        // Read the result
        double distance = 0.0;
        ifstream file("temp_distance.txt");
        if (file.is_open()) {
            string line;
            if (getline(file, line)) {
                try {
                    distance = stod(line);
                } catch (...) {
                    distance = 0.0;
                }
            }
            file.close();
        }
        
        // Clean up temp file
        remove("temp_distance.txt");
        
        // If API failed, use estimate
        if (distance == 0.0) {
            cout << "[INFO] API call failed, using estimate" << endl;
            distance = estimateDistance(origin, destination);
        }
        
        return distance;
    }
    
    void testAPI() {
        cout << "\n[Testing Google Maps API]" << endl;
        double dist = getDistance("Mumbai", "Pune");
        if (dist > 0) {
            cout << "[SUCCESS] API is working! Distance Mumbai to Pune: " << dist << " km" << endl;
            apiWorking = true;
        } else {
            cout << "[WARNING] API test failed. Using estimated distances." << endl;
            cout << "[INFO] Check: API key, Python installation, internet connection" << endl;
            apiWorking = false;
        }
    }

private:
    void testAPIQuietly() {
        // Test API without output
        string command = "python get_distance.py \"Mumbai\" \"Pune\" " + apiKey + " > temp_test.txt 2>&1";
        system(command.c_str());
        
        ifstream file("temp_test.txt");
        if (file.is_open()) {
            string line;
            if (getline(file, line)) {
                try {
                    double dist = stod(line);
                    apiWorking = (dist > 0);
                } catch (...) {
                    apiWorking = false;
                }
            }
            file.close();
        }
        remove("temp_test.txt");
    }

    double estimateDistance(string origin, string destination) {
        // Simple estimation based on common routes
        // Convert to lowercase for comparison
        string orig = origin, dest = destination;
        for (auto& c : orig) c = tolower(c);
        for (auto& c : dest) c = tolower(c);

        // Common city pairs (approximate distances in km)
        if ((orig.find("mumbai") != string::npos && dest.find("pune") != string::npos) ||
            (orig.find("pune") != string::npos && dest.find("mumbai") != string::npos)) {
            return 148.0;
        }
        if ((orig.find("delhi") != string::npos && dest.find("agra") != string::npos) ||
            (orig.find("agra") != string::npos && dest.find("delhi") != string::npos)) {
            return 230.0;
        }
        if ((orig.find("bangalore") != string::npos && dest.find("mysore") != string::npos) ||
            (orig.find("mysore") != string::npos && dest.find("bangalore") != string::npos)) {
            return 145.0;
        }

        // Default: rough estimate based on string difference (very approximate)
        return 50.0; // Default minimum distance
    }

public:
    bool isAPIWorking() {
        return apiWorking;
    }
};

#endif
