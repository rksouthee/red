#ifndef RED_EDITOR_H
#define RED_EDITOR_H

#include "buffer.h"

struct View {
	Buffer* buffer;
	int width;
	int height;
	Buffer::iterator cursor;
	Buffer::iterator top_line;
	int first_column;
	int column_desired;
};

struct Editor_state {
	Buffer buffer;
	View view;
};

#endif
