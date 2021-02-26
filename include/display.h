#ifndef RED_DISPLAY_H
#define RED_DISPLAY_H

#include "editor.h"
#include <string_view>

void display_refresh(View& view);

void set_status_line(std::string_view str);

#endif
