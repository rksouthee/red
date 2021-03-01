#ifndef RED_INPUT_H
#define RED_INPUT_H

#include <Windows.h> // TODO: used for DWORD below, remove this

struct Key {
	int code;
	bool ctrl;
	bool shift;
	bool alt;
};

bool operator==(const Key& x, const Key& y);
bool operator!=(const Key& x, const Key& y);

bool operator <(const Key& x, const Key& y);
bool operator >(const Key& x, const Key& y);
bool operator<=(const Key& x, const Key& y);
bool operator>=(const Key& x, const Key& y);

struct Key_input {
	Key key;
	char ascii;
};

DWORD input_initialize();

Key_input wait_for_key();

#endif
