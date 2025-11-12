#ifndef DATABASE_H
#define DATABASE_H

#include "user.h"
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <openssl/sha.h>
#include <sqlite3.h>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

using namespace std;

class Database {
private:
  sqlite3 *db;
  string dbFile = "data/rideshare.db";

public:
  Database() {
    system("mkdir data 2>nul");

    int rc = sqlite3_open(dbFile.c_str(), &db);
    if (rc) {
      cerr << "[ERROR] Can't open database: " << sqlite3_errmsg(db) << endl;
      exit(1);
    }

    initializeTables();
    cout << "[OK] Database initialized (SQLite-based)!" << endl;
  }

  ~Database() { sqlite3_close(db); }

private:
  // SHA256 password hashing
  string hashPassword(const string &password) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char *)password.c_str(), password.size(), hash);
    stringstream ss;
    for (unsigned char c : hash)
      ss << hex << setw(2) << setfill('0') << (int)c;
    return ss.str();
  }

  void initializeTables() {
    char *errMsg = nullptr;

    // Create users table
    const char *sqlUsers = "CREATE TABLE IF NOT EXISTS users ("
                           "username TEXT PRIMARY KEY,"
                           "fullname TEXT NOT NULL,"
                           "password_hash TEXT NOT NULL,"
                           "phone TEXT NOT NULL,"
                           "emergency_contact TEXT,"
                           "emergency_phone TEXT,"
                           "payment_method TEXT DEFAULT 'Cash'"
                           ");";

    if (sqlite3_exec(db, sqlUsers, nullptr, nullptr, &errMsg) != SQLITE_OK) {
      cerr << "[ERROR] SQL error (users): " << errMsg << endl;
      sqlite3_free(errMsg);
    }

    // Create drivers table
    const char *sqlDrivers = "CREATE TABLE IF NOT EXISTS drivers ("
                             "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                             "name TEXT NOT NULL,"
                             "phone TEXT NOT NULL,"
                             "vehicle_number TEXT NOT NULL,"
                             "cab_type TEXT NOT NULL"
                             ");";

    if (sqlite3_exec(db, sqlDrivers, nullptr, nullptr, &errMsg) != SQLITE_OK) {
      cerr << "[ERROR] SQL error (drivers): " << errMsg << endl;
      sqlite3_free(errMsg);
    }

    // Create rides table
    const char *sqlRides = "CREATE TABLE IF NOT EXISTS rides ("
                           "ride_id INTEGER PRIMARY KEY AUTOINCREMENT,"
                           "rider_name TEXT NOT NULL,"
                           "rider_phone TEXT NOT NULL,"
                           "pickup TEXT NOT NULL,"
                           "drop_location TEXT NOT NULL,"
                           "cab_type TEXT NOT NULL,"
                           "distance REAL NOT NULL,"
                           "fare REAL NOT NULL,"
                           "driver_name TEXT NOT NULL,"
                           "vehicle_number TEXT NOT NULL,"
                           "passenger_count INTEGER NOT NULL,"
                           "status TEXT NOT NULL,"
                           "payment_method TEXT NOT NULL,"
                           "scheduled_date TEXT,"
                           "scheduled_time TEXT"
                           ");";

    if (sqlite3_exec(db, sqlRides, nullptr, nullptr, &errMsg) != SQLITE_OK) {
      cerr << "[ERROR] SQL error (rides): " << errMsg << endl;
      sqlite3_free(errMsg);
    }
  }

public:
  bool insertUser(string username, string fullname, string phone,
                  string passwordPlain) {
    string passwordHash = hashPassword(passwordPlain);

    sqlite3_stmt *stmt;
    const char *sql = "INSERT INTO users (username, fullname, password_hash, "
                      "phone, payment_method) "
                      "VALUES (?, ?, ?, ?, 'Cash');";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
      cerr << "[ERROR] Failed to prepare statement: " << sqlite3_errmsg(db)
           << endl;
      return false;
    }

    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, fullname.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, passwordHash.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, phone.c_str(), -1, SQLITE_TRANSIENT);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return rc == SQLITE_DONE;
  }

  bool checkUsernameExists(string username) {
    sqlite3_stmt *stmt;
    const char *sql = "SELECT COUNT(*) FROM users WHERE username = ?;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
      return false;
    }

    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);

    bool exists = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
      exists = sqlite3_column_int(stmt, 0) > 0;
    }

    sqlite3_finalize(stmt);
    return exists;
  }

  int getUserByUsername(string username, string password, string &fullname,
                        string &phone, string &emergencyContact,
                        string &emergencyPhone, string &paymentMethod) {
    // returns:
    // 1 = success
    // 0 = user not found
    // -1 = wrong password

    sqlite3_stmt *stmt;
    const char *sql =
        "SELECT fullname, password_hash, phone, emergency_contact, "
        "emergency_phone, payment_method FROM users WHERE username = ?;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
      return 0;
    }

    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);

    int result = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
      string storedHash =
          reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
      string passwordHash = hashPassword(password);

      if (storedHash != passwordHash) {
        result = -1;
      } else {
        fullname = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
        phone = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));

        const unsigned char *ec = sqlite3_column_text(stmt, 3);
        emergencyContact = ec ? reinterpret_cast<const char *>(ec) : "";

        const unsigned char *ep = sqlite3_column_text(stmt, 4);
        emergencyPhone = ep ? reinterpret_cast<const char *>(ep) : "";

        const unsigned char *pm = sqlite3_column_text(stmt, 5);
        paymentMethod = pm ? reinterpret_cast<const char *>(pm) : "Cash";

        result = 1;
      }
    }

    sqlite3_finalize(stmt);
    return result;
  }

  bool updateUserEmergencyContact(string username, string ec, string ep) {
    sqlite3_stmt *stmt;
    const char *sql =
        "UPDATE users SET emergency_contact = ?, emergency_phone = ? "
        "WHERE username = ?;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
      return false;
    }

    sqlite3_bind_text(stmt, 1, ec.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, ep.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, username.c_str(), -1, SQLITE_TRANSIENT);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return rc == SQLITE_DONE;
  }

  bool updateUserPaymentMethod(string username, string pm) {
    sqlite3_stmt *stmt;
    const char *sql = "UPDATE users SET payment_method = ? WHERE username = ?;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
      return false;
    }

    sqlite3_bind_text(stmt, 1, pm.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, username.c_str(), -1, SQLITE_TRANSIENT);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return rc == SQLITE_DONE;
  }

  bool insertDriver(string name, string phone, string vehicleNumber,
                    string cabType) {
    sqlite3_stmt *stmt;
    const char *sql =
        "INSERT INTO drivers (name, phone, vehicle_number, cab_type) "
        "VALUES (?, ?, ?, ?);";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
      return false;
    }

    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, phone.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, vehicleNumber.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, cabType.c_str(), -1, SQLITE_TRANSIENT);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return rc == SQLITE_DONE;
  }

  vector<Driver> getAllDrivers() {
    vector<Driver> drivers;
    sqlite3_stmt *stmt;
    const char *sql =
        "SELECT name, phone, vehicle_number, cab_type FROM drivers;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
      return drivers;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
      string name =
          reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
      string phone =
          reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
      string vehicle =
          reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));
      string cabType =
          reinterpret_cast<const char *>(sqlite3_column_text(stmt, 3));

      drivers.push_back(Driver(name, phone, vehicle, cabType));
    }

    sqlite3_finalize(stmt);
    return drivers;
  }

  int insertRide(string riderName, string riderPhone, string pickup,
                 string drop, string cabType, double distance, double fare,
                 string driverName, string vehicleNumber, int passengerCount,
                 string status, string paymentMethod, string scheduledDate,
                 string scheduledTime) {
    sqlite3_stmt *stmt;
    const char *sql =
        "INSERT INTO rides (rider_name, rider_phone, pickup, drop_location, "
        "cab_type, distance, fare, driver_name, vehicle_number, "
        "passenger_count, status, payment_method, scheduled_date, "
        "scheduled_time) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
      return -1;
    }

    sqlite3_bind_text(stmt, 1, riderName.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, riderPhone.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, pickup.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, drop.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, cabType.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 6, distance);
    sqlite3_bind_double(stmt, 7, fare);
    sqlite3_bind_text(stmt, 8, driverName.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 9, vehicleNumber.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 10, passengerCount);
    sqlite3_bind_text(stmt, 11, status.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 12, paymentMethod.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 13, scheduledDate.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 14, scheduledTime.c_str(), -1, SQLITE_TRANSIENT);

    int rc = sqlite3_step(stmt);
    int rideId = -1;

    if (rc == SQLITE_DONE) {
      rideId = sqlite3_last_insert_rowid(db);
    }

    sqlite3_finalize(stmt);
    return rideId;
  }

  vector<tuple<int, string, string, string, string, int, double>>
  getActiveRides() {
    vector<tuple<int, string, string, string, string, int, double>> rides;
    sqlite3_stmt *stmt;
    const char *sql =
        "SELECT ride_id, pickup, drop_location, cab_type, rider_name, "
        "passenger_count, fare FROM rides WHERE status = 'Active';";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
      return rides;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
      rides.push_back(make_tuple(
          sqlite3_column_int(stmt, 0),
          reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1)),
          reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2)),
          reinterpret_cast<const char *>(sqlite3_column_text(stmt, 3)),
          reinterpret_cast<const char *>(sqlite3_column_text(stmt, 4)),
          sqlite3_column_int(stmt, 5), sqlite3_column_double(stmt, 6)));
    }

    sqlite3_finalize(stmt);
    cout << "[DEBUG] Total active rides found: " << rides.size() << endl;
    return rides;
  }

  bool updateRideStatus(int rideID, string status) {
    sqlite3_stmt *stmt;
    const char *sql = "UPDATE rides SET status = ? WHERE ride_id = ?;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
      return false;
    }

    sqlite3_bind_text(stmt, 1, status.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, rideID);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return rc == SQLITE_DONE;
  }

  bool addPassengerToRide(int rideID, string newRiderName) {
    sqlite3_stmt *stmt;
    const char *sql =
        "UPDATE rides SET rider_name = rider_name || ' & ' || ?, "
        "passenger_count = passenger_count + 1 WHERE ride_id = ?;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
      return false;
    }

    sqlite3_bind_text(stmt, 1, newRiderName.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, rideID);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return rc == SQLITE_DONE;
  }

  vector<tuple<int, string, string, string, double, int, double, string>>
  getRidesByPhone(string phone) {
    vector<tuple<int, string, string, string, double, int, double, string>>
        rides;
    sqlite3_stmt *stmt;
    const char *sql =
        "SELECT ride_id, pickup, drop_location, cab_type, distance, "
        "passenger_count, fare, status FROM rides WHERE rider_phone = ?;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
      return rides;
    }

    sqlite3_bind_text(stmt, 1, phone.c_str(), -1, SQLITE_TRANSIENT);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
      rides.push_back(make_tuple(
          sqlite3_column_int(stmt, 0),
          reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1)),
          reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2)),
          reinterpret_cast<const char *>(sqlite3_column_text(stmt, 3)),
          sqlite3_column_double(stmt, 4), sqlite3_column_int(stmt, 5),
          sqlite3_column_double(stmt, 6),
          reinterpret_cast<const char *>(sqlite3_column_text(stmt, 7))));
    }

    sqlite3_finalize(stmt);
    return rides;
  }

  vector<tuple<int, string, string, string, double, int, double, string>>
  getRidesByUsername(string username) {
    vector<tuple<int, string, string, string, double, int, double, string>>
        rides;

    // First, get the phone number for this username
    sqlite3_stmt *stmt1;
    const char *sql1 = "SELECT phone FROM users WHERE username = ?;";

    if (sqlite3_prepare_v2(db, sql1, -1, &stmt1, nullptr) != SQLITE_OK) {
      return rides;
    }

    sqlite3_bind_text(stmt1, 1, username.c_str(), -1, SQLITE_TRANSIENT);

    string phone = "";
    if (sqlite3_step(stmt1) == SQLITE_ROW) {
      phone = reinterpret_cast<const char *>(sqlite3_column_text(stmt1, 0));
    }
    sqlite3_finalize(stmt1);

    // If no phone found, return empty vector
    if (phone.empty()) {
      return rides;
    }

    // Now get rides for this phone number
    sqlite3_stmt *stmt2;
    const char *sql2 =
        "SELECT ride_id, pickup, drop_location, cab_type, distance, "
        "passenger_count, fare, status FROM rides WHERE rider_phone = ?;";

    if (sqlite3_prepare_v2(db, sql2, -1, &stmt2, nullptr) != SQLITE_OK) {
      return rides;
    }

    sqlite3_bind_text(stmt2, 1, phone.c_str(), -1, SQLITE_TRANSIENT);

    while (sqlite3_step(stmt2) == SQLITE_ROW) {
      rides.push_back(make_tuple(
          sqlite3_column_int(stmt2, 0),
          reinterpret_cast<const char *>(sqlite3_column_text(stmt2, 1)),
          reinterpret_cast<const char *>(sqlite3_column_text(stmt2, 2)),
          reinterpret_cast<const char *>(sqlite3_column_text(stmt2, 3)),
          sqlite3_column_double(stmt2, 4), sqlite3_column_int(stmt2, 5),
          sqlite3_column_double(stmt2, 6),
          reinterpret_cast<const char *>(sqlite3_column_text(stmt2, 7))));
    }

    sqlite3_finalize(stmt2);
    return rides;
  }

  double getFarePerPerson(int rideID) {
    sqlite3_stmt *stmt;
    const char *sql =
        "SELECT fare, passenger_count FROM rides WHERE ride_id = ?;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
      return 0.0;
    }

    sqlite3_bind_int(stmt, 1, rideID);

    double farePerPerson = 0.0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
      double fare = sqlite3_column_double(stmt, 0);
      int count = sqlite3_column_int(stmt, 1);
      farePerPerson = fare / count;
    }

    sqlite3_finalize(stmt);
    return farePerPerson;
  }
};

#endif
