#include <iostream>
#include "library.h"

using namespace std;

const int button_radius = 40;
const int start_1 = 100;
const int start_2 = 200;
const int spacing = 100;
int current_number = 0;
int previous_number = 0;
char pending_operation = '\0';

void draw_button(int x, int y) {
    draw_point(x, y);
}

void fill_circle(int center1, int center2, int radius) {
    set_pen_color(color::light_blue);
    for (int y = -radius; y <= radius; y++) {
        for (int x = -radius; x <= radius; x++) {
            if (x * x + y * y <= radius * radius) {
                draw_button(center1 + x, center2 + y);
            }
        }
    }
}

void draw_circle_outline(int center1, int center2, int radius) {
    set_pen_color(color::purple);
    int x = radius;
    int y = 0;
    int radius_error = 1 - x;
    while (x >= y) {
        draw_button(center1 + x, center2 + y);
        draw_button(center1 + y, center2 + x);
        draw_button(center1 - y, center2 + x);
        draw_button(center1 - x, center2 + y);
        draw_button(center1 - x, center2 - y);
        draw_button(center1 - y, center2 - x);
        draw_button(center1 + y, center2 - x);
        draw_button(center1 + x, center2 - y);
        y++;
        if (radius_error < 0) {
            radius_error += 2 * y + 1;
        }
        else {
            x--;
            radius_error += 2 * (y - x + 1);
        }
    }
}

void draw_label(int center1, int center2, const string& label) {
    set_pen_color(color::black);
    move_to(center1, center2);

    if (label == "×") {
        write_char(L'×');
    }
    else if (label == "÷") {
        write_char(L'÷');
    }
    else if (label == "Σ") {
        write_char(L'Σ');
    }
    else if (label == "↑") {
        write_char(L'↑');
    }
    else if (label == "©") {
        write_char(L'©');
    }
    else {
        write_string(label);
    }
}

void draw_display(int value) {
    set_pen_color(color::black);
    move_to(start_1, start_2 - 100);
    if (value != 0 || current_number != 0) {
        write_string(value);
    }
}

void draw_calculator() {
    make_window(600, 600);
    set_pen_width(5);

    string labels[4][5] = {
        {"7", "8", "9", "+", "^"},
        {"4", "5", "6", "-", "©"},
        {"1", "2", "3", "×", "Σ"},
        {"0", "C", "=", "÷", "M"}
    };
    draw_display(current_number);

    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 5; col++) {
            int center1 = start_1 + col * spacing;
            int center2 = start_2 + row * spacing;
            fill_circle(center1, center2, button_radius);
            draw_circle_outline(center1, center2, button_radius);
            draw_label(center1, center2, labels[row][col]);
        }
    }
}

int button_value() {
    wait_for_mouse_click();
    const int x = get_click_x(), y = get_click_y();
    int row = (y - start_2 + spacing / 2) / spacing;
    int col = (x - start_1 + spacing / 2) / spacing;
    if (row >= 0 && row < 4 && col >= 0 && col < 5) {
        int center1 = start_1 + col * spacing;
        int center2 = start_2 + row * spacing;
        if ((x - center1) * (x - center1) + (y - center2) * (y - center2) <= button_radius * button_radius) {
            if (row == 0 && col == 0) return 7;
            if (row == 0 && col == 1) return 8;
            if (row == 0 && col == 2) return 9;
            if (row == 0 && col == 3) return 10;
            if (row == 0 && col == 4) return 11;
            if (row == 1 && col == 0) return 4;
            if (row == 1 && col == 1) return 5;
            if (row == 1 && col == 2) return 6;
            if (row == 1 && col == 3) return 12;
            if (row == 1 && col == 4) return 13;
            if (row == 2 && col == 0) return 1;
            if (row == 2 && col == 1) return 2;
            if (row == 2 && col == 2) return 3;
            if (row == 2 && col == 3) return 14;
            if (row == 2 && col == 4) return 15;
            if (row == 3 && col == 0) return 0;
            if (row == 3 && col == 1) return 16;
            if (row == 3 && col == 2) return 17;
            if (row == 3 && col == 3) return 18;
            if (row == 3 && col == 4) return 19;
        }
    }
    return -1;
}

void print_operation() {
    if (pending_operation == '+') {
        previous_number += current_number;
    }
    else if (pending_operation == '-') {
        previous_number -= current_number;
    }
    else if (pending_operation == '×') {
        previous_number *= current_number;
    }
    else if (pending_operation == '÷') {
        if (current_number != 0) {
            previous_number /= current_number;
        }
    }
    current_number = previous_number;
}

void main() {
    draw_calculator();
    while (true) {
        int value = button_value();
        if (value >= 0 && value <= 9) {
            if (current_number == 0 && value != 0) {
                current_number = value;
            }
            else {
                current_number = current_number * 10 + value;
            }
            draw_display(current_number);
        }
        else if (value == 10) {
            if (pending_operation != '\0') {
                print_operation();
            }
            previous_number = current_number;
            pending_operation = '+';
            current_number = 0;
            draw_display(current_number);
        }
        else if (value == 12) {
            if (pending_operation != '\0') {
                print_operation();
            }
            previous_number = current_number;
            pending_operation = '-';
            current_number = 0;
            draw_display(current_number);
        }
        else if (value == 14) {
            if (pending_operation != '\0') {
                print_operation();
            }
            previous_number = current_number;
            pending_operation = '×';
            current_number = 0;
            draw_display(current_number);
        }
        else if (value == 18) {
            if (pending_operation != '\0') {
                print_operation();
            }
            previous_number = current_number;
            pending_operation = '÷';
            current_number = 0;
            draw_display(current_number);
        }
        else if (value == 17) {
            if (pending_operation != '\0') {
                print_operation();
                draw_display(current_number);
                previous_number = 0;
                pending_operation = '\0';
                current_number = 0;
            }
        }
        else if (value == 16) {
            current_number = 0;
            previous_number = 0;
            pending_operation = '\0';
            draw_display(current_number);
        }
    }
}



















































































