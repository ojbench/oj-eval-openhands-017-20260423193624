

#ifndef MODELS_HPP
#define MODELS_HPP

#include <cstring>
#include <string>

namespace sjtu {

struct User {
    char username[21];
    char password[31];
    char name[21];
    char mailAddr[31];
    int privilege;

    User() {
        memset(username, 0, sizeof(username));
        memset(password, 0, sizeof(password));
        memset(name, 0, sizeof(name));
        memset(mailAddr, 0, sizeof(mailAddr));
        privilege = 0;
    }
};

struct Train {
    char trainID[21];
    int stationNum;
    char stations[101][31];
    int seatNum;
    int prices[101]; // prices[i] is price from station i to i+1
    int startTime; // in minutes from 00:00
    int travelTimes[101];
    int stopoverTimes[101];
    int saleDateStart; // days from 06-01
    int saleDateEnd;
    char type;
    bool released;

    Train() {
        memset(trainID, 0, sizeof(trainID));
        stationNum = 0;
        memset(stations, 0, sizeof(stations));
        seatNum = 0;
        memset(prices, 0, sizeof(prices));
        startTime = 0;
        memset(travelTimes, 0, sizeof(travelTimes));
        memset(stopoverTimes, 0, sizeof(stopoverTimes));
        saleDateStart = 0;
        saleDateEnd = 0;
        type = 0;
        released = false;
    }
};

enum OrderStatus { SUCCESS, PENDING, REFUNDED };

struct Order {
    OrderStatus status;
    char username[21];
    char trainID[21];
    char from[31];
    char to[31];
    int leavingTime; // absolute minutes
    int arrivingTime; // absolute minutes
    int price;
    int num;
    int timestamp;
    int startStationIndex;
    int endStationIndex;
    int date; // departure date from starting station

    Order() {
        status = SUCCESS;
        memset(trainID, 0, sizeof(trainID));
        memset(from, 0, sizeof(from));
        memset(to, 0, sizeof(to));
        leavingTime = 0;
        arrivingTime = 0;
        price = 0;
        num = 0;
        timestamp = 0;
        startStationIndex = 0;
        endStationIndex = 0;
        date = 0;
    }
};

// Helper for date/time
inline int dateToDays(int month, int day) {
    if (month == 6) return day - 1;
    if (month == 7) return 30 + day - 1;
    if (month == 8) return 30 + 31 + day - 1;
    return 0;
}

inline void daysToDate(int days, int& month, int& day) {
    if (days < 30) {
        month = 6;
        day = days + 1;
    } else if (days < 61) {
        month = 7;
        day = days - 30 + 1;
    } else {
        month = 8;
        day = days - 61 + 1;
    }
}

} // namespace sjtu

#endif

