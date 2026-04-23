


#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include "system.hpp"

using namespace sjtu;

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    System sys;
    std::string line;
    while (std::getline(std::cin, line)) {
        if (line.empty()) continue;
        std::stringstream ss(line);
        std::string cmd_with_timestamp;
        ss >> cmd_with_timestamp;
        std::cout << cmd_with_timestamp << " ";
        std::string cmd;
        ss >> cmd;

        if (cmd == "add_user") {
            char cur_u[21] = "", u[21] = "", p[31] = "", n[21] = "", m[31] = "";
            int g = -1;
            std::string key, val;
            while (ss >> key >> val) {
                if (key == "-c") strcpy(cur_u, val.c_str());
                else if (key == "-u") strcpy(u, val.c_str());
                else if (key == "-p") strcpy(p, val.c_str());
                else if (key == "-n") strcpy(n, val.c_str());
                else if (key == "-m") strcpy(m, val.c_str());
                else if (key == "-g") g = std::stoi(val);
            }
            sys.add_user(cur_u, u, p, n, m, g);
        } else if (cmd == "login") {
            char u[21] = "", p[31] = "";
            std::string key, val;
            while (ss >> key >> val) {
                if (key == "-u") strcpy(u, val.c_str());
                else if (key == "-p") strcpy(p, val.c_str());
            }
            sys.login(u, p);
        } else if (cmd == "logout") {
            char u[21] = "";
            std::string key, val;
            while (ss >> key >> val) {
                if (key == "-u") strcpy(u, val.c_str());
            }
            sys.logout(u);
        } else if (cmd == "query_profile") {
            char c[21] = "", u[21] = "";
            std::string key, val;
            while (ss >> key >> val) {
                if (key == "-c") strcpy(c, val.c_str());
                else if (key == "-u") strcpy(u, val.c_str());
            }
            sys.query_profile(c, u);
        } else if (cmd == "modify_profile") {
            char c[21] = "", u[21] = "", p[31] = "", n[21] = "", m[31] = "";
            char *pp = nullptr, *pn = nullptr, *pm = nullptr;
            int g = -1;
            std::string key, val;
            while (ss >> key >> val) {
                if (key == "-c") strcpy(c, val.c_str());
                else if (key == "-u") strcpy(u, val.c_str());
                else if (key == "-p") { strcpy(p, val.c_str()); pp = p; }
                else if (key == "-n") { strcpy(n, val.c_str()); pn = n; }
                else if (key == "-m") { strcpy(m, val.c_str()); pm = m; }
                else if (key == "-g") g = std::stoi(val);
            }
            sys.modify_profile(c, u, pp, pn, pm, g);
        } else if (cmd == "add_train") {
            Train t;
            std::string key, val;
            while (ss >> key >> val) {
                if (key == "-i") strcpy(t.trainID, val.c_str());
                else if (key == "-n") t.stationNum = std::stoi(val);
                else if (key == "-m") t.seatNum = std::stoi(val);
                else if (key == "-s") {
                    std::stringstream sss(val);
                    std::string s;
                    int idx = 0;
                    while (std::getline(sss, s, '|')) strcpy(t.stations[idx++], s.c_str());
                } else if (key == "-p") {
                    std::stringstream sss(val);
                    std::string s;
                    int idx = 0;
                    while (std::getline(sss, s, '|')) t.prices[idx++] = std::stoi(s);
                } else if (key == "-x") {
                    t.startTime = std::stoi(val.substr(0, 2)) * 60 + std::stoi(val.substr(3, 2));
                } else if (key == "-t") {
                    std::stringstream sss(val);
                    std::string s;
                    int idx = 0;
                    while (std::getline(sss, s, '|')) t.travelTimes[idx++] = std::stoi(s);
                } else if (key == "-o") {
                    if (val != "_") {
                        std::stringstream sss(val);
                        std::string s;
                        int idx = 0;
                        while (std::getline(sss, s, '|')) t.stopoverTimes[idx++] = std::stoi(s);
                    }
                } else if (key == "-d") {
                    t.saleDateStart = dateToDays(std::stoi(val.substr(0, 2)), std::stoi(val.substr(3, 2)));
                    t.saleDateEnd = dateToDays(std::stoi(val.substr(6, 2)), std::stoi(val.substr(9, 2)));
                } else if (key == "-y") t.type = val[0];
            }
            sys.add_train(t);
        } else if (cmd == "release_train") {
            char i[21] = "";
            std::string key, val;
            while (ss >> key >> val) {
                if (key == "-i") strcpy(i, val.c_str());
            }
            sys.release_train(i);
        } else if (cmd == "query_train") {
            char i[21] = "";
            int d = -1;
            std::string key, val;
            while (ss >> key >> val) {
                if (key == "-i") strcpy(i, val.c_str());
                else if (key == "-d") d = dateToDays(std::stoi(val.substr(0, 2)), std::stoi(val.substr(3, 2)));
            }
            sys.query_train(i, d);
        } else if (cmd == "delete_train") {
            char i[21] = "";
            std::string key, val;
            while (ss >> key >> val) {
                if (key == "-i") strcpy(i, val.c_str());
            }
            sys.delete_train(i);
        } else if (cmd == "query_ticket") {
            char s[31] = "", t[31] = "";
            int d = -1;
            bool sort_by_time = true;
            std::string key, val;
            while (ss >> key >> val) {
                if (key == "-s") strcpy(s, val.c_str());
                else if (key == "-t") strcpy(t, val.c_str());
                else if (key == "-d") d = dateToDays(std::stoi(val.substr(0, 2)), std::stoi(val.substr(3, 2)));
                else if (key == "-p") sort_by_time = (val == "time");
            }
            sys.query_ticket(s, t, d, sort_by_time);
        } else if (cmd == "query_transfer") {
            char s[31] = "", t[31] = "";
            int d = -1;
            bool sort_by_time = true;
            std::string key, val;
            while (ss >> key >> val) {
                if (key == "-s") strcpy(s, val.c_str());
                else if (key == "-t") strcpy(t, val.c_str());
                else if (key == "-d") d = dateToDays(std::stoi(val.substr(0, 2)), std::stoi(val.substr(3, 2)));
                else if (key == "-p") sort_by_time = (val == "time");
            }
            sys.query_transfer(s, t, d, sort_by_time);
        } else if (cmd == "buy_ticket") {
            char u[21] = "", i[21] = "", f[31] = "", t[31] = "";
            int d = -1, n = -1;
            bool q = false;
            std::string key, val;
            while (ss >> key >> val) {
                if (key == "-u") strcpy(u, val.c_str());
                else if (key == "-i") strcpy(i, val.c_str());
                else if (key == "-d") d = dateToDays(std::stoi(val.substr(0, 2)), std::stoi(val.substr(3, 2)));
                else if (key == "-n") n = std::stoi(val);
                else if (key == "-f") strcpy(f, val.c_str());
                else if (key == "-t") strcpy(t, val.c_str());
                else if (key == "-q") q = (val == "true");
            }
            sys.buy_ticket(u, i, d, n, f, t, q);
        } else if (cmd == "query_order") {
            char u[21] = "";
            std::string key, val;
            while (ss >> key >> val) {
                if (key == "-u") strcpy(u, val.c_str());
            }
            sys.query_order(u);
        } else if (cmd == "refund_ticket") {
            char u[21] = "";
            int n = 1;
            std::string key, val;
            while (ss >> key >> val) {
                if (key == "-u") strcpy(u, val.c_str());
                else if (key == "-n") n = std::stoi(val);
            }
            sys.refund_ticket(u, n);
        } else if (cmd == "clean") {
            sys.clean();
        } else if (cmd == "exit") {
            std::cout << "bye" << std::endl;
            break;
        }
    }

    return 0;
}


