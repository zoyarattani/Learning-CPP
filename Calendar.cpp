#include <iostream>

using namespace std;

bool check_leap_year(int year);
int get_days_in_month(int year, int month);
int get_day_of_week(int year, int month, int day);
void print_calendar(int year, int month);
void print_month_and_year(int& year, int& month);
void print_three_months_calendar(int year, int month);

void print_month_and_year(int& year, int& month) {
    if (month < 1) {
        month = 12;
        year--;
    }
    else if (month > 12) {
        month = 1;
        year++;
    }
}

bool check_leap_year(int year) {
    if (year % 400 == 0) return true;
    if (year % 100 == 0) return false;
    return year % 4 == 0;
}

int get_days_in_month(int year, int month) {
    if (month == 1) return 31;
    if (month == 2) {
        if (check_leap_year(year)) {
            return 29;
        }
        else {
            return 28;
        }
    }
    if (month == 3) return 31;
    if (month == 4) return 30;
    if (month == 5) return 31;
    if (month == 6) return 30;
    if (month == 7) return 31;
    if (month == 8) return 31;
    if (month == 9) return 30;
    if (month == 10) return 31;
    if (month == 11) return 30;
    if (month == 12) return 31;
    return -1;
}

int get_day_of_week(int year, int month, int day) {
    if (month < 3) {
        month += 12;
        year--;
    }
    int k = year % 100;
    int j = year / 100;
    int dayOfWeek = (day + (13 * (month + 1)) / 5 + k + (k / 4) + (j / 4) - (2 * j)) % 7;
    return (dayOfWeek + 6) % 7;
}

void print_calendar(int year, int month) {
    const char* month_names[] = { "January", "February", "March", "April", "May", "June",
                                   "July", "August", "September", "October", "November", "December" };

    const char* daysOfWeek[] = { "Su", "Mo", "Tu", "We", "Th", "Fr", "Sa" };
    int firstDay = get_day_of_week(year, month, 1);
    int daysInMonth = get_days_in_month(year, month);
    cout << month_names[month - 1] << " " << year << endl;
    for (int i = 0; i < 7; i++) {
        cout << daysOfWeek[i] << " ";
    }
    cout << endl;
    for (int i = 0; i < firstDay; i++) {
        cout << "   ";
    }
    for (int day = 1; day <= daysInMonth; day++) {
        if (day < 10) {
            cout << " " << day << " ";
        }
        else {
            cout << day << " ";
        }
        if ((firstDay + day) % 7 == 0) {
            cout << endl;
        }
    }
    cout << endl;
}

void print_three_months(int year, int month) {
    int previous_month = month - 1;
    int next_month = month + 1;
    int previous_year = year, next_year = year;
    print_month_and_year(previous_year, previous_month);
    print_month_and_year(next_year, next_month);
    print_calendar(previous_year, previous_month);
    print_calendar(year, month);
    print_calendar(next_year, next_month);
}

void main() {
    int year, month;
    cout << "Enter the year: ";
    cin >> year;
    cout << "Enter the month: ";
    cin >> month;
    print_three_months(year, month);
}





















































































