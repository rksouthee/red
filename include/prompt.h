#ifndef RED_PROMPT_H
#define RED_PROMPT_H

#include <string_view>

enum User_response {
	yes, no, cancel
};

std::string prompt(std::string_view message);

User_response prompt_yesno(std::string_view message);

#endif
