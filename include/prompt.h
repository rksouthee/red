#ifndef RED_PROMPT_H
#define RED_PROMPT_H

#include <string>

enum User_response {
	yes, no, cancel
};

std::string prompt(const std::string& message);

User_response prompt_yesno(const char* message);

#endif
