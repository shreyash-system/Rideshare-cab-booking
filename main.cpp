#include "cab.h"
#include "database.h"
#include "maps_api.h"
#include "ui.h"
#include "user.h"
#include "users.h"
#include <algorithm>
#include <cctype>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <string>
#include <tuple>
using namespace std;

int main() {
  try {
    // Seed random number generator
    srand(time(0));

    // Initialize database connection
    Database db;

    // Connect UserAuth to Database
    UserAuth::setDatabase(&db);

    // Initialize Google Maps API
    MapsAPI maps("AIzaSyDy5JBAYFpOlQLtKjwgpbfXPMoD4Cmo2_0");

    UI::printWelcomeBanner();
    UI::printInfo("CSV Database Active | Google Maps API Integrated");
    UI::waitForEnter();

    UI::clearScreen();
    UI::printHeader("USER LOGIN", 70);
    UserAccount *currentUser = UserAuth::login();

    if (currentUser == nullptr) {
      UI::printError("Login failed. Exiting...");
      return 0;
    }

    UI::printSuccess("Login successful! Welcome, " + currentUser->fullName);
    UI::waitForEnter();

    int choice;

    while (true) {
      UI::clearScreen();
      UI::printSpacer();
      UI::printHeader("RIDESHARE SYSTEM", 70);
      cout << "  Logged in as: " << currentUser->fullName << " ("
           << currentUser->phone << ")" << endl;
      UI::printSpacer();
      UI::printMenuOption(1, "Register Driver");
      UI::printMenuOption(2, "Book Ride");
      UI::printMenuOption(3, "Schedule Ride");
      UI::printMenuOption(4, "Share Ride");
      UI::printMenuOption(5, "Cancel Ride");
      UI::printMenuOption(6, "View Active Rides");
      UI::printMenuOption(7, "View My Ride History");
      UI::printMenuOption(8, "Set Emergency Contact");
      UI::printMenuOption(9, "Trigger SOS");
      UI::printMenuOption(10, "Change Payment Method");
      UI::printMenuOption(11, "View Profile");
      UI::printMenuOption(12, "Logout");
      UI::printSpacer();
      UI::printSingleLine(70);
      cout << "  Enter your choice: ";

      if (!(cin >> choice)) {
        cin.clear();
        cin.ignore(10000, '\n');
        UI::printError("Invalid input! Please enter a number.");
        UI::waitForEnter();
        continue;
      }

      if (choice == 1) {
        // Register Driver
        UI::clearScreen();
        UI::printSubHeader("REGISTER DRIVER", 70);
        string name, phone, vNum, cType;
        cin.ignore();
        cout << "\n  Enter driver name: ";
        getline(cin, name);
        cout << "  Enter phone (10 digits): ";
        getline(cin, phone);
        while (phone.length() != 10) {
          UI::printError("Invalid phone number!");
          cout << "  Enter phone (10 digits): ";
          getline(cin, phone);
        }
        cout << "  Enter vehicle number: ";
        getline(cin, vNum);
        cout << "  Enter cab type (Mini/Prime/SUV): ";
        getline(cin, cType);

        // Validate cab type
        transform(cType.begin(), cType.end(), cType.begin(), ::tolower);
        if (cType == "mini")
          cType = "Mini";
        else if (cType == "prime")
          cType = "Prime";
        else if (cType == "suv")
          cType = "SUV";
        else {
          UI::printError("Invalid cab type! Using 'Mini' as default.");
          cType = "Mini";
        }

        if (db.insertDriver(name, phone, vNum, cType)) {
          UI::printSuccess("Driver registered successfully!");
        } else {
          UI::printError("Failed to register driver!");
        }
        UI::waitForEnter();
      } else if (choice == 2 || choice == 3) {
        // Book Ride or Schedule Ride
        UI::clearScreen();
        UI::printSubHeader(choice == 2 ? "BOOK RIDE" : "SCHEDULE RIDE", 70);

        string pickup, drop, scheduledDate = "", scheduledTime = "";
        double distance;
        int cabChoice;

        cin.ignore();
        cout << "\n  Enter pickup location: ";
        getline(cin, pickup);
        cout << "  Enter drop location: ";
        getline(cin, drop);

        // Use Google Maps API to calculate distance
        cout << "\n  Calculating distance using Google Maps..." << endl;
        distance = maps.getDistance(pickup, drop);

        if (distance > 0) {
          cout << "  [OK] Distance calculated: " << fixed << setprecision(2)
               << distance << " km" << endl;
        } else {
          cout << "  [WARNING] Could not calculate distance automatically."
               << endl;
          cout << "  Enter distance manually (in km): ";
          cin >> distance;
        }

        if (choice == 3) {
          cin.ignore();
          cout << "  Enter date (DD/MM/YYYY): ";
          getline(cin, scheduledDate);
          cout << "  Enter time (HH:MM): ";
          getline(cin, scheduledTime);
        }

        UI::printSpacer();
        UI::printInfo("Select Cab Type:");
        cout << "    [1] Mini   (Rs.50 base + Rs.10/km)" << endl;
        cout << "    [2] Prime  (Rs.80 base + Rs.15/km)" << endl;
        cout << "    [3] SUV    (Rs.100 base + Rs.20/km)" << endl;
        cout << "\n  Your choice: ";
        cin >> cabChoice;

        // Get all drivers from database
        vector<Driver> drivers = db.getAllDrivers();
        if (drivers.empty()) {
          UI::printError(
              "No drivers available! Please register drivers first.");
          UI::waitForEnter();
          continue;
        }

        // Assign random driver
        int driverIndex = rand() % drivers.size();
        Driver assignedDriver = drivers[driverIndex];

        // Calculate fare
        Cab *selectedCab = NULL;
        string cabType;
        if (cabChoice == 1) {
          selectedCab = new Mini();
          cabType = "Mini";
        } else if (cabChoice == 2) {
          selectedCab = new Prime();
          cabType = "Prime";
        } else {
          selectedCab = new SUV();
          cabType = "SUV";
        }

        double fare = selectedCab->calculateFare(distance);

        // Insert ride into database
        int rideID = db.insertRide(
            currentUser->fullName, currentUser->phone, pickup, drop, cabType,
            distance, fare, assignedDriver.getName(),
            assignedDriver.getVehicleNum(), 1, "Active",
            currentUser->paymentMethod, scheduledDate, scheduledTime);

        if (rideID > 0) {
          UI::printSpacer();
          UI::printBookingConfirmation(rideID, assignedDriver.getName(),
                                       assignedDriver.getVehicleNum(), cabType,
                                       fare, currentUser->paymentMethod);
          if (!scheduledDate.empty()) {
            cout << "  Scheduled For: " << scheduledDate << " at "
                 << scheduledTime << endl;
            UI::printSpacer();
          }
        } else {
          UI::printError("Failed to book ride!");
        }

        delete selectedCab;
        UI::waitForEnter();
      } else if (choice == 4) {
        // Share Ride
        UI::clearScreen();
        UI::printSubHeader("SHARE RIDE", 70);

        vector<tuple<int, string, string, string, string, int, double>>
            activeRides = db.getActiveRides();
        if (activeRides.empty()) {
          UI::printInfo("No rides available for sharing.");
          cout << "\n  Would you like to create a new ride? (Y/N): ";
          char ch;
          cin >> ch;
          if (ch != 'Y' && ch != 'y') {
            continue;
          }

          // Create new ride flow
          string pickup, drop;
          double distance;
          int cabChoice;
          cin.ignore();
          cout << "\n  Enter pickup location: ";
          getline(cin, pickup);
          cout << "  Enter drop location: ";
          getline(cin, drop);

          cout << "\n  Calculating distance..." << endl;
          distance = maps.getDistance(pickup, drop);

          if (distance > 0) {
            cout << "  [OK] Distance: " << fixed << setprecision(2) << distance
                 << " km" << endl;
          } else {
            cout << "  Enter distance manually (in km): ";
            cin >> distance;
          }

          cout << "\n  Select Cab Type: [1] Mini [2] Prime [3] SUV: ";
          cin >> cabChoice;

          vector<Driver> drivers = db.getAllDrivers();
          if (!drivers.empty()) {
            int driverIndex = rand() % drivers.size();
            Driver assignedDriver = drivers[driverIndex];

            Cab *selectedCab = NULL;
            string cabType;
            if (cabChoice == 1) {
              selectedCab = new Mini();
              cabType = "Mini";
            } else if (cabChoice == 2) {
              selectedCab = new Prime();
              cabType = "Prime";
            } else {
              selectedCab = new SUV();
              cabType = "SUV";
            }

            double fare = selectedCab->calculateFare(distance);

            int rideID = db.insertRide(
                currentUser->fullName, currentUser->phone, pickup, drop,
                cabType, distance, fare, assignedDriver.getName(),
                assignedDriver.getVehicleNum(), 1, "Active",
                currentUser->paymentMethod, "", "");

            if (rideID > 0) {
              UI::printSuccess("Ride created! Waiting for others to join...");
              UI::printInfo("Ride ID: " + to_string(rideID));

              // Ask if they want to add passengers now
              cout << "\n  Would you like to add passengers to this ride now? "
                      "(Y/N): ";
              char addNow;
              cin >> addNow;
              cin.ignore();

              if (addNow == 'Y' || addNow == 'y') {
                cout << "\n  How many passengers would you like to add? ";
                int numPassengers;
                cin >> numPassengers;
                cin.ignore();

                for (int i = 0; i < numPassengers; i++) {
                  cout << "\n  --- Passenger " << (i + 1) << " ---" << endl;
                  cout << "  1. Add existing user (by username)" << endl;
                  cout << "  2. Create temporary passenger" << endl;
                  cout << "  Choose option (1-2): ";

                  int option;
                  cin >> option;
                  cin.ignore();

                  if (option == 1) {
                    // Add existing user
                    cout << "  Enter username: ";
                    string username;
                    getline(cin, username);

                    if (db.checkUsernameExists(username)) {
                      // Get user details
                      string fullname, phone, ec, ep, pm;
                      string tempPass = ""; // We don't need password for this

                      // We need a method to get user by username without
                      // password For now, we'll just use the username as the
                      // name
                      if (db.addPassengerToRide(rideID, username)) {
                        UI::printSuccess("User " + username +
                                         " added to ride!");
                      } else {
                        UI::printError("Failed to add user to ride.");
                      }
                    } else {
                      UI::printError("Username not found!");
                      cout << "  Would you like to create a new user? (Y/N): ";
                      char createNew;
                      cin >> createNew;
                      cin.ignore();

                      if (createNew == 'Y' || createNew == 'y') {
                        string fullname, phone, password;
                        cout << "  Enter full name: ";
                        getline(cin, fullname);
                        cout << "  Enter phone: ";
                        getline(cin, phone);
                        cout << "  Enter password for new user: ";
                        getline(cin, password);

                        if (db.insertUser(username, fullname, phone,
                                          password)) {
                          UI::printSuccess("New user created!");
                          if (db.addPassengerToRide(rideID, fullname)) {
                            UI::printSuccess("User added to ride!");
                          }
                        } else {
                          UI::printError("Failed to create user.");
                        }
                      } else {
                        i--; // Retry this passenger slot
                      }
                    }
                  } else if (option == 2) {
                    // Create temporary passenger
                    cout << "  Enter passenger name: ";
                    string passengerName;
                    getline(cin, passengerName);

                    if (db.addPassengerToRide(rideID, passengerName)) {
                      UI::printSuccess("Passenger " + passengerName +
                                       " added to ride!");
                    } else {
                      UI::printError("Failed to add passenger.");
                    }
                  } else {
                    UI::printError("Invalid option. Skipping this passenger.");
                  }
                }

                // Show updated fare
                double farePerPerson = db.getFarePerPerson(rideID);
                UI::printInfo("Updated fare per person: Rs." +
                              to_string(farePerPerson));
              }
            }
            delete selectedCab;
          }
          UI::waitForEnter();
          continue;
        }

        cout << "\n  Available Rides for Sharing:" << endl;
        UI::printSpacer();
        for (size_t i = 0; i < activeRides.size(); i++) {
          int rideID = get<0>(activeRides[i]);
          string pickup = get<1>(activeRides[i]);
          string drop = get<2>(activeRides[i]);
          string cabType = get<3>(activeRides[i]);
          string riderName = get<4>(activeRides[i]);
          int passengers = get<5>(activeRides[i]);
          double fare = get<6>(activeRides[i]);

          UI::printRideCard(rideID, pickup, drop, cabType, passengers,
                            fare / passengers);
        }

        cout << "\n  Enter Ride ID to join (0 to cancel): ";
        int rideID;
        cin >> rideID;
        cin.ignore();

        if (rideID == 0) {
          continue;
        }

        // Verify ride exists
        bool rideFound = false;
        for (const auto &ride : activeRides) {
          if (get<0>(ride) == rideID) {
            rideFound = true;
            break;
          }
        }

        if (!rideFound) {
          UI::printError("Ride not found!");
          UI::waitForEnter();
          continue;
        }

        // Add other passengers
        cout << "\n  How many passengers to add? ";
        int numPassengers;
        cin >> numPassengers;
        cin.ignore();

        int successfulAdds = 0;
        for (int i = 0; i < numPassengers; i++) {
          cout << "\n  --- Passenger " << (i + 1) << " ---" << endl;
          cout << "  1. Add existing user (by username)" << endl;
          cout << "  2. Create temporary passenger" << endl;
          cout << "  3. Create new permanent user" << endl;
          cout << "  Choose option (1-3): ";

          int option;
          cin >> option;
          cin.ignore();

          if (option == 1) {
            // Add existing user
            cout << "  Enter username: ";
            string username;
            getline(cin, username);

            if (db.checkUsernameExists(username)) {
              if (db.addPassengerToRide(rideID, username)) {
                UI::printSuccess("User " + username + " added to ride!");
                successfulAdds++;
              } else {
                UI::printError("Failed to add user to ride.");
              }
            } else {
              UI::printError("Username not found!");
              i--; // Retry this passenger
            }
          } else if (option == 2) {
            // Create temporary passenger
            cout << "  Enter passenger name: ";
            string passengerName;
            getline(cin, passengerName);

            if (db.addPassengerToRide(rideID, passengerName)) {
              UI::printSuccess("Passenger " + passengerName +
                               " added to ride!");
              successfulAdds++;
            } else {
              UI::printError("Failed to add passenger.");
            }
          } else if (option == 3) {
            // Create new permanent user
            string username, fullname, phone, password;

            cout << "  Enter username: ";
            getline(cin, username);

            if (db.checkUsernameExists(username)) {
              UI::printError("Username already exists!");
              i--; // Retry
              continue;
            }

            cout << "  Enter full name: ";
            getline(cin, fullname);
            cout << "  Enter phone: ";
            getline(cin, phone);
            cout << "  Enter password: ";
            getline(cin, password);

            if (db.insertUser(username, fullname, phone, password)) {
              UI::printSuccess("New user created!");
              if (db.addPassengerToRide(rideID, fullname)) {
                UI::printSuccess("User added to ride!");
                successfulAdds++;
              }
            } else {
              UI::printError("Failed to create user.");
            }
          } else {
            UI::printError("Invalid option.");
            i--; // Retry
          }
        }

        if (successfulAdds > 0) {
          double farePerPerson = db.getFarePerPerson(rideID);
          UI::printSuccess(to_string(successfulAdds) +
                           " passenger(s) added successfully!");
          cout << "  Updated fare per person: Rs." << fixed << setprecision(2)
               << farePerPerson << endl;
        }

        UI::waitForEnter();
      } else if (choice == 5) {
        // Cancel Ride
        UI::clearScreen();
        UI::printSubHeader("CANCEL RIDE", 70);

        vector<tuple<int, string, string, string, double, int, double, string>>
            rideHistory = db.getRidesByPhone(currentUser->phone);
        if (rideHistory.empty()) {
          UI::printInfo("No rides found!");
          UI::waitForEnter();
          continue;
        }

        cout << "\n  Your Active Rides:" << endl;
        cout << "  "
                "=============================================================="
                "=========="
             << endl;
        cout << "  ID\tPickup\t\tDrop\t\tStatus\t\tFare" << endl;
        cout << "  "
                "=============================================================="
                "=========="
             << endl;

        bool hasActiveRides = false;
        for (size_t i = 0; i < rideHistory.size(); i++) {
          int rideID = get<0>(rideHistory[i]);
          string pickup = get<1>(rideHistory[i]);
          string drop = get<2>(rideHistory[i]);
          string status = get<7>(rideHistory[i]);
          double fare = get<6>(rideHistory[i]);

          if (status == "Active") {
            hasActiveRides = true;
            cout << "  " << rideID << "\t" << pickup.substr(0, 12) << "\t"
                 << drop.substr(0, 12) << "\t" << status << "\t\t" << fixed
                 << setprecision(2) << fare << endl;
          }
        }

        if (!hasActiveRides) {
          UI::printInfo("No active rides to cancel!");
          UI::waitForEnter();
          continue;
        }

        cout << "\n  Enter Ride ID to cancel (0 to go back): ";
        int rideID;
        cin >> rideID;

        if (rideID == 0) {
          continue;
        }

        if (db.updateRideStatus(rideID, "Cancelled")) {
          UI::printSuccess("Ride cancelled successfully!");
          UI::printWarning("Cancellation charges: Rs.50");
        } else {
          UI::printError("Ride not found or already cancelled!");
        }
        UI::waitForEnter();
      } else if (choice == 6) {
        // View Active Rides
        UI::clearScreen();
        UI::printSubHeader("ALL ACTIVE RIDES", 70);
        vector<tuple<int, string, string, string, string, int, double>>
            activeRides = db.getActiveRides();

        if (activeRides.empty()) {
          UI::printInfo("No active rides at the moment!");
        } else {
          cout << "\n  Found " << activeRides.size() << " active ride(s):\n"
               << endl;

          for (size_t i = 0; i < activeRides.size(); i++) {
            int rideID = get<0>(activeRides[i]);
            string pickup = get<1>(activeRides[i]);
            string drop = get<2>(activeRides[i]);
            string cabType = get<3>(activeRides[i]);
            string riderNames =
                get<4>(activeRides[i]); // This contains all rider names
            int passengers = get<5>(activeRides[i]);
            double fare = get<6>(activeRides[i]);
            double farePerPerson = fare / passengers;

            // Print detailed ride card
            cout << "  "
                    "+---------------------------------------------------------"
                    "-------+"
                 << endl;
            cout << "  | Ride ID: " << left << setw(55) << rideID << endl;
            cout << "  "
                    "|---------------------------------------------------------"
                    "-------|"
                 << endl;
            cout << "  |  From:  " << left << setw(54) << pickup << endl;
            cout << "  |  To:    " << left << setw(54) << drop << endl;
            cout << "  "
                    "|---------------------------------------------------------"
                    "-------|"
                 << endl;
            cout << "  |  Cab Type: " << left << setw(51) << cabType << endl;
            cout << "  |  Passengers (" << passengers << "): " << left
                 << setw(46) << riderNames << endl;
            cout << "  "
                    "|---------------------------------------------------------"
                    "-------|"
                 << endl;
            cout << "  |  Total Fare: Rs." << left << setw(13) << fixed
                 << setprecision(2) << fare;
            cout << "  Fare/Person: Rs." << left << setw(15) << farePerPerson
                 << " |" << endl;
            cout << "  "
                    "+---------------------------------------------------------"
                    "-------+"
                 << endl;
            cout << endl;
          }
        }
        UI::waitForEnter();
      } else if (choice == 7) {
        // View Ride History
        UI::clearScreen();
        UI::printSubHeader("MY RIDE HISTORY", 70);

        vector<tuple<int, string, string, string, double, int, double, string>>
            rideHistory = db.getRidesByPhone(currentUser->phone);
        if (rideHistory.empty()) {
          UI::printInfo("No ride history available!");
        } else {
          cout << "\n";
          for (size_t i = 0; i < rideHistory.size(); i++) {
            int rideID = get<0>(rideHistory[i]);
            string pickup = get<1>(rideHistory[i]);
            string drop = get<2>(rideHistory[i]);
            string cabType = get<3>(rideHistory[i]);
            double distance = get<4>(rideHistory[i]);
            int passengers = get<5>(rideHistory[i]);
            double totalFare = get<6>(rideHistory[i]);
            string status = get<7>(rideHistory[i]);

            UI::printRideCard(rideID, pickup, drop, cabType, passengers,
                              totalFare);
            cout << "  Distance: " << fixed << setprecision(2) << distance
                 << " km | Status: " << status << endl;
            UI::printSpacer();
          }
        }
        UI::waitForEnter();
      } else if (choice == 8) {
        // Set Emergency Contact
        UI::clearScreen();
        UI::printSubHeader("SET EMERGENCY CONTACT", 70);
        string emergencyContact, emergencyPhone;
        cin.ignore();
        cout << "\n  Enter emergency contact name: ";
        getline(cin, emergencyContact);
        cout << "  Enter emergency contact phone (10 digits): ";
        getline(cin, emergencyPhone);

        while (emergencyPhone.length() != 10) {
          UI::printError("Invalid phone number!");
          cout << "  Enter emergency contact phone (10 digits): ";
          getline(cin, emergencyPhone);
        }

        if (db.updateUserEmergencyContact(currentUser->username,
                                          emergencyContact, emergencyPhone)) {
          currentUser->emergencyContact = emergencyContact;
          currentUser->emergencyPhone = emergencyPhone;
          UI::printSuccess("Emergency contact saved!");
        } else {
          UI::printError("Failed to save emergency contact!");
        }
        UI::waitForEnter();
      } else if (choice == 9) {
        // Trigger SOS
        UI::clearScreen();
        UI::printSubHeader("EMERGENCY SOS", 70);
        if (currentUser->emergencyContact.empty()) {
          UI::printError(
              "No emergency contact set! Please set one first (Option 8).");
        } else {
          UI::printSpacer();
          cout << "  ====== SOS ALERT TRIGGERED ======" << endl;
          UI::printSuccess("Emergency contact has been notified!");
          cout << "\n  Contact: " << currentUser->emergencyContact << " ("
               << currentUser->emergencyPhone << ")" << endl;
          cout << "\n  SMS sent: 'EMERGENCY! I need help! - "
               << currentUser->fullName << "'" << endl;
          cout << "  ==================================" << endl;
          UI::printSpacer();
        }
        UI::waitForEnter();
      } else if (choice == 10) {
        // Change Payment Method
        UI::clearScreen();
        UI::printSubHeader("CHANGE PAYMENT METHOD", 70);
        cout << "\n  Current payment method: " << currentUser->paymentMethod
             << endl;
        UI::printSpacer();
        cout << "  Select new payment method:" << endl;
        cout << "    [1] Cash" << endl;
        cout << "    [2] Credit/Debit Card" << endl;
        cout << "    [3] UPI" << endl;
        cout << "    [4] Wallet" << endl;
        cout << "\n  Your choice: ";
        int payChoice;
        cin >> payChoice;

        string paymentMethod = "Cash";
        switch (payChoice) {
        case 1:
          paymentMethod = "Cash";
          break;
        case 2:
          paymentMethod = "Card";
          break;
        case 3:
          paymentMethod = "UPI";
          break;
        case 4:
          paymentMethod = "Wallet";
          break;
        default:
          UI::printError("Invalid choice! Keeping current method.");
          UI::waitForEnter();
          continue;
        }

        if (db.updateUserPaymentMethod(currentUser->username, paymentMethod)) {
          currentUser->paymentMethod = paymentMethod;
          UI::printSuccess("Payment method updated to: " + paymentMethod);
        } else {
          UI::printError("Failed to update payment method!");
        }
        UI::waitForEnter();
      } else if (choice == 11) {
        // View Profile
        UI::clearScreen();
        UI::printSubHeader("USER PROFILE", 70);
        UI::printProfileCard(currentUser->username, currentUser->fullName,
                             currentUser->phone, currentUser->paymentMethod);
        if (!currentUser->emergencyContact.empty()) {
          cout << "\n  Emergency Contact: " << currentUser->emergencyContact
               << " (" << currentUser->emergencyPhone << ")" << endl;
        } else {
          UI::printWarning("No emergency contact set!");
        }
        UI::waitForEnter();
      } else if (choice == 12) {
        UI::clearScreen();
        auto rides = db.getRidesByUsername(currentUser->username);

        // Check if there are any active rides
        bool hasActiveRides = false;
        vector<int> activeRideIds;

        for (const auto &ride : rides) {
          string status = get<7>(ride); // status is at index 7
          if (status == "Active") {
            hasActiveRides = true;
            activeRideIds.push_back(get<0>(ride)); // ride_id at index 0
          }
        }

        if (hasActiveRides) {
          cout << "\n  ⚠️  WARNING: You have active rides!" << endl;
          cout << "\n  Active Ride IDs: ";
          for (size_t i = 0; i < activeRideIds.size(); i++) {
            cout << activeRideIds[i];
            if (i < activeRideIds.size() - 1)
              cout << ", ";
          }
          cout << endl;

          cout << "\n  Before logging out, please mark your rides as:" << endl;
          cout << "  1. Complete (finished ride)" << endl;
          cout << "  2. Cancelled (cancelled ride)" << endl;
          cout << "\n  Choose action (1-2): ";

          int action;
          cin >> action;
          cin.ignore();

          string newStatus;
          if (action == 1) {
            newStatus = "Completed";
          } else if (action == 2) {
            newStatus = "Cancelled";
          } else {
            cout << "\n  ❌ Invalid choice. Logout cancelled." << endl;
            cout << "\n  Press Enter to continue...";
            cin.get();
            continue;
          }

          // Update all active rides
          for (int rideId : activeRideIds) {
            db.updateRideStatus(rideId, newStatus);
          }

          cout << "\n  ✓ All active rides marked as " << newStatus << endl;
          cout << "  You can now logout safely." << endl;
        }

        UI::clearScreen();
        UI::printGoodbye();
        delete currentUser;
        break;
      } else {
        UI::printError("Invalid choice! Please select 1-12.");
        UI::waitForEnter();
      }
    }

  } catch (exception &e) {
    cerr << "\n[FATAL ERROR] " << e.what() << endl;
    return 1;
  }

  return 0;
}
