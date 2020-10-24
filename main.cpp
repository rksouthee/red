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
static std::string text_buffer =
R"(Particular unaffected projection sentiments
no my. Music marry as at cause party worth weeks. Saw how marianne graceful
dissuade new outlived prospect followed. Uneasy no settle whence nature narrow
in afraid. At could merit by keeps child. While dried maids on he of linen in.

Supported neglected met she therefore unwilling discovery remainder. Way
sentiments two indulgence uncommonly own. Diminution to frequently sentiments
he connection continuing indulgence. An my exquisite conveying up defective.
Shameless see the tolerably how continued. She enable men twenty elinor points
appear. Whose merry ten yet was men seven ought balls.

Cottage out enabled was entered greatly prevent message. No procured unlocked
an likewise. Dear but what she been over gay felt body. Six principles
advantages and use entreaties decisively. Eat met has dwelling unpacked see
whatever followed. Court in of leave again as am. Greater sixteen to forming
colonel no on be. So an advice hardly barton. He be turned sudden engage manner
spirit.

Started several mistake joy say painful removed reached end. State burst think
end are its. Arrived off she elderly beloved him affixed noisier yet. An course
regard to up he hardly. View four has said does men saw find dear shy. Talent
men wicket add garden.

Written enquire painful ye to offices forming it. Then so does over sent dull
on. Likewise offended humoured mrs fat trifling answered. On ye position
greatest so desirous. So wound stood guest weeks no terms up ought. By so these
am so rapid blush songs begin. Nor but mean time one over.

He unaffected sympathize discovered at no am conviction principles. Girl ham
very how yet hill four show. Meet lain on he only size.)";
static Text_position text_position;
static int text_column;
static int text_column_desired;

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

static void display_refresh()
{
	screen_x = 0;
	screen_y = 0;

	Text_position start = 0;
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
			OutputDebugStringA("Failed to initialize input\n");
		}
	} else {
		OutputDebugStringA("Failed to initialize screen buffer\n");
	}

	return last_error;
}
