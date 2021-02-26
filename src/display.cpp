#include "display.h"
#include "utility.h"
#include "screen.h"

static void reframe(View& view)
{
	Buffer::iterator cursor_line = find_backward(view.buffer->begin(), view.cursor, '\n');
	if (cursor_line <= view.top_line) {
		view.top_line = cursor_line;
	} else {
		int rows = 1;
		Buffer::iterator line = cursor_line;
		while (rows < view.height) {
			line = find_backward(view.buffer->begin(), std::prev(line), '\n');
			if (line == view.top_line)
				goto done;
			++rows;
		}

		do {
			view.top_line  = std::find(view.top_line, view.buffer->end(), '\n');
			++view.top_line;
			if (view.top_line == line)
				goto done;
			--rows;
		} while (rows > 0);

		view.top_line = cursor_line;
	}

done:
	int column = 0;
	while (cursor_line != view.cursor) {
		if (*view.cursor == '\t') {
			column += 8 - (column & 0x7);
		} else {
			++column;
		}
		++cursor_line;
	}
	if (column < view.first_column) {
		view.first_column = column;
	} else if (view.first_column + view.width <= column) {
		view.first_column = column - view.width + 1;
	}
}

static std::string display_state;

void display_refresh(View& view)
{
	reframe(view);

	display_state.clear();
	display_state.resize(view.width * view.height, ' ');

	int cursor_row = 0;
	int cursor_column = 0;

	Buffer::iterator cursor = view.top_line;
	for (int row = 0; row < view.height; ++row) {
		// advance to the correct column
		int column = 0;
		while (column < view.first_column) {
			if (cursor == view.buffer->end())
				break;
			if (*cursor == '\n')
				break;
			if (*cursor == '\t')
				column += 8 - (column & 0x7);
			else
				++column;
			++cursor;
		}

		for (int width = 0; width < view.width;) {
			if (cursor == view.cursor) {
				cursor_row = row;
				cursor_column = width;
			}

			if (cursor == view.buffer->end())
				goto done;

			char ch = *cursor;
			if (ch == '\n')
				break;

			if (ch == '\t') {
				int nspaces = 8 - (column & 0x7);
				while (nspaces && width < view.width) {
					display_state[row * view.width + width] = ' ';
					--nspaces;
					++width;
					++column;
				}
			} else {
				display_state[row * view.width + width] = ch;
				++column;
				++width;
			}
			++cursor;
		}

		cursor = std::find(cursor, view.buffer->end(), '\n');
		if (cursor != view.buffer->end())
			++cursor;
	}

done:
	screen_cursor_visible(false);
	screen_cursor(0, 0);
	screen_putstring(display_state);
	screen_cursor(cursor_column, cursor_row);
	screen_cursor_visible(true);
}

int set_status_line(const char* str)
{
	Screen_dimension dimension = screen_dimension();
	screen_cursor(0, dimension.height - 1);
	screen_putstring(str);
	screen_clear_end_of_line();
	return static_cast<int>(std::strlen(str));
}

