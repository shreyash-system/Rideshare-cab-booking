# Rideshare-cab-booking
RideShare is a fully-featured command-line cab booking and ride management system written in modern C++ using Object Oriented Programming.

## Command to compile:
```bash
 g++ -std=c++11 main.cpp -o main.exe -lcurl -lssl -lcrypto -lcurl -pthread -lsqlite3 -lssl -lcrypto && ./main.exe
 ``````

## Overview
RideShare is a fully-featured command-line cab booking and ride management system written in modern C++ using Object Oriented Programming. It enables users to register as riders or drivers, book and share rides, manage ride history, and handle payments. Data is stored in CSV files (not .txt) for speed and reliability, using robust file handling and validation.


## Features
Rider and Driver registration (with phone verification and payment setup)

Multiple cab types: Mini, Prime, SUV

Book rides with automatic fare calculation (distance × cab type)

Random assignment of available drivers

CSV-based persistent storage (no database server required)

View all active rides (in progress), ride history, and profile

Ride sharing: Split fare equally between passengers (with support for joining rides)

Modify emergency contact information and payment method

Cancel rides (with status tracked in the system)

Robust input validation and error handling

Simple, clear menu-driven CLI interface — no GUI required

## File Structure
main.cpp — Application entry, menu logic, and feature orchestration

cab.h — Cab type classes (Mini, Prime, SUV, with fare logic)

user.h — Core user classes (abstract User, Rider, Driver)

users.h — User authentication, registration, emergency contact & payment info

database.h — All CSV file handling and database logic

ui.h — Console screen formatting and menu utilities (for clear, pretty CLI)

maps_api.h — Google Maps integration for real distance calculations

get_distance.py — Python script called by C++ to fetch route distances from Google Maps

data/users.csv — Users and profile info

data/drivers.csv — All registered drivers

data/rides.csv — All rides (active, historical, and shared)
............................................................
