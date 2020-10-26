#include <Windows.h>
#include <cassert>
#include <string>
#include <vector>

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

using Text_position = std::string::size_type;
struct Text_buffer {
	std::string contents;
	const char* filename;
	Text_position cursor;
	int column;
	int column_desired;
};

static DWORD file_open(const char* filename, Text_buffer& buffer)
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
				buffer.contents = std::move(temp_buffer);
				buffer.filename = filename;
				buffer.cursor = 0;
				buffer.column = 0;
				buffer.column_desired = 0;
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

Text_position find_backward(const Text_buffer& buffer, Text_position p, char c)
{
	while (p != 0) {
		if (buffer.contents[p - 1] == c)
			break;
		--p;
	}
	return p;
}

Text_position find_forward(const Text_buffer& buffer, Text_position p, char c)
{
	while (p != buffer.contents.size() && buffer.contents[p] != c)
		++p;
	return p;
}

struct View {
	Text_buffer* buffer;
	int width;
	int height;
	Text_position top_line;
	int first_column;
};

static void reframe(View& view)
{
	Text_position cursor_line = find_backward(*view.buffer, view.buffer->cursor, '\n');
	if (cursor_line <= view.top_line) {
		view.top_line = cursor_line;
	} else {
		int rows = 1;
		Text_position line = cursor_line;
		while (rows < view.height) {
			line = find_backward(*view.buffer, line - 1, '\n');
			if (line == view.top_line)
				goto done;
			++rows;
		}

		do {
			view.top_line = find_forward(*view.buffer, view.top_line, '\n') + 1;
			if (view.top_line == line)
				goto done;
			--rows;
		} while (rows > 0);

		view.top_line = cursor_line;
	}

done:
	int column = static_cast<int>(view.buffer->cursor - cursor_line);
	if (column < view.first_column) {
		view.first_column = column;
	} else if (view.first_column + view.width <= column) {
		view.first_column = column - view.width + 1;
	}
}

struct Editor_state {
	Text_buffer buffer;
	View view;
	std::string status_line;
};

static Editor_state editor;

static void editor_initialize()
{
	editor.view.buffer = &editor.buffer;
	Screen_dimension size = screen_dimension();
	editor.view.width = size.width;
	editor.view.height = size.height - 1;
	editor.view.top_line = 0;
	editor.view.first_column = 0;
}

static std::string display_state;

static void display_refresh(View& view)
{
	reframe(view);

	display_state.clear();
	display_state.resize(view.width * (view.height + 1), ' ');

	int cursor_row = 0;
	int cursor_column = 0;

	Text_position cursor = view.top_line;
	for (int row = 0; row < view.height; ++row) {
		// advance to the correct column
		for (int i = 0; i < view.first_column; ++i) {
			if (cursor == view.buffer->contents.size())
				break;
			if (view.buffer->contents[cursor] == '\n')
				break;
			++cursor;
		}

		for (int column = 0; column < view.width; ++column) {
			if (cursor == view.buffer->cursor) {
				cursor_row = row;
				cursor_column = column;
			}

			if (cursor == view.buffer->contents.size())
				goto done;

			char ch = view.buffer->contents[cursor];
			if (ch == '\n')
				break;

			display_state[row * view.width + column] = ch;
			++cursor;
		}

		cursor = find_forward(*view.buffer, cursor, '\n');
		if (cursor != view.buffer->contents.size())
			++cursor;
	}

done:
	int n = std::min(static_cast<int>(editor.status_line.size()), view.width);
	for (int i = 0; i < n; ++i) {
		display_state[view.height * view.width + i] = editor.status_line[i];
	}
	screen_putstring(display_state);
	screen_cursor(cursor_column, cursor_row);
}

static void move_left(Text_buffer& buffer)
{
	if (buffer.cursor > 0) {
		--buffer.cursor;
		if (buffer.column == 0) {
			Text_position line_start = find_backward(buffer, buffer.cursor, '\n');
			buffer.column = static_cast<int>(buffer.cursor - line_start);
		} else {
			--buffer.column;
		}
		buffer.column_desired = buffer.column;
	}
}

static void move_down(Text_buffer& buffer)
{
	Text_position p = find_forward(buffer, buffer.cursor, '\n');
	if (p != buffer.contents.size()) {
		buffer.cursor = p + 1;
		buffer.column = 0;
		while (buffer.cursor != buffer.contents.size() &&
		       buffer.contents[buffer.cursor] != '\n' &&
		       buffer.column < buffer.column_desired) {
			++buffer.cursor;
			++buffer.column;
		}
	}
}

static void move_up(Text_buffer& buffer)
{
	Text_position p = find_backward(buffer, buffer.cursor, '\n');
	if (p != 0) {
		Text_position line_start = find_backward(buffer, p - 1, '\n');
		buffer.column = std::min(buffer.column_desired, static_cast<int>(p - 1 - line_start));
		buffer.cursor = line_start + buffer.column;
	}
}

static void move_right(Text_buffer& buffer)
{
	if (buffer.cursor != buffer.contents.size()) {
		if (buffer.contents[buffer.cursor] == '\n') {
			buffer.column = 0;
		} else {
			++buffer.column;
		}
		++buffer.cursor;
		buffer.column_desired = buffer.column;
	}
}

static void insert(Text_buffer& buffer, char character)
{
	buffer.contents.insert(buffer.cursor, 1, character);
}

typedef void (*Command_function)(void);

static void command_none()
{
}

static void forward_char()
{
	move_right(editor.buffer);
}

static void backward_char()
{
	move_left(editor.buffer);
}

static void forward_line()
{
	move_down(editor.buffer);
}

static void backward_line()
{
	move_up(editor.buffer);
}

static bool running;

static void quit()
{
	running = false;
}

static void start_insert_mode();

static Command_function normal_mode[256];

static void normal_mode_initialize()
{
	for (int i = 0; i < 256; ++i) {
		normal_mode[i] = command_none;
	}

	normal_mode[27] = quit;
	normal_mode['h'] = backward_char;
	normal_mode['j'] = forward_line;
	normal_mode['k'] = backward_line;
	normal_mode['l'] = forward_char;
	normal_mode['i'] = start_insert_mode;
}

static KEY_EVENT_RECORD last_key_event;

static void command_self_insert()
{
	char character = last_key_event.uChar.AsciiChar;
	if (std::isprint(character)) {
		insert(editor.buffer, character);
		++editor.buffer.cursor;
	}
}

static Command_function insert_mode[256];

static void leave_insert_mode();

static void insert_mode_initialize()
{
	for (int i = 0; i < 256; ++i) {
		if (std::isprint(i))
			insert_mode[i] = command_self_insert;
		else
			insert_mode[i] = command_none;
	}

	insert_mode[27] = leave_insert_mode;
}

static Command_function* commands = normal_mode;

static void start_insert_mode()
{
	commands = insert_mode;
	editor.status_line = "--INSERT--";
}

static void leave_insert_mode()
{
	commands = normal_mode;
	editor.status_line.clear();
}

static void handle_key_event(const KEY_EVENT_RECORD& key_event)
{
	if (key_event.bKeyDown) {
		last_key_event = key_event;
		commands[key_event.uChar.AsciiChar]();
		// TODO: only redraw if something changed?
		display_refresh(editor.view);
	}
}

int main()
{
	DWORD last_error = screen_initialize();

	if (last_error == 0) {
		editor_initialize();
		normal_mode_initialize();
		insert_mode_initialize();
		last_error = input_initialize();
		if (last_error == 0) {
			last_error = file_open("lipsum.txt", editor.buffer);
			if (last_error == 0) {
				display_refresh(editor.view);
				running = true;
				INPUT_RECORD input_record;
				DWORD events_read;
				while (running && ReadConsoleInput(input_handle, &input_record, 1, &events_read)) {
					switch (input_record.EventType) {
					case KEY_EVENT: {
						handle_key_event(input_record.Event.KeyEvent);
					} break;
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
