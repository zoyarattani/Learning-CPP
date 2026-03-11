#include <iostream>
#include <fstream>
#include <string>
#include <cctype>
#include <cstdlib>
#include <cmath>
#include <cassert>

using namespace std;

bool is_digit_ch(char c) { return c >= '0' && c <= '9'; }
//checks if character is decimal digit

string to_lower_copy(const string& s) {
    //creates a lowercase version of a string without modifying the original
    string r;    // removed reserve(s.size())
    for (size_t i = 0; i < s.size(); i++) {
        char c = s[i];
        if (c >= 'A' && c <= 'Z') c = char(c - 'A' + 'a');
        r.push_back(c);
    }
    return r;
}


void trim_spaces_inplace(string& s) {
    //removes leading & trailing spaces/tabs (simplified with string methods)
    const char* ws = " \t";
    size_t b = s.find_first_not_of(ws);
    if (b == string::npos) { s.clear(); return; }
    size_t e = s.find_last_not_of(ws);
    s.erase(e + 1);
    s.erase(0, b);
}

// removed slice_copy(...) — std::string::substr already does this

struct Place {
    //one record per line
    int code;
    string state_abbr;
    string name;
    int population;
    double area;
    double latitude;
    double longitude;
    int inter_code;
    double inter_dist;
};





struct Node {
    //singly linked list node for buckets 
    Place val;
    Node* next;
    Node(const Place& p, Node* n) : val(p), next(n) {}
};

struct StateRow {
    string abbr;
    string fullname;
};

struct States {
    StateRow* a = nullptr;
    int sz = 0, cap = 0;
    ~States() { delete[] a; }
    void ensure(int need) {
        if (need <= cap) return;
        int ncap = cap ? cap * 2 : 16;
        while (ncap < need) ncap *= 2;
        StateRow* n = new StateRow[ncap];
        for (int i = 0; i < sz; ++i) n[i] = a[i];
        delete[] a; a = n; cap = ncap;
    }
    void push(const string& ab, const string& full) {
        ensure(sz + 1);
        a[sz].abbr = ab; a[sz].fullname = full; ++sz;
    }
    const char* full_of(const string& ab) const {
        for (int i = 0; i < sz; ++i) if (a[i].abbr == ab) return a[i].fullname.c_str();
        return NULL; //return NULL for unknown
    }
} states;

void load_states(const string& path) {
    //reads the file given
    ifstream in(path);  // no .c_str()
    if (!in) {
        cout << "Warning: the states list can't be opened " << path << "\n";
        return;
    }
    string ab, rest;
    while (true) {
        if (!(in >> ab)) break;
        getline(in, rest);
        if (!rest.empty() && rest[0] == ' ') rest.erase(0, 1);
        // warn on duplicate & overwrite
        bool dup = false;
        for (int i = 0; i < states.sz; ++i) if (states.a[i].abbr == ab) {
            cerr << "Warning: duplicate state " << ab << ", overwriting.\n";
            states.a[i].fullname = rest; dup = true; break;
        }
        if (!dup) states.push(ab, rest);
    }
}

const char* full_state_name(const string& abbr) {
    //map abbreviations to full name; return NULL if unknown 
    return states.full_of(abbr);
}

class HashTable {
    //encapsulates the buckets array and operations
public:
    HashTable() : buckets(0), cap(0), sz(0) { init(1024); }
    ~HashTable() { clear(); if (buckets) delete[] buckets; }

    void insert(const Place& p) {
        //inserts a place into the table
        if ((sz + 1) * 4 > cap * 3) rehash(cap * 2 + 1);
        unsigned long h = hash_key(to_lower_copy(p.name));
        unsigned long i = h % cap;
        buckets[i] = new Node(p, buckets[i]);
        sz++;
    }

    void find_by_name(const string& name, void (*visit)(const Place&)) const {
        //iterates all entries that exactly match a given name and calls a callback for each
        string key = to_lower_copy(name);
        unsigned long h = hash_key(key);
        unsigned long i = h % cap;
        Node* cur = buckets[i];
        while (cur != 0) {
            if (equals_ignore_case(cur->val.name, name)) visit(cur->val);
            cur = cur->next;
        }
    }



    bool find_by_name_state(const string& name, const string& state, Place& out) const {
        //finds one specific record matching both name and state abbreviation
        string key = to_lower_copy(name);
        unsigned long h = hash_key(key);
        unsigned long i = h % cap;
        Node* cur = buckets[i];
        while (cur != 0) {
            if (equals_ignore_case(cur->val.name, name) && cur->val.state_abbr == state) {
                out = cur->val;
                return true;
            }
            cur = cur->next;
        }
        return false;
    }

private:
    Node** buckets;
    unsigned long cap;
    unsigned long sz;

    bool equals_ignore_case(const string& a, const string& b) const {
        //isn't case sensitive for names
        if (a.size() != b.size()) return false;
        for (size_t i = 0; i < a.size(); i++) {
            char ca = a[i], cb = b[i];
            if (ca >= 'A' && ca <= 'Z') ca = char(ca - 'A' + 'a');
            if (cb >= 'A' && cb <= 'Z') cb = char(cb - 'A' + 'a');
            if (ca != cb) return false;
        }
        return true;
    }

    void init(unsigned long c) {
        //creates buckets of capacity c and initializes them to null
        cap = c; sz = 0;
        buckets = new Node * [cap];
        for (unsigned long i = 0; i < cap; i++) buckets[i] = 0;
    }

    void clear() {
        //deletes all nodes in all buckets, sets size to 0
        if (!buckets) return;
        for (unsigned long i = 0; i < cap; i++) {
            Node* cur = buckets[i];
            while (cur != 0) {
                Node* nxt = cur->next;
                delete cur;
                cur = nxt;
            }
            buckets[i] = 0;
        }
        sz = 0;
    }






    void rehash(unsigned long newcap) {
        //resizes the bucket array and rehashes all current nodes into it
        Node** old = buckets;
        unsigned long oldcap = cap;

        buckets = new Node * [newcap];
        for (unsigned long i = 0; i < newcap; i++) buckets[i] = 0;

        cap = newcap;
        for (unsigned long i = 0; i < oldcap; i++) {
            Node* cur = old[i];
            while (cur != 0) {
                Node* nxt = cur->next;
                unsigned long h = hash_key(to_lower_copy(cur->val.name));
                unsigned long idx = h % cap;
                cur->next = buckets[idx];
                buckets[idx] = cur;
                cur = nxt;
            }
        }
        delete[] old;
    }

    int hash_key(const string& s) const {
        //computes a numeric hash code for a string (cleaner types + digit separators)
        unsigned long long h = 0;
        constexpr unsigned long long P = 31ULL;
        constexpr unsigned long long M = 1000000007;
        for (unsigned char c : s) {
            h = (h * P + c) % M;
        }
        return int(h);
    }
};

long parse_int_from(const string& s, int& idx) {
    //reads a signed integer at s[idx], skipping spaces and advancing idx
    while (idx < (int)s.size() && s[idx] == ' ') idx++;
    size_t pos = (size_t)idx, nread = 0;
    try {
        long v = std::stol(s.substr(pos), &nread, 10);
        if (nread == 0) return 0;
        idx += (int)nread;
        return v;
    }
    catch (...) {
        return 0;
    }
}

double parse_double_from(const string& s, int& idx) {
    //parses a floating-point number from s[idx], then advances idx
    //handles touching fields because stod reads as far as it can
    while (idx < (int)s.size() && s[idx] == ' ') idx++;
    size_t pos = (size_t)idx, nread = 0;
    try {
        double v = std::stod(s.substr(pos), &nread);
        if (nread == 0) return 0.0;
        idx += (int)nread;
        return v;
    }
    catch (...) {
        return 0.0;
    }
}

bool parse_place_line(const string& line, Place& out) {
    //parse one complete line from the file into a place
    if (line.size() < 20) return false;

    int code = 0;
    for (int i = 0; i < 8; i++) {
        if (!is_digit_ch(line[i])) return false;
        code = code * 10 + (line[i] - '0');
    }
    out.code = code;

    string st;
    st.push_back(line[8]);
    st.push_back(line[9]);
    out.state_abbr = st;

    int k = 10;
    int name_start = k;
    while (k < (int)line.size() && !is_digit_ch(line[k])) k++;

    string name = line.substr(name_start, k - name_start);
    trim_spaces_inplace(name);
    out.name = name;

    int idx = k;
    long pop = parse_int_from(line, idx);
    out.population = (int)pop;

    double area = parse_double_from(line, idx);
    out.area = area;

    double lat = parse_double_from(line, idx);
    out.latitude = lat;

    double lon = parse_double_from(line, idx);
    out.longitude = lon;

    long inter = parse_int_from(line, idx);
    out.inter_code = (int)inter;

    double dist = parse_double_from(line, idx);
    out.inter_dist = dist;

    return true;
}

void print_place(const Place& p) {
    //outputs all Place attributes, using full_state_name to expand the state abbreviation
    const char* full = full_state_name(p.state_abbr);
    cout << "Code: " << p.code << "\n";
    cout << "State: " << p.state_abbr << " (" << (full ? full : "unknown") << ")\n";
    cout << "Name: " << p.name << "\n";
    cout << "Population: " << p.population << "\n";
    cout << "Area: " << p.area << "\n";
    cout << "Latitude: " << p.latitude << "\n";
    cout << "Longitude: " << p.longitude << "\n";
    cout << "Nearest intersection code: " << p.inter_code << "\n";
    cout << "Distance to intersection: " << p.inter_dist << "\n";
}

void visit_collect_states(const Place& p) {
    //prints state abbreviation and full name for each match (used with find_by_name)
    const char* full = full_state_name(p.state_abbr);
    cout << p.state_abbr << "  " << (full ? full : "unknown") << "\n";
}

int main() {
    load_states("/home/www/class/een318/states.txt");
    HashTable ht;
    ifstream in("/home/www/class/een318/named-places.txt");
    if (!in) {
        cout << "Error: cannot open /home/www/class/een318/named-places.txt\n";
        return 1;
    }
    string line;
    long count = 0;
    while (true) {
        if (!getline(in, line)) break;
        if (line.size() == 0) continue;
        Place p;
        if (parse_place_line(line, p)) {
            ht.insert(p);
            count++;
        }
        else {
            cerr << "Warning: could not parse line: \"" << line << "\"\n";
        }
    }
    cout << "Loaded places: " << count << "\n";
    while (true) {
        cout << "\nEnter command (N name | S name state | Q): ";
        string cmd;
        if (!(cin >> cmd)) break;

        if (cmd == "Q" || cmd == "q") break;

        if (cmd == "N" || cmd == "n") {
            string rest;
            getline(cin, rest);
            if (!rest.empty() && rest[0] == ' ') rest.erase(0, 1);
            trim_spaces_inplace(rest);
            if (rest.size() == 0) {
                cout << "Please provide a place name.\n";
                continue;
            }
            ht.find_by_name(rest, visit_collect_states);
        }
        else if (cmd == "S" || cmd == "s") {
            string rest;
            getline(cin, rest);
            if (!rest.empty() && rest[0] == ' ') rest.erase(0, 1);
            trim_spaces_inplace(rest);
            if (rest.size() < 3) {
                cout << "Usage: S placename STATE\n";
                continue;
            }
            int end = (int)rest.size() - 1;
            while (end >= 0 && rest[end] == ' ') end--;
            if (end < 1) {
                cout << "Usage: S placename STATE\n";
                continue;
            }

            int start_state = end;
            while (start_state >= 0 && rest[start_state] != ' ') start_state--;
            string state;
            for (int t = start_state + 1; t <= end; t++) state.push_back(rest[t]);
            string nm;
            for (int t = 0; t <= start_state - 1; t++) nm.push_back(rest[t]);
            trim_spaces_inplace(nm);
            trim_spaces_inplace(state);
            if (state.size() == 2) {
                if (state[0] >= 'a' && state[0] <= 'z') state[0] = char(state[0] - 'a' + 'A');
                if (state[1] >= 'a' && state[1] <= 'z') state[1] = char(state[1] - 'a' + 'A');
            }

            if (state.size() != 2) {
                cout << "State abbreviation must be two letters.\n";
                continue;
            }

            Place found;
            if (ht.find_by_name_state(nm, state, found)) {
                print_place(found);
            }
            else {
                cout << "Not found: \"" << nm << "\" in " << state << "\n";
            }
        }
        else {
            cout << "Unknown command. Try N, S, or Q.\n";
            string junk; getline(cin, junk);
        }
    }

    return 0;
}



