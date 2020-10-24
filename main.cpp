#include <Windows.h>
#include <cassert>
#include <string>
#include <vector>

static HANDLE screen_handle;
static int screen_width;
static int screen_height;
static std::vector<char> screen_state;
static int screen_x;
static int screen_y;

static DWORD screen_initialize()
{
	DWORD last_error = 0;
	screen_handle = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE,
						  FILE_SHARE_WRITE,
						  NULL,
						  CONSOLE_TEXTMODE_BUFFER,
						  NULL);
	if (screen_handle != INVALID_HANDLE_VALUE) {
		if (SetConsoleActiveScreenBuffer(screen_handle)) {
			CONSOLE_SCREEN_BUFFER_INFO csbi;
			if (GetConsoleScreenBufferInfo(screen_handle, &csbi)) {
				screen_width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
				screen_height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
				screen_state.resize(screen_width * screen_height, ' ');
			} else {
				last_error = GetLastError();
			}
		} else {
			last_error = GetLastError();
		}
	} else {
		last_error = GetLastError();
	}

	return last_error;
}

static void screen_putchar(char c)
{
	assert(screen_x < screen_width && screen_y <= screen_height);
	if (screen_y == screen_height)
		return;

	int index = screen_y * screen_width + screen_x;
	if (c == '\n') {
		std::fill_n(screen_state.begin() + index, screen_width - screen_x, ' ');
		++screen_y;
		screen_x = 0;
		return;
	}

	screen_state[index] = c;

	++screen_x;
	if (screen_x == screen_width) {
		++screen_y;
		screen_x = 0;
	}
}

/* static void screen_putstring(const char* str) */
/* { */
/* 	while (*str) */
/* 		screen_putchar(*str++); */
/* } */

static void screen_update()
{
	DWORD size = static_cast<DWORD>(screen_state.size());
	DWORD chars_written;
	COORD cursor{0, 0};
	// TODO: check success and number of characters written
	WriteConsoleOutputCharacterA(screen_handle, screen_state.data(), size, cursor, &chars_written);
	cursor.X = static_cast<SHORT>(screen_x);
	cursor.Y = static_cast<SHORT>(screen_y);
	SetConsoleCursorPosition(screen_handle, cursor);
}

static HANDLE input_handle;

static DWORD input_initialize()
{
	DWORD last_error = 0;
	input_handle = CreateFileA("CONIN$",
				   GENERIC_READ | GENERIC_WRITE,
				   FILE_SHARE_READ,
				   NULL,
				   OPEN_EXISTING,
				   0,
				   0);
	if (input_handle != INVALID_HANDLE_VALUE) {
		DWORD input_mode;
		if (GetConsoleMode(input_handle, &input_mode)) {
			DWORD cooked_flags = ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT | ENABLE_PROCESSED_INPUT;
			input_mode &= ~cooked_flags;
			if (SetConsoleMode(input_handle, input_mode)) {
			} else {
				last_error = GetLastError();
			}
		} else {
			last_error = GetLastError();
		}
	} else {
		last_error = GetLastError();
	}

	return last_error;
}


using Text_position = std::string::size_type;
static std::string text_buffer;
static Text_position text_position;
static int text_column;
static int text_column_desired;
static Text_position window_line_top;

static DWORD file_open(const char* filename)
{
	DWORD last_error = 0;
	HANDLE file_handle = CreateFileA(filename,
					 GENERIC_READ | GENERIC_WRITE,
					 FILE_SHARE_READ,
					 NULL,
					 OPEN_EXISTING,
					 FILE_ATTRIBUTE_NORMAL,
					 0);
	if (file_handle != INVALID_HANDLE_VALUE) {
		DWORD file_size = GetFileSize(file_handle, NULL);
		if (file_size != INVALID_FILE_SIZE) {
			DWORD bytes_read;
			std::string temp_buffer(static_cast<std::string::size_type>(file_size), 0);
			if (ReadFile(file_handle, &temp_buffer[0], file_size, &bytes_read, NULL)) {
				text_buffer = std::move(temp_buffer);
			} else {
				last_error = GetLastError();
			}
		} else {
			last_error = GetLastError();
		}
		CloseHandle(file_handle);
	} else {
		last_error = GetLastError();
	}
	return last_error;
}

Text_position find_backward(Text_position p, char c)
{
	while (p != 0) {
		if (text_buffer[p - 1] == c)
			break;
		--p;
	}
	return p;
}

Text_position find_forward(Text_position p, char c)
{
	while (p != text_buffer.size() && text_buffer[p] != c)
		++p;
	return p;
}

static void reframe()
{
	Text_position cursor_line = find_backward(text_position, '\n');
	if (cursor_line <= window_line_top) {
		window_line_top = cursor_line;
	} else {
		int rows = 1;
		Text_position line = cursor_line;
		while (rows < screen_height) {
			line = find_backward(line - 1, '\n');
			if (line == window_line_top)
				return;
			++rows;
		}

		do {
			window_line_top = find_forward(window_line_top, '\n') + 1;
			if (window_line_top == line)
				return;
			--rows;
		} while (rows > 0);

		window_line_top = cursor_line;
	}
}

static void display_refresh()
{
	reframe();

	screen_x = 0;
	screen_y = 0;

	Text_position start = window_line_top;
	while (start != text_position) {
		screen_putchar(text_buffer[start]);
		++start;
	}

	int cursor_x = screen_x;
	int cursor_y = screen_y;

	while (start != text_buffer.size()) {
		screen_putchar(text_buffer[start]);
		++start;
	}

	int index = screen_y * screen_width + screen_x;
	std::fill(screen_state.begin() + index, screen_state.end(), ' ');

	screen_x = cursor_x;
	screen_y = cursor_y;

	screen_update();
}

static void move_left()
{
	if (text_position > 0) {
		--text_position;
		if (text_column == 0) {
			Text_position line_start = find_backward(text_position, '\n');
			text_column = static_cast<int>(text_position - line_start);
		} else {
			--text_column;
		}
		text_column_desired = text_column;
	}
}

static void move_down()
{
	Text_position p = find_forward(text_position, '\n');
	if (p != text_buffer.size()) {
		text_position = p + 1;
		text_column = 0;
		while (text_position != text_buffer.size() &&
		       text_buffer[text_position] != '\n' &&
		       text_column < text_column_desired) {
			++text_position;
			++text_column;
		}
	}
}

static void move_up()
{
	Text_position p = find_backward(text_position, '\n');
	if (p != 0) {
		Text_position line_start = find_backward(p - 1, '\n');
		text_column = std::min(text_column_desired, static_cast<int>(p - 1 - line_start));
		text_position = line_start + text_column;
	}
}

static void move_right()
{
	if (text_position != text_buffer.size()) {
		if (text_buffer[text_position] == '\n') {
			text_column = 0;
		} else {
			++text_column;
		}
		++text_position;
		text_column_desired = text_column;
	}
}

int main()
{
	DWORD last_error = screen_initialize();

	if (last_error == 0) {
		last_error = input_initialize();
		if (last_error == 0) {
			last_error = file_open("lipsum.txt");
			if (last_error == 0) {
				display_refresh();
				bool running = true;
				INPUT_RECORD input_record;
				DWORD events_read;
				while (running && ReadConsoleInput(input_handle, &input_record, 1, &events_read)) {
					switch (input_record.EventType) {
					case KEY_EVENT: {
						const KEY_EVENT_RECORD& key_event = input_record.Event.KeyEvent;
						if (key_event.bKeyDown) {
							switch (key_event.uChar.AsciiChar) {
							case 27:
								running = false;
								break;
							case 'h':
								move_left();
								break;
							case 'j':
								move_down();
								break;
							case 'k':
								move_up();
								break;
							case 'l':
								move_right();
								break;
							}
							display_refresh();
						}
					}
					}
				}
			} else {
				OutputDebugStringA("Failed to load file\n");
			}
		} else {
			OutputDebugStringA("Failed to initialize input\n");
		}
	} else {
		OutputDebugStringA("Failed to initialize screen buffer\n");
	}

	return last_error;
}
