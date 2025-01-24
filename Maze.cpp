#include <iostream>
#include "library.h"
#include <fstream>
#include <ctime>
#include <cstdlib>
using namespace std;

const int rows = 20;
const int cols = 20;
char maze[rows][cols];
int robot_row, robot_col, target_row, target_col, enemy_row, enemy_col;
bool auto_mode = false;

void load_maze(const string& filename) {
    ifstream fin(filename);
    if (!fin) {
        cout << "Error opening file.\n";
        return;
    }
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            fin >> maze[i][j];
            if (maze[i][j] == '+') {
                robot_row = i;
                robot_col = j;
            }
            if (maze[i][j] == '$') {
                target_row = i;
                target_col = j;
            }
            if (maze[i][j] == 'E') {
                enemy_row = i;
                enemy_col = j;
            }
        }
    }
    fin.close();
}

void draw_maze() {
    set_pen_color(color::black);
    int cell_size = 20;
    int grid_border = 2;
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (maze[i][j] == '#') {
                fill_rectangle(j * cell_size, i * cell_size, cell_size, cell_size);
            }
        }
    }
    set_pen_color(color::blue);
    fill_rectangle(robot_col * cell_size, robot_row * cell_size, cell_size, cell_size);
    set_pen_color(color::red);
    fill_rectangle(target_col * cell_size, target_row * cell_size, cell_size, cell_size);
    set_pen_color(color::green);
    fill_rectangle(enemy_col * cell_size, enemy_row * cell_size, cell_size, cell_size);
}

bool move_robot(int x, int y) {
    int new_row = robot_row + x;
    int new_col = robot_col + y;
    if (new_row >= 0 && new_row < rows && new_col >= 0 && new_col < cols && maze[new_row][new_col] != '#') {
        robot_row = new_row;
        robot_col = new_col;
        return true;
    }
    return false;
}

bool move_enemy() {
    int direction = rand() % 4;
    switch (direction) {
    case 0:
        if (maze[enemy_row - 1][enemy_col] != '#' && enemy_row > 0) enemy_row--;
        break;
    case 1:
        if (maze[enemy_row + 1][enemy_col] != '#' && enemy_row < rows - 1) enemy_row++;
        break;
    case 2:
        if (maze[enemy_row][enemy_col - 1] != '#' && enemy_col > 0) enemy_col--;
        break;
    case 3:
        if (maze[enemy_row][enemy_col + 1] != '#' && enemy_col < cols - 1) enemy_col++;
        break;
    }
    return true;
}

void check_game_over() {
    if (robot_row == enemy_row && robot_col == enemy_col) {
        cout << "Game Over! The enemy caught you!\n";
        exit(0);
    }
}

void main() {
    make_window(cols * 20, rows * 20);
    load_maze("Maze.txt");
    srand(time(0));
    while (true) {
        clear();
        draw_maze();
        if (robot_row == target_row && robot_col == target_col) {
            cout << "Robot found the target!\n";
            break;
        }
        char c = wait_for_key_typed(0.3);
        if (c == 'a' || c == 'A') auto_mode = true;
        else if (c == 'm' || c == 'M') auto_mode = false;

        if (!auto_mode) {
            if (c == 'u') move_robot(-1, 0);
            else if (c == 'd') move_robot(1, 0);
            else if (c == 'l') move_robot(0, -1);
            else if (c == 'r') move_robot(0, 1);
        }
        else {
            move_robot(rand() % 3 - 1, rand() % 3 - 1);
            move_enemy();
        }
        check_game_over();
        wait_for_key_typed(0.5);
    }
}
















































































