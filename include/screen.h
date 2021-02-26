#ifndef RED_SCREEN_H
#define RED_SCREEN_H

#include <Windows.h>
#include <string_view>

struct Screen_dimension {
	int width;
	int height;
};

enum class Cursor_style {
	block,
	underline
};

DWORD screen_initialize();

/*
 * Returns the width and height of the screen. Units in characters
 */
Screen_dimension screen_dimension();

/*
 * Writes the string at the cursor position, once printed the cursor is placed after the string
 */
void screen_putstring(std::string_view str);

/*
 * Set the cursor to the specified column and row.  The origin of the
 * coordinate system (0, 0) is the top left.
 */
void screen_cursor(int column, int row);

void screen_column(int column);

void screen_clear_end_of_line();

void screen_cursor_style(Cursor_style style);

void screen_cursor_visible(bool visible);

#endif
