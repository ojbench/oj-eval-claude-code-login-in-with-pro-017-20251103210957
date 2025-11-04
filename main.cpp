#include <iostream>
#include <cstring>
#include <cstdio>
#include <string>
#include <cstdlib>

using std::string;
using std::cin;
using std::cout;
using std::endl;

const int MAXN = 100005;

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

    void sort() {
        // Simple bubble sort for small arrays
        for (int i = 0; i < sz - 1; i++) {
            for (int j = 0; j < sz - 1 - i; j++) {
                if (data[j + 1] < data[j]) {
                    T tmp = data[j];
                    data[j] = data[j + 1];
                    data[j + 1] = tmp;
                }
            }
        }
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

HashMap<string, User> users;
HashMap<string, bool> loggedInUsers;
int userCount = 0;

// Parse command parameters
void parseParams(const string& params, string& c, string& u, string& p, string& n, string& m, string& g) {
    size_t pos = 0;
    while (pos < params.length()) {
        while (pos < params.length() && params[pos] == ' ') pos++;
        if (pos >= params.length() || params[pos] != '-') break;

        char key = params[pos + 1];
        pos += 3;  // skip "-x "

        size_t end = params.find(" -", pos);
        if (end == string::npos) end = params.length();
        string value = params.substr(pos, end - pos);

        if (key == 'c') c = value;
        else if (key == 'u') u = value;
        else if (key == 'p') p = value;
        else if (key == 'n') n = value;
        else if (key == 'm') m = value;
        else if (key == 'g') g = value;

        pos = end;
    }
}

void addUser(const string& params) {
    string c, u, p, n, m, g;
    parseParams(params, c, u, p, n, m, g);

    User existing;
    if (users.find(u, existing)) {
        cout << "-1" << endl;
        return;
    }

    if (userCount > 0) {
        // Not first user - check permissions
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
    strcpy(newUser.password, p.c_str());
    strcpy(newUser.name, n.c_str());
    strcpy(newUser.mailAddr, m.c_str());
    newUser.privilege = (userCount == 0) ? 10 : std::stoi(g);

    users.insert(u, newUser);
    userCount++;
    cout << "0" << endl;
}

void login(const string& params) {
    string c, u, p, n, m, g;
    parseParams(params, c, u, p, n, m, g);

    User user;
    if (!users.find(u, user)) {
        cout << "-1" << endl;
        return;
    }

    if (strcmp(user.password, p.c_str()) != 0) {
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
    string c, u, p, n, m, g;
    parseParams(params, c, u, p, n, m, g);

    bool isLoggedIn = false;
    if (!loggedInUsers.find(u, isLoggedIn) || !isLoggedIn) {
        cout << "-1" << endl;
        return;
    }

    loggedInUsers.insert(u, false);
    cout << "0" << endl;
}

void queryProfile(const string& params) {
    string c, u, p, n, m, g;
    parseParams(params, c, u, p, n, m, g);

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
    string c, u, p, n, m, g;
    parseParams(params, c, u, p, n, m, g);

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

    if (!p.empty()) strcpy(targetUser.password, p.c_str());
    if (!n.empty()) strcpy(targetUser.name, n.c_str());
    if (!m.empty()) strcpy(targetUser.mailAddr, m.c_str());

    users.insert(u, targetUser);
    cout << targetUser.username << " " << targetUser.name << " "
         << targetUser.mailAddr << " " << targetUser.privilege << endl;
}

void clean() {
    users.clear();
    loggedInUsers.clear();
    userCount = 0;
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
        } else if (cmd == "clean") {
            clean();
        } else if (cmd == "exit") {
            cout << "bye" << endl;
            break;
        } else {
            // Unimplemented commands - return -1
            cout << "-1" << endl;
        }
    }

    return 0;
}
