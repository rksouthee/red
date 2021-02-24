#include <Windows.h>
#include <cassert>
#include <iostream>
#include <string>
#include <vector>
#include "buffer.h"

static HANDLE screen_handle;

struct Screen_dimension {
	int width;
	int height;
};

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
		} else {
			last_error = GetLastError();
		}
	} else {
		last_error = GetLastError();
	}

	return last_error;
}

static Screen_dimension screen_dimension()
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(screen_handle, &csbi);
	Screen_dimension dimension;
	dimension.width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
	dimension.height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
	return dimension;
}

static void screen_cursor(int column, int row)
{
	COORD position;
	position.X = static_cast<SHORT>(column);
	position.Y = static_cast<SHORT>(row);
	SetConsoleCursorPosition(screen_handle, position);
}

enum class Cursor_style {
	block,
	underline
};

static void screen_cursor_style(Cursor_style style)
{
	CONSOLE_CURSOR_INFO cursor_info;
	if (style == Cursor_style::block)
		cursor_info.dwSize = 100;
	else
		cursor_info.dwSize = 1;
	cursor_info.bVisible = true;
	SetConsoleCursorInfo(screen_handle, &cursor_info);
}

static void screen_putstring(const std::string& str)
{
	DWORD length = static_cast<DWORD>(str.size());
	COORD write_coord{0, 0};
	DWORD chars_written;
	WriteConsoleOutputCharacterA(screen_handle, str.data(), length, write_coord, &chars_written);
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

struct Key {
	int code;
	bool ctrl;
	bool shift;
	bool alt;
	char ascii;
};

Key wait_for_key()
{
	INPUT_RECORD input;
	DWORD read;
	while (ReadConsoleInput(input_handle, &input, 1, &read)) {
		if (input.EventType != KEY_EVENT)
			continue;
		const KEY_EVENT_RECORD& key_event = input.Event.KeyEvent;
		if (!key_event.bKeyDown)
			continue;

		Key key;
		key.code = key_event.wVirtualKeyCode;
		key.ctrl = (key_event.dwControlKeyState & (LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED)) != 0;
		key.shift = (key_event.dwControlKeyState & SHIFT_PRESSED) != 0;
		key.alt = (key_event.dwControlKeyState & (LEFT_ALT_PRESSED | RIGHT_ALT_PRESSED)) != 0;
		key.ascii = key_event.uChar.AsciiChar;
		return key;
	}
	// TODO: handle ReadConsoleInput failure
	return {};
}

static DWORD file_open(const char* filename, Buffer& buffer)
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
		// TODO: Use GetFileSizeEx instead
		DWORD file_size = GetFileSize(file_handle, NULL);
		if (file_size != INVALID_FILE_SIZE) {
			DWORD bytes_read;
			Gap_buffer temp_buffer(std::size_t(file_size), 0);
			// TODO: Check we read the correct number of bytes
			if (ReadFile(file_handle, &temp_buffer[0], file_size, &bytes_read, NULL)) {
				buffer = Buffer(filename, std::move(temp_buffer));
			} else {
				last_error = GetLastError();
			}
		} else {
			last_error = GetLastError();
		}
		CloseHandle(file_handle);
	} else {
		last_error = GetLastError();
		if (last_error == ERROR_FILE_NOT_FOUND) {
			last_error = 0;
			buffer = Buffer(filename, Gap_buffer());
		}
	}
	return last_error;
}

/*
 * Try to safely save the file by writing to a temporary file and if that succeeds
 * move the file.
 */
static DWORD file_save(Buffer& buffer)
{
	DWORD last_error = 0;
	char temp_path[MAX_PATH];
	DWORD length = GetTempPathA(MAX_PATH, temp_path);
	if (length > 0 && length <= MAX_PATH) {
		char temp_filename[MAX_PATH];
		if (GetTempFileNameA(temp_path, "RED", 0, temp_filename) != 0) {
			HANDLE temp_file_handle = CreateFileA(temp_filename,
							      GENERIC_WRITE,
							      0,
							      NULL,
							      CREATE_ALWAYS,
							      FILE_ATTRIBUTE_NORMAL,
							      NULL);
			if (temp_file_handle != INVALID_HANDLE_VALUE) {
				if (buffer.write_file(temp_file_handle)) {
					CloseHandle(temp_file_handle);
					if (MoveFileEx(temp_filename,
						       buffer.filename(),
						       MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED)) {
					} else {
						last_error = GetLastError();
					}
				} else {
					last_error = GetLastError();
					CloseHandle(temp_file_handle);
				}
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

struct View {
	Buffer* buffer;
	int width;
	int height;
	Buffer::iterator cursor;
	Buffer::iterator top_line;
	int first_column;
	int column_desired;
};

template <typename I>
// requires BidirectionalIterator(I)
I find_backward(I f, I l, const typename std::iterator_traits<I>::value_type& x)
{
	while (l != f) {
		--l;
		if (*l == x)
			return std::next(l);
	}
	return l;
}

static void reframe(View& view)
{
	Buffer::iterator cursor_line = find_backward(view.buffer->begin(), view.cursor, '\n');
	if (cursor_line <= view.top_line) {
		view.top_line = cursor_line;
	} else {
		int rows = 1;
		Buffer::iterator line = cursor_line;
		while (rows < view.height) {
			line = find_backward(view.buffer->begin(), std::prev(line), '\n');
			if (line == view.top_line)
				goto done;
			++rows;
		}

		do {
			view.top_line  = std::find(view.top_line, view.buffer->end(), '\n');
			++view.top_line;
			if (view.top_line == line)
				goto done;
			--rows;
		} while (rows > 0);

		view.top_line = cursor_line;
	}

done:
	int column = 0;
	while (cursor_line != view.cursor) {
		if (*view.cursor == '\t') {
			column += 8 - (column & 0x7);
		} else {
			++column;
		}
		++cursor_line;
	}
	if (column < view.first_column) {
		view.first_column = column;
	} else if (view.first_column + view.width <= column) {
		view.first_column = column - view.width + 1;
	}
}

struct Editor_state {
	Buffer buffer;
	View view;
};

static Editor_state editor;

static void editor_initialize()
{
	editor.view.buffer = &editor.buffer;
	Screen_dimension size = screen_dimension();
	editor.view.width = size.width;
	editor.view.height = size.height - 1;
	editor.view.cursor = editor.buffer.begin();
	editor.view.top_line = editor.buffer.begin();
	editor.view.first_column = 0;
	editor.view.column_desired = 0;
}

static std::string display_state;

static void display_refresh(View& view)
{
	reframe(view);

	display_state.clear();
	display_state.resize(view.width * view.height, ' ');

	int cursor_row = 0;
	int cursor_column = 0;

	Buffer::iterator cursor = view.top_line;
	for (int row = 0; row < view.height; ++row) {
		// advance to the correct column
		int column = 0;
		while (column < view.first_column) {
			if (cursor == view.buffer->end())
				break;
			if (*cursor == '\n')
				break;
			if (*cursor == '\t')
				column += 8 - (column & 0x7);
			else
				++column;
			++cursor;
		}

		for (int width = 0; width < view.width;) {
			if (cursor == view.cursor) {
				cursor_row = row;
				cursor_column = width;
			}

			if (cursor == view.buffer->end())
				goto done;

			char ch = *cursor;
			if (ch == '\n')
				break;

			if (ch == '\t') {
				int nspaces = 8 - (column & 0x7);
				while (nspaces && width < view.width) {
					display_state[row * view.width + width] = ' ';
					--nspaces;
					++width;
					++column;
				}
			} else {
				display_state[row * view.width + width] = ch;
				++column;
				++width;
			}
			++cursor;
		}

		cursor = std::find(cursor, view.buffer->end(), '\n');
		if (cursor != view.buffer->end())
			++cursor;
	}

done:
	screen_putstring(display_state);
	screen_cursor(cursor_column, cursor_row);
}

void set_status_line(const char* str)
{
	Screen_dimension dimension = screen_dimension();
	COORD write_coord{0, static_cast<SHORT>(dimension.height - 1)};
	DWORD chars_written;
	FillConsoleOutputCharacterA(screen_handle, ' ', dimension.width, write_coord, &chars_written);
	DWORD length = static_cast<DWORD>(strlen(str));
	WriteConsoleOutputCharacterA(screen_handle, str, length, write_coord, &chars_written);
}

enum User_response {
	yes, no, cancel
};

User_response prompt_yesno(const char* message)
{
	User_response result;
	while (true) {
		set_status_line(message);
		Key response = wait_for_key();

		if (response.ascii == 'y' || response.ascii == 'Y') {
			result = User_response::yes;
			break;
		}

		if (response.ascii == 'n' || response.ascii == 'N') {
			result = User_response::no;
			break;
		}

		if (response.code == VK_ESCAPE) {
			result = User_response::cancel;
			break;
		}
	}

	set_status_line("");
	return result;
}

#define COMMAND_FUNCTION(name) void name(const Key& key, bool& should_exit)
typedef COMMAND_FUNCTION((*Command_function));

COMMAND_FUNCTION(command_none)
{
}

COMMAND_FUNCTION(save)
{
	DWORD last_error = file_save(editor.buffer);
	if (last_error == 0) {
		set_status_line("Saved buffer");
	} else {
		set_status_line("Error saving buffer");
	}
}

COMMAND_FUNCTION(forward_char)
{
	View& view = editor.view;
	if (view.cursor != view.buffer->end()) {
		++view.cursor;
		view.column_desired = -1;
	}
}

COMMAND_FUNCTION(backward_char)
{
	View& view = editor.view;
	if (view.cursor != view.buffer->begin()) {
		--view.cursor;
		view.column_desired = -1;
	}
}

static int get_column(Buffer::iterator first, Buffer::iterator last)
{
	first = find_backward(first, last, '\n');
	int column = 0;
	while (first != last) {
		if (*first == '\t')
			column += 8 - (column & 0x7);
		else
			++column;
		++first;
	}
	return column;
}

static Buffer::iterator set_column(Buffer::iterator first, Buffer::iterator last, int column)
{
	int current = 0;
	while (first != last && *first != '\n' && current < column) {
		if (*first == '\t')
			current += 8 - (current & 0x7);
		else
			++current;
		++first;
	}
	return first;
}

COMMAND_FUNCTION(forward_line)
{
	View& view = editor.view;
	if (view.column_desired == -1)
		view.column_desired = get_column(view.buffer->begin(), view.cursor);
	Buffer::iterator end_of_line = std::find(view.cursor, view.buffer->end(), '\n');
	if (end_of_line != view.buffer->end())
		view.cursor = set_column(std::next(end_of_line), view.buffer->end(), view.column_desired);
}

COMMAND_FUNCTION(backward_line)
{
	View& view = editor.view;
	if (view.column_desired == -1)
		view.column_desired = get_column(view.buffer->begin(), view.cursor);
	view.cursor = find_backward(view.buffer->begin(), view.cursor, '\n');
	if (view.cursor != view.buffer->begin()) {
		Buffer::iterator start_of_line = find_backward(view.buffer->begin(), std::prev(view.cursor), '\n');
		view.cursor = set_column(start_of_line, view.buffer->end(), view.column_desired);
	}
}

COMMAND_FUNCTION(quit)
{
	if (editor.buffer.modified()) {
		User_response answer = prompt_yesno("quit (y/n)");
		should_exit = answer == User_response::yes;
	} else {
		should_exit = true;
	}
}

COMMAND_FUNCTION(save);

COMMAND_FUNCTION(start_insert_mode);

/*
 * 11 bits for key state
 * A9876543210
 * sackkkkkkkk
 * s = shift
 * a = alt
 * c = ctrl
 * k = key code
 */
#define MAX_KEYS (1 << (8 + 3))
#define CONTROL_SHIFT (1 << 8)
static Command_function normal_mode[MAX_KEYS];

static inline UINT control(UINT key_code)
{
	return key_code | CONTROL_SHIFT;
}

static void normal_mode_initialize()
{
	for (int i = 0; i < MAX_KEYS; ++i) {
		normal_mode[i] = command_none;
	}

	normal_mode['H'] = backward_char;
	normal_mode['J'] = forward_line;
	normal_mode['K'] = backward_line;
	normal_mode['L'] = forward_char;
	normal_mode['I'] = start_insert_mode;
	normal_mode[control('S')] = save;
	normal_mode[control('Q')] = quit;
}

/*
 * XXX: Assuming codepage 850, should correctly use locales and switch to utf-8
 */
static bool is_print(char character)
{
	auto uchar = static_cast<unsigned char>(character);
	return uchar == '\t' || (uchar >= ' ' && uchar <= '~') || (uchar >= 128 && uchar <= 254);
}

COMMAND_FUNCTION(command_self_insert)
{
	char character = key.ascii;
	if (is_print(character)) {
		editor.buffer.insert(editor.view.cursor, character);
		++editor.view.cursor;
	}
}

COMMAND_FUNCTION(command_newline)
{
	editor.buffer.insert(editor.view.cursor, '\n');
	++editor.view.cursor;
}

COMMAND_FUNCTION(command_backspace)
{
	View& view = editor.view;
	if (view.cursor != view.buffer->begin()) {
		--view.cursor;
		view.buffer->erase(view.cursor);
	}
}

static Command_function insert_mode[MAX_KEYS];

COMMAND_FUNCTION(leave_insert_mode);

static void insert_mode_initialize()
{
	for (int i = 0; i < MAX_KEYS; ++i) {
		insert_mode[i] = command_self_insert;
	}

	insert_mode[VK_ESCAPE] = leave_insert_mode;
	insert_mode[VK_RETURN] = command_newline;
	insert_mode[VK_BACK] = command_backspace;
}

static Command_function* commands = normal_mode;

COMMAND_FUNCTION(start_insert_mode)
{
	commands = insert_mode;
	set_status_line("--INSERT--");
	screen_cursor_style(Cursor_style::underline);
}

COMMAND_FUNCTION(leave_insert_mode)
{
	commands = normal_mode;
	set_status_line("");
	screen_cursor_style(Cursor_style::block);
	editor.view.column_desired = -1;
}

bool evaluate(const Key& key)
{
	assert(key.code >= 0 && key.code < 256);
	unsigned index = key.code;
	index |= key.ctrl << 8;
	index |= key.alt << 9;
	index |= key.shift << 10;
	assert(index >= 0 && index < MAX_KEYS);

	Command_function cmd = commands[index];
	assert(cmd);
	bool should_exit = false;
	cmd(key, should_exit);
	display_refresh(editor.view);
	return should_exit;
}

static void handle_window_buffer_size_event(const WINDOW_BUFFER_SIZE_RECORD& size_event)
{
	/*
	 * When the console window is resized, if the height of the screen is increased
	 * windows introduces a vertical scrollbar, ideally we would like to remove the
	 * scrollbar but trying to handle it here results in the width of the screen buffer
	 * not matching the width of the window due to the scrollbar.
	 *
	 * Vim handles this by resizing the height when the window size changes, and when
	 * the user presses CTRL-L the width is adjusted (shortened) appropriately.
	 */
	UNREFERENCED_PARAMETER(size_event);
	Screen_dimension size = screen_dimension();
	editor.view.width = size.width;
	editor.view.height = size.height - 1;
	display_refresh(editor.view);
}

int main(int argc, char **argv)
{
	if (argc != 2) {
		std::cerr << "Usage: RED <filename>" << std::endl;
		return -1;
	}

	SetConsoleTitle("RED");
	DWORD last_error = screen_initialize();

	if (last_error == 0) {
		editor_initialize();
		screen_cursor_style(Cursor_style::block);
		normal_mode_initialize();
		insert_mode_initialize();
		last_error = input_initialize();
		if (last_error == 0) {
			const char* filename = argv[1];
			last_error = file_open(filename, editor.buffer);
			if (last_error == 0) {
				display_refresh(editor.view);
				while (true) {
					Key key = wait_for_key();
					if (evaluate(key))
						break;
					display_refresh(editor.view);
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


	// Restore the original title
	const DWORD title_size = 64 * 1024;
	char console_title[title_size];
	GetConsoleOriginalTitle(console_title, title_size);
	SetConsoleTitle(console_title);

	return last_error;
}
