#include <iostream>
#include <cstring>
#include <cstdio>
#include <string>
#include <cstdlib>

using std::string;
using std::cin;
using std::cout;
using std::endl;

// Simple vector implementation
template<typename T>
class Vector {
private:
    T* data;
    int cap;
    int sz;

    void resize() {
        cap = cap * 2 + 1;
        T* newData = new T[cap];
        for (int i = 0; i < sz; i++) {
            newData[i] = data[i];
        }
        delete[] data;
        data = newData;
    }

public:
    Vector() : data(nullptr), cap(0), sz(0) {}

    ~Vector() {
        delete[] data;
    }

    void push_back(const T& val) {
        if (sz >= cap) resize();
        data[sz++] = val;
    }

    int size() const { return sz; }

    T& operator[](int idx) { return data[idx]; }
    const T& operator[](int idx) const { return data[idx]; }

    void clear() {
        sz = 0;
    }
};

// Simple hash map
template<typename K, typename V, int SIZE = 10007>
class HashMap {
private:
    struct Node {
        K key;
        V value;
        Node* next;
        Node(const K& k, const V& v) : key(k), value(v), next(nullptr) {}
    };
    Node* buckets[SIZE];

    int hash(const string& key) const {
        unsigned int h = 0;
        for (size_t i = 0; i < key.length(); i++) {
            h = h * 131 + key[i];
        }
        return h % SIZE;
    }

public:
    HashMap() {
        for (int i = 0; i < SIZE; i++) buckets[i] = nullptr;
    }

    ~HashMap() {
        clear();
    }

    void insert(const K& key, const V& value) {
        int idx = hash(key);
        Node* p = buckets[idx];
        while (p) {
            if (p->key == key) {
                p->value = value;
                return;
            }
            p = p->next;
        }
        Node* newNode = new Node(key, value);
        newNode->next = buckets[idx];
        buckets[idx] = newNode;
    }

    bool find(const K& key, V& value) const {
        int idx = hash(key);
        Node* p = buckets[idx];
        while (p) {
            if (p->key == key) {
                value = p->value;
                return true;
            }
            p = p->next;
        }
        return false;
    }

    bool erase(const K& key) {
        int idx = hash(key);
        Node* p = buckets[idx];
        Node* prev = nullptr;
        while (p) {
            if (p->key == key) {
                if (prev) prev->next = p->next;
                else buckets[idx] = p->next;
                delete p;
                return true;
            }
            prev = p;
            p = p->next;
        }
        return false;
    }

    void clear() {
        for (int i = 0; i < SIZE; i++) {
            Node* p = buckets[i];
            while (p) {
                Node* tmp = p;
                p = p->next;
                delete tmp;
            }
            buckets[i] = nullptr;
        }
    }
};

// User structure
struct User {
    char username[25];
    char password[35];
    char name[35];
    char mailAddr[35];
    int privilege;

    User() {
        username[0] = password[0] = name[0] = mailAddr[0] = 0;
        privilege = 0;
    }
};

// Train structure
struct Train {
    char trainID[25];
    int stationNum;
    char stations[100][35];
    int seatNum;
    int prices[100];        // cumulative prices
    int startTime;          // minutes from 00:00
    int travelTimes[100];
    int stopoverTimes[100];
    int saleDate[2];        // days from 06-01
    char type;
    bool released;
    int seats[93][100];     // seats[day][station] - remaining seats from this station

    Train() {
        trainID[0] = 0;
        stationNum = 0;
        seatNum = 0;
        startTime = 0;
        type = 0;
        released = false;
        for (int i = 0; i < 93; i++) {
            for (int j = 0; j < 100; j++) {
                seats[i][j] = 0;
            }
        }
    }
};

// Order structure
struct Order {
    char trainID[25];
    char fromStation[35];
    char toStation[35];
    int date;              // day from 06-01
    int fromIdx, toIdx;
    int num;
    int price;
    int status;            // 0=success, 1=pending, 2=refunded
    int timestamp;

    Order() {
        trainID[0] = fromStation[0] = toStation[0] = 0;
        date = fromIdx = toIdx = num = price = status = timestamp = 0;
    }
};

HashMap<string, User> users;
HashMap<string, bool> loggedInUsers;
HashMap<string, Train> trains;
HashMap<string, Vector<Order>*> orders;
int userCount = 0;
int orderTimestamp = 0;

// Parse time string "HH:MM" to minutes
int parseTime(const string& timeStr) {
    int h = (timeStr[0] - '0') * 10 + (timeStr[1] - '0');
    int m = (timeStr[3] - '0') * 10 + (timeStr[4] - '0');
    return h * 60 + m;
}

// Parse date string "MM-DD" to days from 06-01
int parseDate(const string& dateStr) {
    int m = (dateStr[0] - '0') * 10 + (dateStr[1] - '0');
    int d = (dateStr[3] - '0') * 10 + (dateStr[4] - '0');
    int days[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int result = 0;
    for (int i = 6; i < m; i++) {
        result += days[i];
    }
    result += d;
    return result;
}

// Format time from day offset and minutes
string formatTime(int dayOffset, int minutes) {
    int days[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int m = 6, d = 1;
    d += dayOffset;
    
    int h = minutes / 60;
    int min = minutes % 60;
    while (h >= 24) {
        h -= 24;
        d++;
    }
    
    while (d > days[m]) {
        d -= days[m];
        m++;
    }

    char buf[20];
    sprintf(buf, "%02d-%02d %02d:%02d", m, d, h, min);
    return string(buf);
}

// Split string by delimiter
Vector<string> split(const string& str, char delim) {
    Vector<string> result;
    size_t start = 0;
    for (size_t i = 0; i <= str.length(); i++) {
        if (i == str.length() || str[i] == delim) {
            if (i > start) {
                result.push_back(str.substr(start, i - start));
            }
            start = i + 1;
        }
    }
    return result;
}

// Parse command parameters - generic version
HashMap<string, string, 31> parseParams(const string& params) {
    HashMap<string, string, 31> result;
    size_t pos = 0;
    while (pos < params.length()) {
        while (pos < params.length() && params[pos] == ' ') pos++;
        if (pos >= params.length() || params[pos] != '-') break;

        string key(1, params[pos + 1]);
        pos += 3;  // skip "-x "

        size_t end = params.find(" -", pos);
        if (end == string::npos) end = params.length();
        string value = params.substr(pos, end - pos);

        result.insert(key, value);
        pos = end;
    }
    return result;
}

void addUser(const string& params) {
    auto p = parseParams(params);
    string c, u, pw, n, m, g;
    p.find("c", c); p.find("u", u); p.find("p", pw);
    p.find("n", n); p.find("m", m); p.find("g", g);

    User existing;
    if (users.find(u, existing)) {
        cout << "-1" << endl;
        return;
    }

    if (userCount > 0) {
        bool isLoggedIn = false;
        if (!loggedInUsers.find(c, isLoggedIn) || !isLoggedIn) {
            cout << "-1" << endl;
            return;
        }

        User curUser;
        if (!users.find(c, curUser)) {
            cout << "-1" << endl;
            return;
        }

        int newPriv = std::stoi(g);
        if (newPriv >= curUser.privilege) {
            cout << "-1" << endl;
            return;
        }
    }

    User newUser;
    strcpy(newUser.username, u.c_str());
    strcpy(newUser.password, pw.c_str());
    strcpy(newUser.name, n.c_str());
    strcpy(newUser.mailAddr, m.c_str());
    newUser.privilege = (userCount == 0) ? 10 : std::stoi(g);

    users.insert(u, newUser);
    userCount++;
    cout << "0" << endl;
}

void login(const string& params) {
    auto p = parseParams(params);
    string u, pw;
    p.find("u", u); p.find("p", pw);

    User user;
    if (!users.find(u, user)) {
        cout << "-1" << endl;
        return;
    }

    if (strcmp(user.password, pw.c_str()) != 0) {
        cout << "-1" << endl;
        return;
    }

    bool isLoggedIn = false;
    if (loggedInUsers.find(u, isLoggedIn) && isLoggedIn) {
        cout << "-1" << endl;
        return;
    }

    loggedInUsers.insert(u, true);
    cout << "0" << endl;
}

void logout(const string& params) {
    auto p = parseParams(params);
    string u;
    p.find("u", u);

    bool isLoggedIn = false;
    if (!loggedInUsers.find(u, isLoggedIn) || !isLoggedIn) {
        cout << "-1" << endl;
        return;
    }

    loggedInUsers.insert(u, false);
    cout << "0" << endl;
}

void queryProfile(const string& params) {
    auto p = parseParams(params);
    string c, u;
    p.find("c", c); p.find("u", u);

    bool isLoggedIn = false;
    if (!loggedInUsers.find(c, isLoggedIn) || !isLoggedIn) {
        cout << "-1" << endl;
        return;
    }

    User curUser, targetUser;
    if (!users.find(c, curUser) || !users.find(u, targetUser)) {
        cout << "-1" << endl;
        return;
    }

    if (curUser.privilege <= targetUser.privilege && c != u) {
        cout << "-1" << endl;
        return;
    }

    cout << targetUser.username << " " << targetUser.name << " "
         << targetUser.mailAddr << " " << targetUser.privilege << endl;
}

void modifyProfile(const string& params) {
    auto p = parseParams(params);
    string c, u, pw, n, m, g;
    p.find("c", c); p.find("u", u); p.find("p", pw);
    p.find("n", n); p.find("m", m); p.find("g", g);

    bool isLoggedIn = false;
    if (!loggedInUsers.find(c, isLoggedIn) || !isLoggedIn) {
        cout << "-1" << endl;
        return;
    }

    User curUser, targetUser;
    if (!users.find(c, curUser) || !users.find(u, targetUser)) {
        cout << "-1" << endl;
        return;
    }

    if (curUser.privilege <= targetUser.privilege && c != u) {
        cout << "-1" << endl;
        return;
    }

    if (!g.empty()) {
        int newPriv = std::stoi(g);
        if (newPriv >= curUser.privilege) {
            cout << "-1" << endl;
            return;
        }
        targetUser.privilege = newPriv;
    }

    if (!pw.empty()) strcpy(targetUser.password, pw.c_str());
    if (!n.empty()) strcpy(targetUser.name, n.c_str());
    if (!m.empty()) strcpy(targetUser.mailAddr, m.c_str());

    users.insert(u, targetUser);
    cout << targetUser.username << " " << targetUser.name << " "
         << targetUser.mailAddr << " " << targetUser.privilege << endl;
}

void addTrain(const string& params) {
    auto p = parseParams(params);
    string i, n, m, s, pr, x, t, o, d, y;
    p.find("i", i); p.find("n", n); p.find("m", m);
    p.find("s", s); p.find("p", pr); p.find("x", x);
    p.find("t", t); p.find("o", o); p.find("d", d); p.find("y", y);

    Train existing;
    if (trains.find(i, existing)) {
        cout << "-1" << endl;
        return;
    }

    Train train;
    strcpy(train.trainID, i.c_str());
    train.stationNum = std::stoi(n);
    train.seatNum = std::stoi(m);
    train.startTime = parseTime(x);
    train.type = y[0];
    train.released = false;

    // Parse stations
    auto stations = split(s, '|');
    for (int i = 0; i < train.stationNum; i++) {
        strcpy(train.stations[i], stations[i].c_str());
    }

    // Parse prices (cumulative)
    auto prices = split(pr, '|');
    train.prices[0] = 0;
    for (int i = 0; i < prices.size(); i++) {
        train.prices[i + 1] = train.prices[i] + std::stoi(prices[i]);
    }

    // Parse travel times
    auto travelTimes = split(t, '|');
    for (int i = 0; i < travelTimes.size(); i++) {
        train.travelTimes[i] = std::stoi(travelTimes[i]);
    }

    // Parse stopover times
    auto stopoverTimes = split(o, '|');
    for (int i = 0; i < stopoverTimes.size(); i++) {
        train.stopoverTimes[i] = std::stoi(stopoverTimes[i]);
    }

    // Parse sale dates
    auto dates = split(d, '|');
    train.saleDate[0] = parseDate(dates[0]);
    train.saleDate[1] = parseDate(dates[1]);

    // Initialize seats
    for (int day = 0; day < 93; day++) {
        for (int sta = 0; sta < train.stationNum; sta++) {
            train.seats[day][sta] = train.seatNum;
        }
    }

    trains.insert(i, train);
    cout << "0" << endl;
}

void releaseTrain(const string& params) {
    auto p = parseParams(params);
    string i;
    p.find("i", i);

    Train train;
    if (!trains.find(i, train)) {
        cout << "-1" << endl;
        return;
    }

    if (train.released) {
        cout << "-1" << endl;
        return;
    }

    train.released = true;
    trains.insert(i, train);
    cout << "0" << endl;
}

void deleteTrain(const string& params) {
    auto p = parseParams(params);
    string i;
    p.find("i", i);

    Train train;
    if (!trains.find(i, train)) {
        cout << "-1" << endl;
        return;
    }

    if (train.released) {
        cout << "-1" << endl;
        return;
    }

    trains.erase(i);
    cout << "0" << endl;
}

void queryTrain(const string& params) {
    auto p = parseParams(params);
    string i, d;
    p.find("i", i); p.find("d", d);

    Train train;
    if (!trains.find(i, train)) {
        cout << "-1" << endl;
        return;
    }

    int queryDate = parseDate(d);
    int startDay = queryDate;

    cout << train.trainID << " " << train.type << endl;

    int currentTime = train.startTime;
    int dayOffset = 0;

    for (int idx = 0; idx < train.stationNum; idx++) {
        // Arrival time
        if (idx == 0) {
            cout << train.stations[idx] << " xx-xx xx:xx -> ";
        } else {
            cout << train.stations[idx] << " " << formatTime(startDay - 1 + dayOffset, currentTime) << " -> ";
        }

        // Add stopover time
        if (idx > 0 && idx < train.stationNum - 1) {
            currentTime += train.stopoverTimes[idx - 1];
        }

        // Leaving time
        if (idx == train.stationNum - 1) {
            cout << "xx-xx xx:xx ";
        } else {
            cout << formatTime(startDay - 1 + dayOffset, currentTime) << " ";
        }

        // Price
        cout << train.prices[idx] << " ";

        // Seats
        if (idx == train.stationNum - 1) {
            cout << "x" << endl;
        } else {
            // Calculate minimum seats from idx to any later station
            int minSeats = train.seatNum;
            if (train.released && startDay >= 0 && startDay < 93) {
                for (int j = idx; j < train.stationNum - 1; j++) {
                    if (train.seats[startDay][j] < minSeats) {
                        minSeats = train.seats[startDay][j];
                    }
                }
            }
            cout << minSeats << endl;
        }

        // Add travel time to next station
        if (idx < train.stationNum - 1) {
            currentTime += train.travelTimes[idx];
            while (currentTime >= 24 * 60) {
                currentTime -= 24 * 60;
                dayOffset++;
            }
        }
    }
}

void clean() {
    users.clear();
    loggedInUsers.clear();
    trains.clear();
    orders.clear();
    userCount = 0;
    orderTimestamp = 0;
    cout << "0" << endl;
}

int main() {
    std::ios::sync_with_stdio(false);
    cin.tie(0);

    string line;
    while (getline(cin, line)) {
        if (line.empty()) continue;

        size_t cmdEnd = line.find(' ');
        string cmd = (cmdEnd == string::npos) ? line : line.substr(0, cmdEnd);
        string params = (cmdEnd == string::npos) ? "" : line.substr(cmdEnd + 1);

        if (cmd == "add_user") {
            addUser(params);
        } else if (cmd == "login") {
            login(params);
        } else if (cmd == "logout") {
            logout(params);
        } else if (cmd == "query_profile") {
            queryProfile(params);
        } else if (cmd == "modify_profile") {
            modifyProfile(params);
        } else if (cmd == "add_train") {
            addTrain(params);
        } else if (cmd == "release_train") {
            releaseTrain(params);
        } else if (cmd == "delete_train") {
            deleteTrain(params);
        } else if (cmd == "query_train") {
            queryTrain(params);
        } else if (cmd == "query_ticket") {
            cout << "0" << endl;  // TODO: implement
        } else if (cmd == "query_transfer") {
            cout << "0" << endl;  // TODO: implement
        } else if (cmd == "buy_ticket") {
            cout << "-1" << endl;  // TODO: implement
        } else if (cmd == "query_order") {
            cout << "-1" << endl;  // TODO: implement
        } else if (cmd == "refund_ticket") {
            cout << "-1" << endl;  // TODO: implement
        } else if (cmd == "clean") {
            clean();
        } else if (cmd == "exit") {
            cout << "bye" << endl;
            break;
        } else {
            cout << "-1" << endl;
        }
    }

    return 0;
}
