#include "library.h"

void draw_star(const int x, const int y, const int length)
{
	move_to(x, y);
	start_shape();
	set_heading_degrees(90);
	draw_distance(length);
	note_position();
	turn_left_by_degrees(72);
	draw_distance(length);
	note_position();
	turn_right_by_degrees(140);
	draw_distance(length);
	note_position();
	turn_left_by_degrees(72);
	draw_distance(length);
	note_position();
	turn_right_by_degrees(140);
	draw_distance(length);
	note_position();
	turn_left_by_degrees(72);
	draw_distance(length);
	note_position();
	turn_right_by_degrees(140);
	draw_distance(length);
	note_position();
	turn_left_by_degrees(55);
	draw_distance(length);
	note_position();
	turn_right_by_degrees(144);
	draw_distance(length);
	note_position();
	turn_left_by_degrees(75);
	draw_distance(length);
	note_position();
	fill_shape();
}
void draw_rectangle(const int x, const int y, int width, int height)
{
	move_to(x, y);
	start_shape();
	draw_distance(width);
	note_position();
	turn_right_by_degrees(90);
	draw_distance(height);
	note_position();
	turn_right_by_degrees(90);
	draw_distance(width);
	note_position();
	turn_right_by_degrees(90);
	draw_distance(height);
	note_position();
	fill_shape();
}

void draw_stars(int x)
{
	if (x > 0)
	{
		draw_star(25);
		move_to(50 * x, 0);
		draw_stars(x - 1);
	}
}

void draw_rectangle()
{
	make_window(500, 500);
	set_pen_color(color::dark_blue);
	fill_rectangle(0, 0, 300, 3000 / 10);
	set_pen_color(color::red);
	fill_rectangle(300, 0, 450, 400 / 10);
	fill_rectangle(300, 800 / 13, 450, 400 / 10);
	fill_rectangle(300, 1600 / 13, 450, 400 / 10);
	fill_rectangle(300, 2500 / 13, 450, 400 / 10);
	fill_rectangle(0, 3000 / 10, 760, 400 / 10);
	fill_rectangle(0, 4000 / 10, 760, 400 / 10);
	fill_rectangle(0, 5000 / 10, 760, 400 / 10);
	fill_rectangle(0, 5500 / 13, 760, 400 / 10);
}


void main()
{
	draw_rectangle();
	move_to(30, 500 / 49);
	draw_stars(6);
	move_to(60, 1000 / 40);
	draw_stars(5);
	move_to(30, 2000 / 40);
	draw_stars(6);
	move_to(60, 3000 / 40);
	draw_stars(5);
	move_to(30, 4000 / 40);
	draw_stars(6);
	move_to(60, 5000 / 40);
	draw_stars(5);
	move_to(30, 5500 / 40);
	draw_stars(6);
	move_to(60, 6500 / 40);
	draw_stars(5);
	move_to(30, 7000 / 40);
	draw_stars(6);
}























































































