#include "screen.h"

static HANDLE screen_handle;

DWORD screen_initialize()
{
	DWORD last_error = 0;
	screen_handle = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE,
						  FILE_SHARE_WRITE,
						  NULL,
						  CONSOLE_TEXTMODE_BUFFER,
						  NULL);
	if (screen_handle != INVALID_HANDLE_VALUE) {
		if (SetConsoleActiveScreenBuffer(screen_handle)) {
		} else {
			last_error = GetLastError();
		}
	} else {
		last_error = GetLastError();
	}

	return last_error;
}

Screen_dimension screen_dimension()
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(screen_handle, &csbi);
	Screen_dimension dimension;
	dimension.width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
	dimension.height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
	return dimension;
}

void screen_cursor(int column, int row)
{
	COORD position;
	position.X = static_cast<SHORT>(column);
	position.Y = static_cast<SHORT>(row);
	SetConsoleCursorPosition(screen_handle, position);
}

void screen_column(int column)
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(screen_handle, &csbi);
	csbi.dwCursorPosition.X = static_cast<SHORT>(column);
	SetConsoleCursorPosition(screen_handle, csbi.dwCursorPosition);
}

void screen_cursor_style(Cursor_style style)
{
	CONSOLE_CURSOR_INFO cursor_info;
	if (style == Cursor_style::block)
		cursor_info.dwSize = 100;
	else
		cursor_info.dwSize = 1;
	cursor_info.bVisible = true;
	SetConsoleCursorInfo(screen_handle, &cursor_info);
}

void screen_putstring(const std::string& str)
{
	DWORD length = static_cast<DWORD>(str.size());
	DWORD chars_written;
	WriteConsole(screen_handle, str.data(), length, &chars_written, nullptr);
}

void screen_clear_end_of_line()
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(screen_handle, &csbi);
	COORD position = csbi.dwCursorPosition;
	DWORD width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
	DWORD chars_written;
	FillConsoleOutputCharacterA(screen_handle, ' ', width - position.X, position, &chars_written);
}

void screen_cursor_visible(bool visible)
{
	CONSOLE_CURSOR_INFO cursor;
	GetConsoleCursorInfo(screen_handle, &cursor);
	cursor.bVisible = visible;
	SetConsoleCursorInfo(screen_handle, &cursor);
}
