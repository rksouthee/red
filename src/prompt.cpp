#include <cassert>
#include "prompt.h"
#include "display.h"
#include "input.h"
#include "screen.h"
#include "utility.h"

User_response prompt_yesno(const char* message)
{
	User_response result;
	while (true) {
		set_status_line(message);
		Key response = wait_for_key();

		if (response.ascii == 'y' || response.ascii == 'Y') {
			result = User_response::yes;
			break;
		}

		if (response.ascii == 'n' || response.ascii == 'N') {
			result = User_response::no;
			break;
		}

		if (response.code == VK_ESCAPE) {
			result = User_response::cancel;
			break;
		}
	}

	set_status_line("");
	return result;
}

void render_prompt(int column, const std::string& prompt, std::string::size_type cursor)
{
	Screen_dimension dimension = screen_dimension();
	screen_cursor_visible(false);
	screen_cursor(column, dimension.height - 1);
	screen_putstring(prompt.c_str());
	screen_clear_end_of_line();
	screen_column(column + static_cast<int>(cursor));
	screen_cursor_visible(true);
}

std::string prompt(const std::string& message)
{
	std::string result;
	std::string::size_type position = 0;
	set_status_line(message);
	auto prompt_start = static_cast<int>(message.size());
	while (true) {
		assert(position <= result.size());
		render_prompt(prompt_start, result, position);
		Key key = wait_for_key();
		if (is_print(key.ascii)) {
			result.insert(position, 1, key.ascii);
			++position;
		} else if (key.code == VK_RETURN) {
			break;
		} else if (key.code == VK_LEFT) {
			if (position > 0)
				--position;
		} else if (key.code == VK_RIGHT) {
			if (position < result.size())
				++position;
		} else if (key.code == VK_BACK) {
			if (position > 0) {
				--position;
				result.erase(position, 1);
			}
		} else if (key.code == VK_ESCAPE || (key.code == 0xDB && key.ctrl)) {
			result.clear();
			break;
		}
	}
	set_status_line("");
	return result;
}

