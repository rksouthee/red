#ifndef RED_INPUT_H
#define RED_INPUT_H

#include <Windows.h> // TODO: used for DWORD below, remove this

struct Key {
	int code;
	bool ctrl;
	bool shift;
	bool alt;
	char ascii;
};

DWORD input_initialize();

Key wait_for_key();

#endif
