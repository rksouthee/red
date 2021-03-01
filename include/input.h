#ifndef RED_INPUT_H
#define RED_INPUT_H

#include <Windows.h> // TODO: used for DWORD below, remove this

#define SHIFT (1 << 8)
#define CONTROL (1 << 9)
#define ALT (1 << 10)

struct Key_input {
	SHORT key;
	char ascii;
};

DWORD input_initialize();

Key_input wait_for_key();

#endif
