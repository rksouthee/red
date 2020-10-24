#include <Windows.h>
#include <cassert>
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

	screen_state[screen_y * screen_width + screen_x] = c;

	++screen_x;
	if (screen_x == screen_width) {
		++screen_y;
		screen_x = 0;
	}
}

static void screen_putstring(const char* str)
{
	while (*str)
		screen_putchar(*str++);
}

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

int main()
{
	DWORD last_error = screen_initialize();

	if (last_error == 0) {
		screen_putstring("Hello, world!");
		screen_update();
		Sleep(1000);
	} else {
		OutputDebugStringA("Failed to initialize screen buffer\n");
	}

	return last_error;
}
