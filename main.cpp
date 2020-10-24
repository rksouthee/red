#include <Windows.h>
#include <vector>

static HANDLE screen_handle;
static int screen_width;
static int screen_height;
static std::vector<char> screen_state;

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

int main()
{
	DWORD last_error = screen_initialize();

	if (last_error == 0) {
		Sleep(1000);
	} else {
		OutputDebugStringA("Failed to initialize screen buffer\n");
	}

	return last_error;
}
