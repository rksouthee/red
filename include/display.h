#ifndef RED_DISPLAY_H
#define RED_DISPLAY_H

#include "editor.h"

void display_refresh(View& view);

// TODO: Use string_view instead
int set_status_line(const char* str);

#endif
