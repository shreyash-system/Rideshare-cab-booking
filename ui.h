#ifndef UI_H
#define UI_H
#include <iostream>
#include <string>
#include <iomanip>
using namespace std;

class UI {
public:
    static void clearScreen() {
        #ifdef _WIN32
            system("cls");
        #else
            system("clear");
        #endif
    }

    static void printLine(char ch = '=', int length = 70) {
        for (int i = 0; i < length; i++) cout << ch;
        cout << endl;
    }

    static void printDoubleLine(int length = 70) {
        printLine('=', length);
    }

    static void printSingleLine(int length = 70) {
        printLine('-', length);
    }

    static void printHeader(string title, int width = 70) {
        int padding = (width - title.length() - 2) / 2;
        cout << "\n";
        printDoubleLine(width);
        cout << "|";
        for (int i = 0; i < padding; i++) cout << " ";
        cout << title;
        for (int i = 0; i < width - padding - title.length() - 2; i++) cout << " ";
        cout << "|" << endl;
        printDoubleLine(width);
    }

    static void printSubHeader(string title, int width = 70) {
        cout << "\n";
        printSingleLine(width);
        cout << "  " << title << endl;
        printSingleLine(width);
    }

    static void printBox(string content, int width = 70) {
        int contentLen = content.length();
        int padding = (width - contentLen - 4) / 2;
        
        cout << "+";
        for (int i = 0; i < width - 2; i++) cout << "-";
        cout << "+" << endl;
        
        cout << "|";
        for (int i = 0; i < padding; i++) cout << " ";
        cout << content;
        for (int i = 0; i < width - padding - contentLen - 2; i++) cout << " ";
        cout << "|" << endl;
        
        cout << "+";
        for (int i = 0; i < width - 2; i++) cout << "-";
        cout << "+" << endl;
    }

    static void printSuccess(string message) {
        cout << "\n";
        cout << "  [SUCCESS]  " << message << endl;
        cout << "\n";
    }

    static void printError(string message) {
        cout << "\n";
        cout << "  [ERROR]  " << message << endl;
        cout << "\n";
    }

    static void printInfo(string message) {
        cout << "\n";
        cout << "  [INFO]  " << message << endl;
        cout << "\n";
    }

    static void printWarning(string message) {
        cout << "\n";
        cout << "  [WARNING]  " << message << endl;
        cout << "\n";
    }

    static void printMenuOption(int number, string option) {
        cout << "  [" << number << "]  " << option << endl;
    }

    static void printTableHeader(string headers[], int sizes[], int count) {
        cout << "\n";
        printSingleLine(70);
        for (int i = 0; i < count; i++) {
            cout << left << setw(sizes[i]) << headers[i];
            if (i < count - 1) cout << " | ";
        }
        cout << endl;
        printSingleLine(70);
    }

    static void printSpacer(int lines = 1) {
        for (int i = 0; i < lines; i++) cout << endl;
    }

    static void printWelcomeBanner() {
        clearScreen();
        cout << "\n";
        printDoubleLine(70);
        cout << "|                                                                    |" << endl;
        cout << "|            RIDESHARE - YOUR JOURNEY BEGINS HERE                    |" << endl;
        cout << "|                                                                    |" << endl;
        printDoubleLine(70);
        printSpacer();
    }

    static void printGoodbye() {
        printSpacer();
        printBox("Thank you for using RideShare!", 70);
        printBox("Drive Safe! See you soon!", 70);
        printSpacer(2);
    }

    static void waitForEnter(bool wait = true) {
        cout << "\n  Press Enter to continue...";
        if (wait) cin.ignore();
        cin.get();
    }

    static void printRideCard(int id, string pickup, string drop, string cabType, int passengers, double fare) {
        // Truncate strings if they're too long
        const int MAX_LOCATION_WIDTH = 54;
        const int MAX_CABTYPE_WIDTH = 14;
        
        if (pickup.length() > MAX_LOCATION_WIDTH) {
            pickup = pickup.substr(0, MAX_LOCATION_WIDTH - 3) + "...";
        }
        if (drop.length() > MAX_LOCATION_WIDTH) {
            drop = drop.substr(0, MAX_LOCATION_WIDTH - 3) + "...";
        }
        if (cabType.length() > MAX_CABTYPE_WIDTH) {
            cabType = cabType.substr(0, MAX_CABTYPE_WIDTH - 3) + "...";
        }
        
        cout << "\n";
        cout << "  +----------------------------------------------------------------+" << endl;
        cout << "  | Ride ID: " << left << setw(55) << id << endl;
        cout << "  |----------------------------------------------------------------|" << endl;
        cout << "  |  From:  " << left << setw(MAX_LOCATION_WIDTH) << pickup << endl;
        cout << "  |  To:    " << left << setw(MAX_LOCATION_WIDTH) << drop << endl;
        cout << "  |----------------------------------------------------------------|" << endl;
        cout << "  |  Cab Type: " << left << setw(MAX_CABTYPE_WIDTH) << cabType;
        cout << "  Passengers: " << left << setw(2) << passengers;
        cout << "  Fare: Rs." << fixed << setprecision(2) << setw(10) << fare << " |" << endl;
        cout << "  +----------------------------------------------------------------+" << endl;
    }
    static void printProfileCard(string username, string name, string phone, string payment) {
        cout << "\n";
        cout << "  +----------------------------------+" << endl;
        cout << "  |         USER PROFILE             |" << endl;
        cout << "  +----------------------------------+" << endl;
        cout << "  | Username: " << left << setw(22) << username << "|" << endl;
        cout << "  | Name:     " << left << setw(22) << name << "|" << endl;
        cout << "  | Phone:    " << left << setw(22) << phone << "|" << endl;
        cout << "  | Payment:  " << left << setw(22) << payment << "|" << endl;
        cout << "  +----------------------------------+" << endl;
    }

    static void printBookingConfirmation(int rideID, string driver, string vehicle, string cab, double fare, string payment) {
        printSpacer();
        printDoubleLine(70);
        cout << "|                     BOOKING CONFIRMED!                           |" << endl;
        printDoubleLine(70);
        cout << "  Ride ID:        " << rideID << endl;
        cout << "  Driver:         " << driver << endl;
        cout << "  Vehicle:        " << vehicle << endl;
        cout << "  Cab Type:       " << cab << endl;
        cout << "  Total Fare:     Rs. " << fixed << setprecision(2) << fare << endl;
        cout << "  Payment:        " << payment << endl;
        printDoubleLine(70);
        printSpacer();
    }

    static string getColorCode(string color) {
        if (color == "RED") return "\033[31m";
        if (color == "GREEN") return "\033[32m";
        if (color == "YELLOW") return "\033[33m";
        if (color == "BLUE") return "\033[34m";
        if (color == "MAGENTA") return "\033[35m";
        if (color == "CYAN") return "\033[36m";
        if (color == "WHITE") return "\033[37m";
        return "\033[0m"; // RESET
    }

    static void printColored(string text, string color) {
        cout << getColorCode(color) << text << getColorCode("RESET");
    }
};

#endif
