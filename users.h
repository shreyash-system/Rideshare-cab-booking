#ifndef USERS_H
#define USERS_H
#include "database.h"
#include <iostream>
#include <string>
using namespace std;

string trim(const string &str);

class UserAccount {
public:
  string username;
  string fullName;
  string phone;
  string emergencyContact;
  string emergencyPhone;
  string paymentMethod;

  UserAccount() { paymentMethod = "Cash"; }

  UserAccount(string u, string n, string p) {
    username = u;
    fullName = n;
    phone = p;
    paymentMethod = "Cash";
    emergencyContact = "";
    emergencyPhone = "";
  }

  void displayProfile() {
    cout << "\n====== Your Profile ======" << endl;
    cout << "Username: " << username << endl;
    cout << "Full Name: " << fullName << endl;
    cout << "Phone: " << phone << endl;
    cout << "Emergency Contact: "
         << (emergencyContact.empty() ? "Not set" : emergencyContact) << endl;
    cout << "Emergency Phone: "
         << (emergencyPhone.empty() ? "Not set" : emergencyPhone) << endl;
    cout << "Payment Method: " << paymentMethod << endl;
    cout << "==========================" << endl;
  }
};

class UserAuth {
private:
  static Database *db;

public:
  static void setDatabase(Database *database) { db = database; }

  static UserAccount *login() {
    string username, password;
    cout << "\nEnter username: ";
    cin >> username;

    bool userExists = db->checkUsernameExists(username);

    if (userExists) {
      cin.ignore();
      cout << "Enter password: ";
      getline(cin, password); // to handle spaces in password

      string fullName, phone, emergencyContact, emergencyPhone, paymentMethod;
      int resultCode = db->getUserByUsername(username, password, fullName,
                                             phone, emergencyContact,
                                             emergencyPhone, paymentMethod);

      if (resultCode == 1) {
        UserAccount *user = new UserAccount(username, fullName, phone);
        user->emergencyContact = emergencyContact;
        user->emergencyPhone = emergencyPhone;
        user->paymentMethod = paymentMethod;

        user->displayProfile();
        return user;
      } else if (resultCode == -1) {
        cout << "\nIncorrect password!" << endl;
        return nullptr;
      }
    }

    // User not found - offer registration
    cout << "\nUsername not found. Would you like to create a new account? "
            "(Y/N): ";
    char choice;
    cin >> choice;
    if (choice == 'Y' || choice == 'y') {
      return registerUser(username);
    }
    return nullptr;
  }

  static UserAccount *registerUser(string username) {
    string fullName, phone, password;
    cout << "\n====== Registration ======" << endl;
    cout << "Username: " << username << endl;
    cin.ignore();
    cout << "Password: ";
    getline(cin, password);
    cout << "Enter full name: ";
    getline(cin, fullName);
    cout << "Enter phone (10 digits): ";
    getline(cin, phone);

    while (phone.length() != 10 || !isValidPhone(phone)) {
      cout << "Invalid phone. Enter 10 digits: ";
      getline(cin, phone);
    }

    // Save to database
    if (db->insertUser(username, fullName, phone, password)) {
      UserAccount *newUser = new UserAccount(username, fullName, phone);
      cout << "\nAccount created successfully!" << endl;
      return newUser;
    } else {
      cout << "\nFailed to create account!" << endl;
      return nullptr;
    }
  }

  static void setEmergencyContact(UserAccount *user) {
    cout << "\n====== Emergency Contact Setup ======" << endl;
    cin.ignore();
    cout << "Enter emergency contact name: ";
    getline(cin, user->emergencyContact);
    cout << "Enter emergency contact phone (10 digits): ";
    getline(cin, user->emergencyPhone);

    while (user->emergencyPhone.length() != 10 ||
           !isValidPhone(user->emergencyPhone)) {
      cout << "Invalid phone. Enter 10 digits: ";
      getline(cin, user->emergencyPhone);
    }

    // Update in database
    if (db->updateUserEmergencyContact(user->username, user->emergencyContact,
                                       user->emergencyPhone)) {
      cout << "Emergency contact saved!" << endl;
    } else {
      cout << "Failed to save emergency contact!" << endl;
    }
  }

  static void setPaymentMethod(UserAccount *user) {
    cout << "\n====== Payment Method ======" << endl;
    cout << "1. Cash" << endl;
    cout << "2. Credit/Debit Card" << endl;
    cout << "3. UPI" << endl;
    cout << "4. Wallet" << endl;
    cout << "Select payment method: ";
    int choice;
    cin >> choice;

    switch (choice) {
    case 1:
      user->paymentMethod = "Cash";
      break;
    case 2:
      user->paymentMethod = "Card";
      break;
    case 3:
      user->paymentMethod = "UPI";
      break;
    case 4:
      user->paymentMethod = "Wallet";
      break;
    default:
      user->paymentMethod = "Cash";
    }

    // Update in database
    if (db->updateUserPaymentMethod(user->username, user->paymentMethod)) {
      cout << "Payment method updated to: " << user->paymentMethod << endl;
    } else {
      cout << "Failed to update payment method!" << endl;
    }
  }

private:
  static bool isValidPhone(const string &phone) {
    if (phone.length() != 10)
      return false;
    for (char c : phone) {
      if (!isdigit(c))
        return false;
    }
    return true;
  }
};

// Initialize static member
Database *UserAuth::db = nullptr;

string trim(const string &str) {
  size_t first = str.find_first_not_of(" \t\n\r");
  if (first == string::npos)
    return "";
  size_t last = str.find_last_not_of(" \t\n\r");
  return str.substr(first, last - first + 1);
}

#endif
