#include <iostream>

#include <cmath>

#include "library.h"

using namespace std;

const double PI = 3.14159265358979323846;
const int g = 32;
const int house_width = 120;
const int house_height = 80;

double calc_height(int v, double t, double angle) {
    double radians = angle * (PI / 180);
    return v * t * sin(radians) - (g * t * t) / 2;
}
double calc_dist(int v, double t, double angle) {
    double radians = angle * (PI / 180);
    return v * t * cos(radians);
}

void print_draw_dots(int x, int y) {
    set_pen_color(color::black);
    draw_point(x, y);
}

bool check_hit(int x, int y, int house_x) {
    return (x >= house_x && x <= (house_x + house_width) && y >= (400 - house_height) && y <= 400);
}

void draw_cannon() {
    set_pen_color(color::black);
    int cannon_x = 10;
    int cannon_y = 400;
    print_draw_dots(cannon_x, cannon_y);
    print_draw_dots(cannon_x + 10, cannon_y);
    print_draw_dots(cannon_x + 20, cannon_y);
    print_draw_dots(cannon_x + 30, cannon_y);
    print_draw_dots(cannon_x + 40, cannon_y);
    print_draw_dots(cannon_x + 20, cannon_y - 10);
    print_draw_dots(cannon_x + 20, cannon_y - 20);
    print_draw_dots(cannon_x + 20, cannon_y - 30);
    print_draw_dots(cannon_x + 20, cannon_y - 40);
    print_draw_dots(cannon_x + 20, cannon_y - 50);
}

void draw_house(int house_x) {
    set_pen_color(color::black);
    print_draw_dots(house_x, 400);
    print_draw_dots(house_x, 390);
    print_draw_dots(house_x, 380);
    print_draw_dots(house_x, 370);
    print_draw_dots(house_x, 360);
    print_draw_dots(house_x, 350);
    print_draw_dots(house_x, 340);
    print_draw_dots(house_x, 330);
    print_draw_dots(house_x, 320);
    print_draw_dots(house_x, 310);
    print_draw_dots(house_x, 300);
    print_draw_dots(house_x, 290);
    print_draw_dots(house_x, 280);
    print_draw_dots(house_x, 270);
    print_draw_dots(house_x, 260);
    print_draw_dots(house_x + house_width, 400);
    print_draw_dots(house_x + house_width, 390);
    print_draw_dots(house_x + house_width, 380);
    print_draw_dots(house_x + house_width, 370);
    print_draw_dots(house_x + house_width, 360);
    print_draw_dots(house_x + house_width, 350);
    print_draw_dots(house_x + house_width, 340);
    print_draw_dots(house_x + house_width, 330);
    print_draw_dots(house_x + house_width, 320);
    print_draw_dots(house_x + house_width, 310);
    print_draw_dots(house_x + house_width, 300);
    print_draw_dots(house_x + house_width, 290);
    print_draw_dots(house_x + house_width, 280);
    print_draw_dots(house_x + house_width, 270);
    print_draw_dots(house_x + house_width, 260);
    int roof_peak_x = house_x + house_width / 2;
    int roof_peak_y = 230;
    int roof_left_x = house_x;
    int roof_left_y = 260;
    while (roof_left_y >= roof_peak_y) {
        print_draw_dots(roof_left_x, roof_left_y);
        roof_left_x += 5;
        roof_left_y -= 5;
    }
    print_draw_dots(roof_peak_x, roof_peak_y);
    int roof_right_x = house_x + house_width;
    int roof_right_y = 260;
    while (roof_right_y >= roof_peak_y) {
        print_draw_dots(roof_right_x, roof_right_y);
        roof_right_x -= 5;
        roof_right_y -= 5;
    }
    print_draw_dots(roof_peak_x, roof_peak_y);
    int door_x = house_x + house_width / 2 - 15;
    print_draw_dots(door_x, 400);
    print_draw_dots(door_x, 390);
    print_draw_dots(door_x, 380);
    print_draw_dots(door_x, 370);
    print_draw_dots(door_x, 360);
    print_draw_dots(door_x + 10, 400);
    print_draw_dots(door_x + 10, 390);
    print_draw_dots(door_x + 10, 380);
    print_draw_dots(door_x + 20, 400);
    print_draw_dots(door_x + 20, 390);
    print_draw_dots(door_x + 20, 380);
    int window_x = house_x + 10;
    print_draw_dots(window_x, 300);
    print_draw_dots(window_x, 290);
    print_draw_dots(window_x + 20, 300);
    print_draw_dots(window_x + 20, 290);
    window_x = house_x + house_width - 30;
    print_draw_dots(window_x, 300);
    print_draw_dots(window_x, 290);
    print_draw_dots(window_x + 20, 300);
    print_draw_dots(window_x + 20, 290);
}

void print_trajectory(int v, double angle, double t, double flight_time, bool& hit, int house_x) {  // Added house_x
    if (t > flight_time) {
        return;
    }

    double height = calc_height(v, t, angle);
    double dist = calc_dist(v, t, angle);

    int x = dist;
    int y = 400 - (height * 0.5);

    if (height >= 0) {
        print_draw_dots(x, y);
    }

    if (check_hit(x, y, house_x)) {
        hit = true;
        return;
    }

    print_trajectory(v, angle, t + 0.1, flight_time, hit, house_x);
}

void main() {
    int house_x = rand() % 600 + 700;
    make_window(1000, 400);
    set_pen_width(15);
    draw_cannon();
    draw_house(house_x);
    int attempts = 3;
    bool hit = false;
    while (attempts > 0 && !hit) {
        cout << "Attempt " << (4 - attempts) << ": \n";
        cout << "Initial velocity: ";
        int v;
        cin >> v;
        cout << "Angle: ";
        double angle;
        cin >> angle;
        double radians = angle * (PI / 180);
        double flight_time = (2 * v * sin(radians)) / g;
        print_trajectory(v, angle, 0, flight_time, hit, house_x);
        if (hit) {
            cout << "Hit! \n";
        }
        else {
            cout << "Missed! You have " << (attempts - 1) << " attempts left.\n";
        }
        attempts--;
    }
    if (!hit) {
        cout << "Game over! The house was at position: " << house_x << "\n";
    }
}
























































































