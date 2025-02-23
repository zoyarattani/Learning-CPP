#include <iostream>
#include <vector>
#include <string>

using namespace std;

int GCD(int x, int y) {
    while (y) {
        int temp = y;
        y = x % y;
        x = temp;
    }
    return abs(x);
}

struct Frac {
    int num, den;

    Frac(int n, int d) {
        if (d == 0) {
            cout << "Error: Division by zero." << endl;
            num = 0;
            den = 1;
            return;
        }
        int g = GCD(n, d);
        num = n / g;
        den = d / g;
        if (den < 0) {
            num = -num;
            den = -den;
        }
    }

    double fractodec() const {
        return (double)num / den;
    }

    Frac add(const Frac& other) const {
        return Frac(num * other.den + other.num * den, den * other.den);
    }

    Frac sub(const Frac& other) const {
        return Frac(num * other.den - other.num * den, den * other.den);
    }

    Frac mul(const Frac& other) const {
        return Frac(num * other.num, den * other.den);
    }

    Frac div(const Frac& other) const {
        return Frac(num * other.den, den * other.num);
    }

    Frac pow(int exp) const {
        int newnum = 1, newden = 1, basenum = num, baseden = den;
        for (int i = 0; i < abs(exp); i++) {
            newnum *= basenum;
            newden *= baseden;
        }
        if (exp < 0) return Frac(newden, newnum);
        return Frac(newnum, newden);
    }

    void print() const {
        cout << num << "/" << den << " = " << fractodec() << endl;
    }
};

int fractoint(const string& s) {
    int value = 0, sign = 1, i = 0;
    if (s[i] == '-') {
        sign = -1;
        i++;
    }
    while (i < s.length()) {
        if (s[i] < '0' || s[i] > '9') return -1;
        value = value * 10 + (s[i] - '0');
        i++;
    }
    return value * sign;
}

void rpn(string input, vector<string>& varNames, vector<Frac>& varValues) {
    vector<Frac> operands;
    vector<string> substrings;
    string substring;

    for (int i = 0; i < input.length(); i++) {
        if (input[i] == ' ') {
            if (!substring.empty()) {
                substrings.push_back(substring);
                substring.clear();
            }
        }
        else {
            substring += input[i];
        }
    }
    if (!substring.empty()) substrings.push_back(substring);

    for (const string& element : substrings) {
        if ((element[0] >= '0' && element[0] <= '9') || (element[0] == '-' && element.length() > 1)) {
            int num, den = 1;
            int index = -1, count = 0;

            for (int i = 0; i < element.length(); i++) {
                if (element[i] == '/') {
                    index = i;
                    count++;
                }
            }

            if (count > 1) {
                cout << "Error: Invalid fraction format '" << element << "'\n";
                return;
            }

            if (index != -1) {
                num = fractoint(element.substr(0, index));
                den = fractoint(element.substr(index + 1));
                if (num == -1 || den == -1) {
                    cout << "Error: Invalid number format '" << element << "'\n";
                    return;
                }
            }
            else {
                num = fractoint(element);
                if (num == -1) {
                    cout << "Error: Invalid number format '" << element << "'\n";
                    return;
                }
            }
            operands.push_back(Frac(num, den));
        }
        else if (element == "+" || element == "-" || element == "*" || element == "/" || element == "^") {
            if (operands.size() < 2) {
                cout << "Error: Not enough operands.\n";
                return;
            }
            Frac b = operands.back();
            operands.pop_back();
            Frac a = operands.back();
            operands.pop_back();

            if (element == "+") operands.push_back(a.add(b));
            else if (element == "-") operands.push_back(a.sub(b));
            else if (element == "*") operands.push_back(a.mul(b));
            else if (element == "/") operands.push_back(a.div(b));
            else if (element == "^") operands.push_back(a.pow(b.num));
        }
        else if (element == "=") {
            if (operands.empty()) {
                cout << "Error: No value to assign.\n";
                return;
            }
            if (substrings.size() < 2) {
                cout << "Error: No variable name provided.\n";
                return;
            }
            string varName = substrings[0];

            bool found = false;
            for (int i = 0; i < varNames.size(); i++) {
                if (varNames[i] == varName) {
                    varValues[i] = operands.back();
                    found = true;
                    break;
                }
            }
            if (!found) {
                varNames.push_back(varName);
                varValues.push_back(operands.back());
            }
            cout << "Variable '" << varName << "' set to ";
            operands.back().print();
            return;
        }
        else {
            bool found = false;
            for (int i = 0; i < varNames.size(); i++) {
                if (varNames[i] == element) {
                    operands.push_back(varValues[i]);
                    found = true;
                    break;
                }
            }
            if (!found) {
                cout << "Error: Unknown variable '" << element << "'\n";
                return;
            }
        }
    }

    if (operands.size() == 1) {
        operands[0].print();
    }
    else {
        cout << "Error: Invalid expression.\n";
    }
}

int main() {
    vector<string> varNames = { "x", "y" };
    vector<Frac> varValues = { Frac(3, 4), Frac(5, 6) };

    string input;
    cout << "Enter values:\n";
    while (true) {
        cout << "> ";
        getline(cin, input);
        if (input == "quit") break;
        rpn(input, varNames, varValues);
    }
    return 0;
}




































