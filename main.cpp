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
				return;
			++rows;
		}

		do {
			view.top_line = find_forward(*view.buffer, view.top_line, '\n') + 1;
			if (view.top_line == line)
				return;
			--rows;
		} while (rows > 0);

		view.top_line = cursor_line;
	}
}

static std::string display_state;

static void display_refresh(View& view)
{
	reframe(view);

	display_state.clear();
	display_state.resize(view.width * view.height, ' ');

	int cursor_row = 0;
	int cursor_column = 0;

	Text_position cursor = view.top_line;
	for (int row = 0; row < view.height; ++row) {
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

int main()
{
	DWORD last_error = screen_initialize();

	if (last_error == 0) {
		Screen_dimension size = screen_dimension();
		Text_buffer buffer;
		View view;
		view.buffer = &buffer;
		view.width = size.width;
		view.height = size.height - 1;
		view.top_line = 0;
		last_error = input_initialize();
		if (last_error == 0) {
			last_error = file_open("lipsum.txt", buffer);
			if (last_error == 0) {
				display_refresh(view);
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
								move_left(buffer);
								break;
							case 'j':
								move_down(buffer);
								break;
							case 'k':
								move_up(buffer);
								break;
							case 'l':
								move_right(buffer);
								break;
							}
							display_refresh(view);
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
