#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <cctype>
#include <cmath>
#include <cassert>
using namespace std;

struct Person {
    string last;
    string first;
    string state;
    string zip;
    int byear;
    int bmonth;
    int bday;
    string password;
    double balance;
    string ssn;
};

struct Node {
    Person p;
    Node* left;
    Node* right;
    Node(const Person& x) : p(x), left(NULL), right(NULL) {}
};

int cmpKeys(const string& la, const string& fa, const string& lb, const string& fb) {
    //compares two people’s keys (last name + first name)
    //returns -1 if (la,fa) < (lb,fb), returns 1 if (la, fa) > (lb, fb), returns 0 if they are equal
    if (la < lb) return -1;
    if (la > lb) return 1;
    if (fa < fb) return -1;
    if (fa > fb) return 1;
    return 0;
}

bool older(const Person& a, const Person& b) {
    //returns true if a is older than b (compares year, then month, then day)
    if (a.byear != b.byear) return a.byear < b.byear;
    if (a.bmonth != b.bmonth) return a.bmonth < b.bmonth;
    return a.bday < b.bday;
}

string twoDigits(int x) {
    //formats integer like day or month as a 2-digit string and also adds a leading 0 if it needs it
    if (x < 0) x = 0;
    if (x < 10) return string("0") + char('0' + x);
    string s;
    s += char('0' + (x / 10) % 10);
    s += char('0' + (x % 10));
    return s;
}

string money2(double v) {
    //converts a floating point-balance to a string with 2 decimal places
    long long sign = 1;
    if (v < 0) { sign = -1; v = -v; }
    long long cents = (long long)floor(v * 100.0 + 0.5);
    long long dollars = cents / 100;
    int c = (int)(cents % 100);
    string s;
    if (sign < 0) s += "-";
    if (dollars == 0) s += "0";
    else {
        string t;
        while (dollars > 0) {
            int d = (int)(dollars % 10);
            t.push_back(char('0' + d));
            dollars /= 10;
        }
        for (int i = (int)t.size() - 1; i >= 0; --i) s.push_back(t[i]);
    }
    s += ".";
    if (c < 10) { s += "0"; s += char('0' + c); }
    else { s += char('0' + (c / 10)); s += char('0' + (c % 10)); }
    return s;
}

void printPersonLine(const Person& p) {
    //prints all the fields of a person record on one line, in the exact format of the database file
    cout << p.last << " " << p.first << " " << p.state << " " << p.zip << " "
        << p.byear << " " << twoDigits(p.bmonth) << " " << twoDigits(p.bday) << " "
        << p.password << " " << money2(p.balance) << " " << p.ssn << "\n";
}

Node* insert(Node* root, const Person& x) {
    //inserts a new person into the bst
    //if the spot is empty, make a new node
    //if the person already exists (same last and first), replace their data
    //otherwise, insert left or right based on cmpkeys
    if (root == NULL) return new Node(x);
    int c = cmpKeys(x.last, x.first, root->p.last, root->p.first);
    if (c < 0) root->left = insert(root->left, x);
    else if (c > 0) root->right = insert(root->right, x);
    else root->p = x;
    return root;
}

Node* findNode(Node* root, const string& first, const string& last) {
    //searches the bst for a specific person by first and last name
    //returns a pointer to the node if found
    //returns null if not found
    while (root != NULL) {
        int c = cmpKeys(last, first, root->p.last, root->p.first);
        if (c < 0) root = root->left;
        else if (c > 0) root = root->right;
        else return root;
    }
    return NULL;
}

void inorderPrint(Node* root) {
    //traverses the bst in sorted order, last name, then first name, and prints every record
    if (root == NULL) return;
    inorderPrint(root->left);
    printPersonLine(root->p);
    inorderPrint(root->right);
}

void familyPrint(Node* root, const string& last) {
    //prints records that have same last name
    if (root == NULL) return;
    if (last < root->p.last) { familyPrint(root->left, last); return; }
    if (last > root->p.last) { familyPrint(root->right, last); return; }
    familyPrint(root->left, last);
    printPersonLine(root->p);
    familyPrint(root->right, last);
}

void firstNamePrint(Node* root, const string& first) {
    //prints all records with a given first name
    if (root == NULL) return;
    firstNamePrint(root->left, first);
    if (root->p.first == first) printPersonLine(root->p);
    firstNamePrint(root->right, first);
}

void oldestScan(Node* root, bool& have, Person& best) {
    //scans the tree to find the oldest person in it
    //updates best if it finds someone older
    if (root == NULL) return;
    oldestScan(root->left, have, best);
    if (!have || older(root->p, best)) { best = root->p; have = true; }
    oldestScan(root->right, have, best);
}

Node* minNode(Node* root) {
    //finds the node with the smallest key
    //aka the leftmost node
    while (root && root->left) root = root->left;
    return root;
}

Node* erase(Node* root, const string& first, const string& last, bool& removed) {
    //deletes a person from th bst when their name is typed
    //handles 3 cases:
    //nodes that have no children just get removed
    //for nodes that only have one child they just link around it
    //for nodes that have two children they replace the inorder successor minNode and remove it
    if (root == NULL) return NULL;
    int c = cmpKeys(last, first, root->p.last, root->p.first);
    if (c < 0) root->left = erase(root->left, first, last, removed);
    else if (c > 0) root->right = erase(root->right, first, last, removed);
    else {
        removed = true;
        if (!root->left) { Node* r = root->right; delete root; return r; }
        if (!root->right) { Node* l = root->left; delete root; return l; }
        Node* succ = minNode(root->right);
        root->p = succ->p;
        root->right = erase(root->right, succ->p.first, succ->p.last, removed);
    }
    return root;
}

int countNodes(Node* root) {
    //counts total number of nodes in tree
    if (!root) return 0;
    return 1 + countNodes(root->left) + countNodes(root->right);
}

void fillInorder(Node* root, Node** arr, int& idx) {
    //fills an array with all nodes from tree in sorted (inorder) order 
    if (!root) return;
    fillInorder(root->left, arr, idx);
    arr[idx++] = root;
    fillInorder(root->right, arr, idx);
}

void saveMedianFirst(Node** arr, int lo, int hi, ofstream& fout) {
    //writes the nodes from an inorder array in median first order
    //makes sure that tree stays balanced 
    if (lo > hi) return;
    int mid = lo + (hi - lo) / 2;
    const Person& p = arr[mid]->p;
    fout << p.last << " " << p.first << " " << p.state << " " << p.zip << " "
        << p.byear << " " << twoDigits(p.bmonth) << " " << twoDigits(p.bday) << " "
        << p.password << " " << money2(p.balance) << " " << p.ssn << "\n";
    saveMedianFirst(arr, lo, mid - 1, fout);
    saveMedianFirst(arr, mid + 1, hi, fout);
}

bool saveDB(Node* root, const string& filename) {
    //saves entire database
    ofstream fout(filename.c_str());
    if (!fout) return false;
    int n = countNodes(root);
    if (n == 0) return true;
    Node** arr = new Node * [n];
    int idx = 0;
    fillInorder(root, arr, idx);
    saveMedianFirst(arr, 0, n - 1, fout);
    delete[] arr;
    return true;
}

void freeTree(Node* root) {
    // recursively deletes every node to free memory when the program ends
    if (!root) return;
    freeTree(root->left);
    freeTree(root->right);
    delete root;
}

bool readPerson(ifstream& fin, Person& r) {
    //reads one line from database file into person struct
    //returns true if successful, false at end of file
    if (!(fin >> r.last))   return false;
    if (!(fin >> r.first))  return false;
    if (!(fin >> r.state))  return false;
    if (!(fin >> r.zip))    return false;
    if (!(fin >> r.byear))  return false;
    if (!(fin >> r.bmonth)) return false;
    if (!(fin >> r.bday))   return false;
    if (!(fin >> r.password)) return false;
    if (!(fin >> r.balance))  return false;
    if (!(fin >> r.ssn))      return false;
    return true;
}

Node* findWithParent(Node* root, const string& first, const string& last, Node*& parent) {
    //find node and remember its parent (parent = null if result is the root)
    parent = NULL;
    Node* cur = root;
    while (cur != NULL) {
        int c = cmpKeys(last, first, cur->p.last, cur->p.first);
        if (c < 0) { parent = cur; cur = cur->left; }
        else if (c > 0) { parent = cur; cur = cur->right; }
        else return cur;
    }
    return NULL;
}

void printPtr(const char* label, Node* p) {
    cout << label << ": ";
    if (p == NULL) { cout << "NULL\n"; return; }
    cout << p << "  (" << p->p.first << " " << p->p.last << ")\n";
}

void printSurroundings(Node* root, const string& first, const string& last) {
    //print surroundings of a named node (node, its parent, and its children)
    Node* parent = NULL;
    Node* n = findWithParent(root, first, last, parent);
    if (n == NULL) { cout << "NOT FOUND\n"; return; }
    printPtr("NODE   ", n);
    printPtr("PARENT ", parent);
    printPtr("LEFT   ", n->left);
    printPtr("RIGHT  ", n->right);
}

void printSurroundingsByPointer(const char* title, Node* p) {
    //print surroundings for a node pointer (used to show parent content after deletion)
    cout << title << "\n";
    if (p == NULL) { cout << "(NULL)\n"; return; }
    printPtr("PARENT*", p);
    printPtr("LEFT   ", p->left);
    printPtr("RIGHT  ", p->right);
}

// Convenience: after deleting the root, show the new root surroundings
void printRootSurroundings(Node* root) {
    //after deleting root, show new root surroundings
    cout << "ROOT AFTER DELETE\n";
    if (root == NULL) { cout << "(EMPTY TREE)\n"; return; }
    printPtr("ROOT   ", root);
    printPtr("LEFT   ", root->left);
    printPtr("RIGHT  ", root->right);
}

int main() {
    ifstream fin("/home/318/database2025.txt");
    if (!fin) {
        fin.clear();
        fin.open("database2025.txt");
    }
    if (!fin) {
        cout << "Error: cannot open input file.\n";
        return 1;
    }

    Node* root = NULL;
    Person tmp;
    while (readPerson(fin, tmp)) {
        root = insert(root, tmp);
    }
    fin.close();

    string cmd;
    while (cin >> cmd) {
        if (cmd == "Exit") {
            break;
        }
        else if (cmd == "Find") {
            string f, l; if (!(cin >> f >> l)) { cout << "Not found\n"; continue; }
            Node* n = findNode(root, f, l);
            if (n) printPersonLine(n->p); else cout << "Not found\n";
        }
        else if (cmd == "Family") {
            string l; if (!(cin >> l)) continue; familyPrint(root, l);
        }
        else if (cmd == "First") {
            string f; if (!(cin >> f)) continue; firstNamePrint(root, f);
        }
        else if (cmd == "Print") {
            inorderPrint(root);
        }
        else if (cmd == "Oldest") {
            bool have = false; Person best;
            oldestScan(root, have, best);
            if (have) cout << best.first << " " << best.last << " " << best.zip << "\n";
        }
        else if (cmd == "Save") {
            string fn; if (!(cin >> fn)) continue;
            if (saveDB(root, fn)) cout << "Saved\n"; else cout << "Save failed\n";
        }
        else if (cmd == "Relocate") {
            string f, l, z; if (!(cin >> f >> l >> z)) { cout << "Not found\n"; continue; }
            Node* n = findNode(root, f, l);
            if (n) { n->p.zip = z; cout << "Updated\n"; }
            else cout << "Not found\n";
        }
        else if (cmd == "Delete") {
            string f, l; if (!(cin >> f >> l)) { cout << "Not found\n"; continue; }
            bool removed = false; root = erase(root, f, l, removed);
            cout << (removed ? "Deleted\n" : "Not found\n");
        }
        else if (cmd == "Count") {
            cout << countNodes(root) << "\n";
        }
        else if (cmd == "Inspect") {
            string f, l; if (!(cin >> f >> l)) { cout << "Not found\n"; continue; }
            printSurroundings(root, f, l);
        }
        else if (cmd == "DelTest") {
            string f, l; if (!(cin >> f >> l)) { cout << "Not found\n"; continue; }

            // find target and its parent before deletion
            Node* parent = NULL;
            Node* target = findWithParent(root, f, l, parent);
            int before = countNodes(root);
            cout << "Before count: " << before << "\n";
            if (target == NULL) {
                cout << "Not found\n";
                continue;
            }
            cout << "Before surroundings\n";
            printPtr("Node   ", target);
            printPtr("Parent ", parent);
            printPtr("Left   ", target->left);
            printPtr("Right  ", target->right);

            //deleting the target
            bool removed = false;
            root = erase(root, f, l, removed);
            cout << (removed ? "Deleted\n" : "Not found\n");

            int after = countNodes(root);
            cout << "AFTER COUNT: " << after << "\n";

            if (parent != NULL) {
                printSurroundingsByPointer("AFTER: PARENT SURROUNDINGS", parent);
            }
            else {
                // shows new root surroundings
                printRootSurroundings(root);
            }
        }
    }

    freeTree(root);
    return 0;
}


