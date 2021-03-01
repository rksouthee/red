#ifndef RED_INPUT_H
#define RED_INPUT_H

#include <Windows.h> // TODO: used for DWORD below, remove this


struct Key_input {
	unsigned key;
	char ascii;
};

DWORD input_initialize();

Key_input wait_for_key();

#endif
