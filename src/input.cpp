#include "input.h"
#include <Windows.h>

static HANDLE input_handle;

DWORD input_initialize()
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

Key_input wait_for_key()
{
	INPUT_RECORD input;
	DWORD read;
	while (ReadConsoleInput(input_handle, &input, 1, &read)) {
		if (input.EventType != KEY_EVENT)
			continue;
		const KEY_EVENT_RECORD& key_event = input.Event.KeyEvent;
		if (!key_event.bKeyDown)
			continue;

		Key_input key_input;
		key_input.key = key_event.wVirtualKeyCode;
		key_input.key |= ((key_event.dwControlKeyState & (LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED)) != 0) << 8;
		key_input.key |= ((key_event.dwControlKeyState & SHIFT_PRESSED) != 0) << 9;
		key_input.key |= ((key_event.dwControlKeyState & (LEFT_ALT_PRESSED | RIGHT_ALT_PRESSED)) != 0) << 10;
		key_input.ascii = key_event.uChar.AsciiChar;
		return key_input;
	}
	// TODO: handle ReadConsoleInput failure
	return {};
}

