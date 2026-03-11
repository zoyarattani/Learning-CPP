#include <iostream>
#include <string>
#include <cassert>
using namespace std;

template <typename T, typename K>
class two_three_tree {
public:
    two_three_tree() : root(0), count_(0), showSteps(false) {}
    //delete all tree nodes
    ~two_three_tree() { destroy(root); }

    void show_steps(bool v) { showSteps = v; }

    void print_tree(ostream& ostr) { printSideways(ostr, root, 0); }
    // right subtree first, then node, then left

    void insert(T item) {
        //wrap recursive insert and enforce black root
        K k = keyOf(item);
        if (showSteps) {
            cout << "\n=== INSERT " << k << " ===\n";
        }
        root = insertRec(root, item, k);
        if (root != 0) root->red = false;
        if (showSteps) {
            cout << "Tree after insert " << k << ":\n";
            print_tree(cout);
        }
    }

    T find(K key) {
        //search by key
        node* x = root;
        while (x != 0) {
            K cur = keyOf(x->data);
            if (key < cur) x = x->left;
            else if (key > cur) x = x->right;
            else return x->data;
        }
        return (T)0;
    }

    bool remove(K key) {
        //if present, delete key and re-blacken the root
        if (!contains(root, key)) {
            if (showSteps) cout << "\n=== REMOVE " << key << " (not present) ===\n";
            //if both children of root are black make them red temporarily
            return false;
        }
        if (showSteps) {
            cout << "\n=== REMOVE " << key << " ===\n";
        }
        if (!isRed(root->left) && !isRed(root->right)) {
            if (root != 0) {
                if (showSteps) cout << "root prepped red to ease deletion descent\n";
                root->red = true;
            }
        }
        root = deleteRec(root, key);
        if (root != 0) root->red = false;
        if (showSteps) {
            cout << "Tree after remove " << key << ":\n";
            print_tree(cout);
        }
        return true;
    }

protected:
    struct node {
        T data; // payload pointer (user object)
        node* left; // left child
        node* right; //right child
        bool red;
        node(T d, bool r) : data(d), left(0), right(0), red(r) {}
    };

    node* root; //tree root
    long long count_; //number of keys
    bool showSteps; //print steps

    bool isRed(node* x) { return x != 0 && x->red; } //null links are black
    K keyOf(T ptr) { return ptr->key(); } //extracts the key from the pay load

    node* rotateLeft(node* h) {
        //fix right leaning red link
        if (showSteps) cout << "rotateLeft at " << keyOf(h->data) << " (right red)\n";
        node* x = h->right;
        h->right = x->left;
        x->left = h;
        x->red = h->red; //x inherits h's color
        h->red = true; //h becomes red child
        return x;
    }

    node* rotateRight(node* h) {
        //fix two reds in a row on left
        if (showSteps) cout << "rotateRight at " << keyOf(h->data) << " (left-left red)\n";
        node* x = h->left;
        h->left = x->right;
        x->right = h;
        x->red = h->red;
        h->red = true;
        return x;
    }

    void flipColors(node* h) {
        //flip parent and children colors
        if (showSteps) {
            cout << "flipColors at " << keyOf(h->data);
            if (h->left != 0)  cout << " (L=" << keyOf(h->left->data) << ")";
            if (h->right != 0) cout << " (R=" << keyOf(h->right->data) << ")";
            cout << "\n";
        }
        h->red = !h->red;
        if (h->left != 0) h->left->red = !h->left->red;
        if (h->right != 0) h->right->red = !h->right->red;
    }

    node* fixUp(node* h) {
        if (isRed(h->right) && !isRed(h->left)) { //no right-leaning red
            h = rotateLeft(h);
        }
        if (isRed(h->left) && isRed(h->left->left)) { //no two reds in a row
            h = rotateRight(h);
        }
        if (isRed(h->left) && isRed(h->right)) { // split 4-node
            flipColors(h);
        }
        return h;
    }

    node* moveRedLeft(node* h) {
        //deletion helper when descending left
        if (showSteps) cout << "moveRedLeft at " << keyOf(h->data) << "\n";
        flipColors(h);
        if (isRed(h->right) && isRed(h->right->left)) {
            h->right = rotateRight(h->right);
            h = rotateLeft(h);
            flipColors(h);
        }
        return h;
    }

    node* moveRedRight(node* h) {
        //deletion helper when descending right
        if (showSteps) cout << "moveRedRight at " << keyOf(h->data) << "\n";
        flipColors(h);
        if (isRed(h->left) && isRed(h->left->left)) {
            h = rotateRight(h);
            flipColors(h);
        }
        return h;
    }

    node* insertRec(node* h, T item, const K& k) {
        //recursive insert
        if (h == 0) {
            count_ = count_ + 1;
            if (showSteps) cout << "  create red leaf " << k << "\n";
            return new node(item, true);
        }

        K hk = keyOf(h->data);
        if (k < hk) {
            h->left = insertRec(h->left, item, k);
        }
        else if (k > hk) {
            h->right = insertRec(h->right, item, k);
        }
        else { //duplicate key
            if (showSteps) cout << "  replace payload at key " << k << "\n";
            h->data = item;
        }

        if (isRed(h->right) && !isRed(h->left)) h = rotateLeft(h);
        if (isRed(h->left) && isRed(h->left->left)) h = rotateRight(h);
        if (isRed(h->left) && isRed(h->right)) flipColors(h);
        return h;
    }

    node* minNode(node* h) {
        //return pointer to minimum node in subtree
        node* x = h;
        while (x != 0 && x->left != 0) x = x->left;
        return x;
    }

    node* deleteMin(node* h) {
        //delete minimum node from subtree and fix invariants
        if (h->left == 0) {
            if (showSteps) cout << "  deleteMin leaf " << keyOf(h->data) << "\n";
            delete h;
            return (node*)0;
        }
        if (!isRed(h->left) && !isRed(h->left->left)) {
            h = moveRedLeft(h);
        }
        h->left = deleteMin(h->left);
        return fixUp(h);
    }

    node* deleteRec(node* h, const K& key) {
        //recursive delete of specific key
        if (key < keyOf(h->data)) {
            if (h->left != 0) {
                if (!isRed(h->left) && !isRed(h->left->left)) {
                    h = moveRedLeft(h);
                }
                h->left = deleteRec(h->left, key);
            }
        }
        else {
            if (isRed(h->left)) {
                h = rotateRight(h); //rotate to show key on right path
            }
            if (key == keyOf(h->data) && h->right == 0) { //delete leaf
                if (showSteps) cout << "  delete leaf " << key << "\n";
                delete h;
                return (node*)0;
            }
            if (h->right != 0) {
                if (!isRed(h->right) && !isRed(h->right->left)) {
                    h = moveRedRight(h);
                }
                if (key == keyOf(h->data)) { //replace it with its successor
                    node* m = minNode(h->right);
                    if (showSteps) cout << "  replace " << key << " with successor " << keyOf(m->data) << "\n";
                    h->data = m->data;
                    h->right = deleteMin(h->right);
                }
                else {
                    h->right = deleteRec(h->right, key);
                }
            }
        }
        return fixUp(h);
    }

    bool contains(node* h, const K& key) {
        node* x = h;
        while (x != 0) {
            K kx = keyOf(x->data);
            if (key < kx) x = x->left;
            else if (key > kx) x = x->right;
            else return true;
        }
        return false;
    }

    void printSideways(ostream& ostr, node* h, int depth) {
        //prints sideways, right subtree on top
        if (h == 0) return;
        printSideways(ostr, h->right, depth + 1);
        for (int i = 0; i < depth; ++i) ostr << "    ";
        ostr << (h->red ? "[R] " : "[B] ") << keyOf(h->data) << "\n";
        printSideways(ostr, h->left, depth + 1);
    }

    void destroy(node* h) {
        //frees nodes, not user payloads
        if (h == 0) return;
        destroy(h->left);
        destroy(h->right);
        delete h;
    }
};

struct Item {
    int k;
    Item(int kk) : k(kk) {}
    int key() const { return k; }
};

int main() {
    two_three_tree<Item*, int> t;
    t.show_steps(true); //show each movement and tree snapshot

    int vals[] = { 5,2,8,1,3,7,9,6,4 };
    for (int i = 0; i < 9; ++i) t.insert(new Item(vals[i]));

    cout << "\n=== FIND 7 ===\n";
    Item* f = t.find(7);
    cout << (f != 0 ? "found 7\n" : "not found\n");

    t.remove(5);
    t.remove(2);
    t.remove(8);

    return 0;
}





