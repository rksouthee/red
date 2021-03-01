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

bool operator==(const Key& x, const Key& y)
{
	return x.code == y.code && x.ctrl == y.ctrl && x.shift == y.shift && x.alt == y.alt;
}

bool operator!=(const Key& x, const Key& y)
{
	return !(x == y);
}

bool operator <(const Key& x, const Key& y)
{
	if (x.code < y.code)
		return true;
	if (y.code < x.code)
		return false;
	if (x.ctrl < y.ctrl)
		return true;
	if (y.ctrl < x.ctrl)
		return false;
	if (x.shift < y.shift)
		return true;
	if (y.shift < x.shift)
		return false;
	if (x.alt < y.alt)
		return true;
	if (y.alt < x.alt)
		return false;
	return false;
}

bool operator >(const Key& x, const Key& y)
{
	return y < x;
}

bool operator<=(const Key& x, const Key& y)
{
	return !(y < x);
}

bool operator>=(const Key& x, const Key& y)
{
	return !(x < y);
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
		key_input.key.code = key_event.wVirtualKeyCode;
		key_input.key.ctrl = (key_event.dwControlKeyState & (LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED)) != 0;
		key_input.key.shift = (key_event.dwControlKeyState & SHIFT_PRESSED) != 0;
		key_input.key.alt = (key_event.dwControlKeyState & (LEFT_ALT_PRESSED | RIGHT_ALT_PRESSED)) != 0;
		key_input.ascii = key_event.uChar.AsciiChar;
		return key_input;
	}
	// TODO: handle ReadConsoleInput failure
	return {};
}

