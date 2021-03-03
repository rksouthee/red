#include <Windows.h>
#include <cassert>
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
#include "buffer.h"
#include "screen.h"
#include "input.h"
#include "file.h"
#include "display.h"
#include "utility.h"
#include "prompt.h"
#include "command.h"

#if 0
static void handle_window_buffer_size_event(Editor_state& editor, const WINDOW_BUFFER_SIZE_RECORD& size_event)
{
	/*
	 * When the console window is resized, if the height of the screen is increased
	 * windows introduces a vertical scrollbar, ideally we would like to remove the
	 * scrollbar but trying to handle it here results in the width of the screen buffer
	 * not matching the width of the window due to the scrollbar.
	 *
	 * Vim handles this by resizing the height when the window size changes, and when
	 * the user presses CTRL-L the width is adjusted (shortened) appropriately.
	 */
	UNREFERENCED_PARAMETER(size_event);
	Screen_dimension size = screen_dimension();
	editor.view.width = size.width;
	editor.view.height = size.height - 1;
	display_refresh(editor.view);
}
#endif

static void editor_initialize(Editor_state& editor)
{
	editor.buffer.modified = false;
	editor.view.buffer = &editor.buffer;
	Screen_dimension size = screen_dimension();
	editor.view.width = size.width;
	editor.view.height = size.height - 1;
	editor.view.cursor = editor.buffer.begin();
	editor.view.top_line = editor.buffer.begin();
	editor.view.first_column = 0;
	editor.view.column_desired = 0;
}

int main(int argc, char **argv)
{
	SetConsoleTitle("RED");
	DWORD last_error = screen_initialize();

	Editor_state editor;

	if (last_error == 0) {
		editor_initialize(editor);
		screen_cursor_style(Cursor_style::block);
		last_error = input_initialize();
		if (last_error == 0) {
			if (argc == 2)
				last_error = file_open(argv[1], editor.buffer);
			if (last_error == 0) {
				display_refresh(editor.view);
				while (true) {
					Key_input input = wait_for_key();
					if (evaluate(editor, input))
						break;
				}
			} else {
				OutputDebugStringA("Failed to load file\n");
			}
		} else {
			OutputDebugStringA("Failed to initialize input\n");
		}
	} else {
		OutputDebugStringA("Failed to initialize screen buffer\n");
	}


	// Restore the original title
	const DWORD title_size = 64 * 1024;
	char console_title[title_size];
	GetConsoleOriginalTitle(console_title, title_size);
	SetConsoleTitle(console_title);

	return last_error;
}
