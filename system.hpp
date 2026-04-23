

#ifndef SYSTEM_HPP
#define SYSTEM_HPP

#include <iostream>
#include <string>
#include <iomanip>
#include "utility.hpp"
#include "BPTree.hpp"
#include "models.hpp"

namespace sjtu {

struct StationTrainKey {
    FixedString<31> station;
    FixedString<21> trainID;
    bool operator<(const StationTrainKey& other) const {
        if (!(station == other.station)) return station < other.station;
        return trainID < other.trainID;
    }
};

struct UserOrderKey {
    FixedString<21> username;
    int timestamp_neg;
    bool operator<(const UserOrderKey& other) const {
        if (!(username == other.username)) return username < other.username;
        return timestamp_neg < other.timestamp_neg;
    }
};

struct TrainDateKey {
    FixedString<21> trainID;
    int date;
    bool operator<(const TrainDateKey& other) const {
        if (!(trainID == other.trainID)) return trainID < other.trainID;
        return date < other.date;
    }
};

struct PendingOrderKey {
    FixedString<21> trainID;
    int date;
    int timestamp;
    bool operator<(const PendingOrderKey& other) const {
        if (!(trainID == other.trainID)) return trainID < other.trainID;
        if (date != other.date) return date < other.date;
        return timestamp < other.timestamp;
    }
};

struct SeatData {
    int seats[100];
};

class System {
private:
    BPTree<FixedString<21>, User> users;
    BPTree<FixedString<21>, Train> trains;
    BPTree<StationTrainKey, int> station_trains;
    BPTree<UserOrderKey, Order> orders;
    BPTree<TrainDateKey, SeatData> seats;
    BPTree<PendingOrderKey, Order> pending_orders;

    struct LoggedUser {
        FixedString<21> username;
        int privilege;
    };
    vector<LoggedUser> logged_in;

    int timestamp_counter;

    int get_privilege(const char* username) {
        for (size_t i = 0; i < logged_in.size(); ++i) {
            if (strcmp(logged_in[i].username.data, username) == 0) return logged_in[i].privilege;
        }
        return -1;
    }

    bool is_logged_in(const char* username) {
        return get_privilege(username) != -1;
    }

public:
    System() : users("users.db"), trains("trains.db"), station_trains("station_trains.db"),
               orders("orders.db"), seats("seats.db"), pending_orders("pending_orders.db"),
               timestamp_counter(0) {}

    void add_user(const char* cur_u, const char* u, const char* p, const char* n, const char* m, int g) {
        User newUser;
        User dummy;
        if (!users.find(FixedString<21>("admin_check"), dummy)) {
            // First user
            strcpy(newUser.username, u);
            strcpy(newUser.password, p);
            strcpy(newUser.name, n);
            strcpy(newUser.mailAddr, m);
            newUser.privilege = 10;
            users.insert(FixedString<21>(u), newUser);
            users.insert(FixedString<21>("admin_check"), dummy);
            std::cout << "0" << std::endl;
            return;
        }

        int cur_priv = get_privilege(cur_u);
        if (cur_priv != -1 && cur_priv > g && !users.find(FixedString<21>(u), dummy)) {
            strcpy(newUser.username, u);
            strcpy(newUser.password, p);
            strcpy(newUser.name, n);
            strcpy(newUser.mailAddr, m);
            newUser.privilege = g;
            users.insert(FixedString<21>(u), newUser);
            std::cout << "0" << std::endl;
        } else {
            std::cout << "-1" << std::endl;
        }
    }

    void login(const char* u, const char* p) {
        User user;
        if (users.find(FixedString<21>(u), user) && strcmp(user.password, p) == 0 && !is_logged_in(u)) {
            logged_in.push_back({FixedString<21>(u), user.privilege});
            std::cout << "0" << std::endl;
        } else {
            std::cout << "-1" << std::endl;
        }
    }

    void logout(const char* u) {
        for (size_t i = 0; i < logged_in.size(); ++i) {
            if (strcmp(logged_in[i].username.data, u) == 0) {
                for (size_t j = i; j < logged_in.size() - 1; ++j) logged_in[j] = logged_in[j + 1];
                logged_in.pop_back();
                std::cout << "0" << std::endl;
                return;
            }
        }
        std::cout << "-1" << std::endl;
    }

    void query_profile(const char* c, const char* u) {
        int cur_priv = get_privilege(c);
        User user;
        if (cur_priv != -1 && users.find(FixedString<21>(u), user)) {
            if (strcmp(c, u) == 0 || cur_priv > user.privilege) {
                std::cout << user.username << " " << user.name << " " << user.mailAddr << " " << user.privilege << std::endl;
                return;
            }
        }
        std::cout << "-1" << std::endl;
    }

    void modify_profile(const char* c, const char* u, const char* p, const char* n, const char* m, int g) {
        int cur_priv = get_privilege(c);
        User user;
        if (cur_priv != -1 && users.find(FixedString<21>(u), user)) {
            if (strcmp(c, u) == 0 || cur_priv > user.privilege) {
                if (g != -1) {
                    if (g >= cur_priv) { std::cout << "-1" << std::endl; return; }
                    user.privilege = g;
                }
                if (p) strcpy(user.password, p);
                if (n) strcpy(user.name, n);
                if (m) strcpy(user.mailAddr, m);
                users.update(FixedString<21>(u), user);
                // Update logged_in privilege if necessary
                for (size_t i = 0; i < logged_in.size(); ++i) {
                    if (strcmp(logged_in[i].username.data, u) == 0) {
                        logged_in[i].privilege = user.privilege;
                        break;
                    }
                }
                std::cout << user.username << " " << user.name << " " << user.mailAddr << " " << user.privilege << std::endl;
                return;
            }
        }
        std::cout << "-1" << std::endl;
    }

    void add_train(const Train& t) {
        Train dummy;
        if (!trains.find(FixedString<21>(t.trainID), dummy)) {
            trains.insert(FixedString<21>(t.trainID), t);
            std::cout << "0" << std::endl;
        } else {
            std::cout << "-1" << std::endl;
        }
    }

    void release_train(const char* i) {
        Train t;
        if (trains.find(FixedString<21>(i), t) && !t.released) {
            t.released = true;
            trains.update(FixedString<21>(i), t);
            for (int j = 0; j < t.stationNum; ++j) {
                station_trains.insert({FixedString<31>(t.stations[j]), FixedString<21>(t.trainID)}, j);
            }
            // Initialize seats for all sale dates
            SeatData sd;
            for (int k = 0; k < 100; ++k) sd.seats[k] = t.seatNum;
            for (int d = t.saleDateStart; d <= t.saleDateEnd; ++d) {
                seats.insert({FixedString<21>(t.trainID), d}, sd);
            }
            std::cout << "0" << std::endl;
        } else {
            std::cout << "-1" << std::endl;
        }
    }

    void query_train(const char* i, int date) {
        Train t;
        if (trains.find(FixedString<21>(i), t) && date >= t.saleDateStart && date <= t.saleDateEnd) {
            std::cout << t.trainID << " " << t.type << std::endl;
            SeatData sd;
            seats.find({FixedString<21>(t.trainID), date}, sd);
            int cur_time = t.startTime;
            int cur_price = 0;
            for (int j = 0; j < t.stationNum; ++j) {
                std::cout << t.stations[j] << " ";
                if (j == 0) std::cout << "xx-xx xx:xx";
                else {
                    int m, d;
                    daysToDate(date + cur_time / 1440, m, d);
                    std::cout << std::setfill('0') << std::setw(2) << m << "-" << std::setw(2) << d << " "
                              << std::setw(2) << (cur_time % 1440) / 60 << ":" << std::setw(2) << cur_time % 60;
                }
                std::cout << " -> ";
                if (j == t.stationNum - 1) std::cout << "xx-xx xx:xx";
                else {
                    int leave_time = cur_time + (j == 0 ? 0 : t.stopoverTimes[j - 1]);
                    int m, d;
                    daysToDate(date + leave_time / 1440, m, d);
                    std::cout << std::setfill('0') << std::setw(2) << m << "-" << std::setw(2) << d << " "
                              << std::setw(2) << (leave_time % 1440) / 60 << ":" << std::setw(2) << leave_time % 60;
                }
                std::cout << " " << cur_price << " ";
                if (j == t.stationNum - 1) std::cout << "x" << std::endl;
                else std::cout << sd.seats[j] << std::endl;

                if (j < t.stationNum - 1) {
                    cur_time += (j == 0 ? 0 : t.stopoverTimes[j - 1]) + t.travelTimes[j];
                    cur_price += t.prices[j];
                }
            }
        } else {
            std::cout << "-1" << std::endl;
        }
    }

    void delete_train(const char* i) {
        Train t;
        if (trains.find(FixedString<21>(i), t) && !t.released) {
            trains.remove(FixedString<21>(i));
            std::cout << "0" << std::endl;
        } else {
            std::cout << "-1" << std::endl;
        }
    }

    struct TicketInfo {
        char trainID[21];
        int leavingTime;
        int arrivingTime;
        int price;
        int seat;
        int duration;
        void print(const char* from, const char* to) const {
            int m1, d1, m2, d2;
            daysToDate(leavingTime / 1440, m1, d1);
            daysToDate(arrivingTime / 1440, m2, d2);
            std::cout << trainID << " " << from << " " << std::setfill('0') << std::setw(2) << m1 << "-" << std::setw(2) << d1 << " "
                      << std::setw(2) << (leavingTime % 1440) / 60 << ":" << std::setw(2) << leavingTime % 60 << " -> "
                      << to << " " << std::setw(2) << m2 << "-" << std::setw(2) << d2 << " "
                      << std::setw(2) << (arrivingTime % 1440) / 60 << ":" << std::setw(2) << arrivingTime % 60 << " "
                      << price << " " << seat << std::endl;
        }
    };

    void query_ticket(const char* s, const char* t, int date, bool sort_by_time) {
        vector<TicketInfo> results;
        auto it = station_trains.lower_bound({FixedString<31>(s), FixedString<21>("")});
        while (it.isValid() && strcmp(it.key().station.data, s) == 0) {
            FixedString<21> tid = it.key().trainID;
            int s_idx = it.value();
            Train train;
            trains.find(tid, train);
            int t_idx = -1;
            for (int j = s_idx + 1; j < train.stationNum; ++j) {
                if (strcmp(train.stations[j], t) == 0) { t_idx = j; break; }
            }
            if (t_idx != -1) {
                int s_offset = 0;
                for (int j = 0; j < s_idx; ++j) s_offset += (j == 0 ? 0 : train.stopoverTimes[j - 1]) + train.travelTimes[j];
                int start_date = date - s_offset / 1440;
                if (start_date >= train.saleDateStart && start_date <= train.saleDateEnd) {
                    TicketInfo info;
                    strcpy(info.trainID, train.trainID);
                    info.leavingTime = start_date * 1440 + train.startTime + s_offset + (s_idx == 0 ? 0 : train.stopoverTimes[s_idx - 1]);
                    int t_offset = s_offset;
                    for (int j = s_idx; j < t_idx; ++j) t_offset += (j == 0 ? 0 : train.stopoverTimes[j - 1]) + train.travelTimes[j];
                    info.arrivingTime = start_date * 1440 + train.startTime + t_offset;
                    info.duration = info.arrivingTime - info.leavingTime;
                    info.price = 0;
                    for (int j = s_idx; j < t_idx; ++j) info.price += train.prices[j];
                    SeatData sd;
                    seats.find({tid, start_date}, sd);
                    info.seat = 1000000;
                    for (int j = s_idx; j < t_idx; ++j) if (sd.seats[j] < info.seat) info.seat = sd.seats[j];
                    results.push_back(info);
                }
            }
            it.next();
        }

        if (sort_by_time) {
            sort(&results[0], &results[0] + results.size(), [](const TicketInfo& a, const TicketInfo& b) {
                if (a.duration != b.duration) return a.duration < b.duration;
                return strcmp(a.trainID, b.trainID) < 0;
            });
        } else {
            sort(&results[0], &results[0] + results.size(), [](const TicketInfo& a, const TicketInfo& b) {
                if (a.price != b.price) return a.price < b.price;
                return strcmp(a.trainID, b.trainID) < 0;
            });
        }

        std::cout << results.size() << std::endl;
        for (size_t i = 0; i < results.size(); ++i) results[i].print(s, t);
    }

    void query_transfer(const char* s, const char* t, int date, bool sort_by_time) {
        // Simplified transfer: iterate all trains from s, then all trains to t, find common station
        // This can be slow, but let's try.
        bool found = false;
        TicketInfo best1, best2;
        char mid_station[31];
        int best_val = 2e9;
        int best_val2 = 2e9;

        auto it1 = station_trains.lower_bound({FixedString<31>(s), FixedString<21>("")});
        while (it1.isValid() && strcmp(it1.key().station.data, s) == 0) {
            Train train1; trains.find(it1.key().trainID, train1);
            int s_idx = it1.value();
            int s_offset = 0;
            for (int j = 0; j < s_idx; ++j) s_offset += (j == 0 ? 0 : train1.stopoverTimes[j - 1]) + train1.travelTimes[j];
            int start_date1 = date - s_offset / 1440;
            if (start_date1 >= train1.saleDateStart && start_date1 <= train1.saleDateEnd) {
                int leave1 = start_date1 * 1440 + train1.startTime + s_offset + (s_idx == 0 ? 0 : train1.stopoverTimes[s_idx - 1]);
                int cur_price1 = 0;
                int cur_offset1 = s_offset;
                for (int j = s_idx + 1; j < train1.stationNum; ++j) {
                    cur_offset1 += (j == 1 ? 0 : train1.stopoverTimes[j - 2]) + train1.travelTimes[j - 1];
                    cur_price1 += train1.prices[j - 1];
                    int arrive1 = start_date1 * 1440 + train1.startTime + cur_offset1;
                    const char* mid = train1.stations[j];
                    
                    auto it2 = station_trains.lower_bound({FixedString<31>(mid), FixedString<21>("")});
                    while (it2.isValid() && strcmp(it2.key().station.data, mid) == 0) {
                        if (strcmp(it2.key().trainID.data, train1.trainID) == 0) { it2.next(); continue; }
                        Train train2; trains.find(it2.key().trainID, train2);
                        int mid_idx = it2.value();
                        int t_idx = -1;
                        for (int k = mid_idx + 1; k < train2.stationNum; ++k) {
                            if (strcmp(train2.stations[k], t) == 0) { t_idx = k; break; }
                        }
                        if (t_idx != -1) {
                            int mid_offset = 0;
                            for (int k = 0; k < mid_idx; ++k) mid_offset += (k == 0 ? 0 : train2.stopoverTimes[k - 1]) + train2.travelTimes[k];
                            int leave2_base = train2.startTime + mid_offset + (mid_idx == 0 ? 0 : train2.stopoverTimes[mid_idx - 1]);
                            int start_date2 = -1;
                            for (int d = train2.saleDateStart; d <= train2.saleDateEnd; ++d) {
                                if (d * 1440 + leave2_base >= arrive1) { start_date2 = d; break; }
                            }
                            if (start_date2 != -1) {
                                int leave2 = start_date2 * 1440 + leave2_base;
                                int t_offset = mid_offset;
                                int price2 = 0;
                                for (int k = mid_idx; k < t_idx; ++k) {
                                    t_offset += (k == 0 ? 0 : train2.stopoverTimes[k - 1]) + train2.travelTimes[k];
                                    price2 += train2.prices[k];
                                }
                                int arrive2 = start_date2 * 1440 + train2.startTime + t_offset;
                                int total_time = arrive2 - leave1;
                                int total_price = cur_price1 + price2;
                                bool update = false;
                                if (sort_by_time) {
                                    if (total_time < best_val || (total_time == best_val && total_price < best_val2)) update = true;
                                } else {
                                    if (total_price < best_val || (total_price == best_val && total_time < best_val2)) update = true;
                                }
                                if (update) {
                                    found = true;
                                    best_val = sort_by_time ? total_time : total_price;
                                    best_val2 = sort_by_time ? total_price : total_time;
                                    strcpy(best1.trainID, train1.trainID);
                                    best1.leavingTime = leave1; best1.arrivingTime = arrive1; best1.price = cur_price1;
                                    SeatData sd1; seats.find({FixedString<21>(train1.trainID), start_date1}, sd1);
                                    best1.seat = 1e6; for (int k = s_idx; k < j; ++k) if (sd1.seats[k] < best1.seat) best1.seat = sd1.seats[k];
                                    strcpy(best2.trainID, train2.trainID);
                                    best2.leavingTime = leave2; best2.arrivingTime = arrive2; best2.price = price2;
                                    SeatData sd2; seats.find({FixedString<21>(train2.trainID), start_date2}, sd2);
                                    best2.seat = 1e6; for (int k = mid_idx; k < t_idx; ++k) if (sd2.seats[k] < best2.seat) best2.seat = sd2.seats[k];
                                    strcpy(mid_station, mid);
                                }
                            }
                        }
                        it2.next();
                    }
                }
            }
            it1.next();
        }
        if (!found) std::cout << "0" << std::endl;
        else {
            best1.print(s, mid_station);
            best2.print(mid_station, t);
        }
    }

    void buy_ticket(const char* u, const char* i, int date, int n, const char* f, const char* t, bool q) {
        if (!is_logged_in(u)) { std::cout << "-1" << std::endl; return; }
        Train train;
        if (!trains.find(FixedString<21>(i), train)) { std::cout << "-1" << std::endl; return; }
        int f_idx = -1, t_idx = -1;
        for (int j = 0; j < train.stationNum; ++j) {
            if (strcmp(train.stations[j], f) == 0) f_idx = j;
            if (strcmp(train.stations[j], t) == 0) t_idx = j;
        }
        if (f_idx == -1 || t_idx == -1 || f_idx >= t_idx) { std::cout << "-1" << std::endl; return; }
        int f_offset = 0;
        for (int j = 0; j < f_idx; ++j) f_offset += (j == 0 ? 0 : train.stopoverTimes[j - 1]) + train.travelTimes[j];
        int start_date = date - f_offset / 1440;
        if (start_date < train.saleDateStart || start_date > train.saleDateEnd) { std::cout << "-1" << std::endl; return; }
        if (n > train.seatNum) { std::cout << "-1" << std::endl; return; }

        SeatData sd;
        seats.find({FixedString<21>(i), start_date}, sd);
        int min_seat = 1e6;
        for (int j = f_idx; j < t_idx; ++j) if (sd.seats[j] < min_seat) min_seat = sd.seats[j];

        Order order;
        order.status = (min_seat >= n) ? SUCCESS : (q ? PENDING : SUCCESS);
        if (order.status == SUCCESS && min_seat < n) { std::cout << "-1" << std::endl; return; }
        
        strcpy(order.username, u);
        strcpy(order.trainID, i); strcpy(order.from, f); strcpy(order.to, t);
        order.leavingTime = start_date * 1440 + train.startTime + f_offset + (f_idx == 0 ? 0 : train.stopoverTimes[f_idx - 1]);
        int t_offset = f_offset;
        order.price = 0;
        for (int j = f_idx; j < t_idx; ++j) {
            t_offset += (j == 0 ? 0 : train.stopoverTimes[j - 1]) + train.travelTimes[j];
            order.price += train.prices[j];
        }
        order.price *= n;
        order.arrivingTime = start_date * 1440 + train.startTime + t_offset;
        order.num = n; order.timestamp = timestamp_counter++;
        order.startStationIndex = f_idx; order.endStationIndex = t_idx; order.date = start_date;

        if (order.status == SUCCESS) {
            for (int j = f_idx; j < t_idx; ++j) sd.seats[j] -= n;
            seats.update({FixedString<21>(i), start_date}, sd);
            std::cout << order.price << std::endl;
        } else {
            pending_orders.insert({FixedString<21>(i), start_date, order.timestamp}, order);
            std::cout << "queue" << std::endl;
        }
        orders.insert({FixedString<21>(u), -order.timestamp}, order);
    }

    void query_order(const char* u) {
        if (!is_logged_in(u)) { std::cout << "-1" << std::endl; return; }
        vector<Order> res;
        auto it = orders.lower_bound({FixedString<21>(u), -2000000000});
        while (it.isValid() && strcmp(it.key().username.data, u) == 0) {
            res.push_back(it.value());
            it.next();
        }
        std::cout << res.size() << std::endl;
        for (size_t i = 0; i < res.size(); ++i) {
            const Order& o = res[i];
            std::cout << "[" << (o.status == SUCCESS ? "success" : (o.status == PENDING ? "pending" : "refunded")) << "] "
                      << o.trainID << " " << o.from << " ";
            int m1, d1, m2, d2;
            daysToDate(o.leavingTime / 1440, m1, d1);
            daysToDate(o.arrivingTime / 1440, m2, d2);
            std::cout << std::setfill('0') << std::setw(2) << m1 << "-" << std::setw(2) << d1 << " "
                      << std::setw(2) << (o.leavingTime % 1440) / 60 << ":" << std::setw(2) << o.leavingTime % 60 << " -> "
                      << o.to << " " << std::setw(2) << m2 << "-" << std::setw(2) << d2 << " "
                      << std::setw(2) << (o.arrivingTime % 1440) / 60 << ":" << std::setw(2) << o.arrivingTime % 60 << " "
                      << o.price / o.num << " " << o.num << std::endl;
        }
    }

    void refund_ticket(const char* u, int n) {
        if (!is_logged_in(u)) { std::cout << "-1" << std::endl; return; }
        auto it = orders.lower_bound({FixedString<21>(u), -2000000000});
        for (int i = 1; i < n && it.isValid(); ++i) it.next();
        if (!it.isValid() || strcmp(it.key().username.data, u) != 0) { std::cout << "-1" << std::endl; return; }
        Order o = it.value();
        if (o.status == REFUNDED) { std::cout << "-1" << std::endl; return; }
        OrderStatus old_status = o.status;
        o.status = REFUNDED;
        orders.update(it.key(), o);
        if (old_status == PENDING) {
            pending_orders.remove({FixedString<21>(o.trainID), o.date, o.timestamp});
        } else {
            SeatData sd; seats.find({FixedString<21>(o.trainID), o.date}, sd);
            for (int j = o.startStationIndex; j < o.endStationIndex; ++j) sd.seats[j] += o.num;
            // Check pending orders
            auto pit = pending_orders.lower_bound({FixedString<21>(o.trainID), o.date, 0});
            while (pit.isValid() && strcmp(pit.key().trainID.data, o.trainID) == 0 && pit.key().date == o.date) {
                Order po = pit.value();
                int min_s = 1e6;
                for (int j = po.startStationIndex; j < po.endStationIndex; ++j) if (sd.seats[j] < min_s) min_s = sd.seats[j];
                if (min_s >= po.num) {
                    for (int j = po.startStationIndex; j < po.endStationIndex; ++j) sd.seats[j] -= po.num;
                    po.status = SUCCESS;
                    orders.update({FixedString<21>(po.username), -po.timestamp}, po);
                    pending_orders.remove(pit.key());
                    // After removal, we should probably re-get the iterator or be careful.
                    // Since we are removing the current element, let's restart the search for simplicity
                    // or just continue if the BPTree iterator handles it (it doesn't).
                    pit = pending_orders.lower_bound({FixedString<21>(o.trainID), o.date, po.timestamp});
                } else {
                    pit.next();
                }
            }
            seats.update({FixedString<21>(o.trainID), o.date}, sd);
        }
        std::cout << "0" << std::endl;
    }

    void clean() {
        users.clear(); trains.clear(); station_trains.clear(); orders.clear(); seats.clear(); pending_orders.clear();
        timestamp_counter = 0;
        std::cout << "0" << std::endl;
    }
};

} // namespace sjtu

#endif

