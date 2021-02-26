
#include <cassert>
#include <algorithm>
#include "command.h"
#include "prompt.h"
#include "display.h"
#include "file.h"
#include "utility.h"
#include "screen.h"

static Command_function normal_mode[MAX_KEYS];

static void normal_mode_initialize()
{
	for (int i = 0; i < MAX_KEYS; ++i) {
		normal_mode[i] = command_none;
	}

	normal_mode['H'] = backward_char;
	normal_mode['J'] = forward_line;
	normal_mode['K'] = backward_line;
	normal_mode['L'] = forward_char;
	normal_mode['I'] = start_insert_mode;
	normal_mode[control('S')] = save;
	normal_mode[control('Q')] = quit;
	normal_mode[control('O')] = open;
	normal_mode[control('F')] = search;
}

static Command_function insert_mode[MAX_KEYS];

static void insert_mode_initialize()
{
	for (int i = 0; i < MAX_KEYS; ++i) {
		insert_mode[i] = command_self_insert;
	}

	insert_mode[VK_ESCAPE] = leave_insert_mode;
	insert_mode[VK_RETURN] = command_newline;
	insert_mode[VK_BACK] = command_backspace;
	insert_mode[control(0xDB)] = leave_insert_mode;
}

static Command_function* commands = normal_mode;

void commands_initialize()
{
	normal_mode_initialize();
	insert_mode_initialize();
}

bool evaluate(Editor_state& editor, const Key& key)
{
	assert(key.code >= 0 && key.code < 256);
	unsigned index = key.code;
	index |= key.ctrl << 8;
	index |= key.alt << 9;
	index |= key.shift << 10;
	assert(index >= 0 && index < MAX_KEYS);

	Command_function cmd = commands[index];
	assert(cmd);
	bool should_exit = false;
	cmd(editor, key, should_exit);
	display_refresh(editor.view);
	return should_exit;
}

COMMAND_FUNCTION(command_none)
{
}

static void save_buffer(Editor_state& editor)
{
	if (editor.buffer.filename().empty()) {
		std::string filename = prompt("enter filename: ");
		if (filename.empty()) {
			set_status_line("buffer not saved");
			return;
		}
		editor.buffer.filename(std::move(filename));
	}

	DWORD last_error = file_save(editor.buffer);
	if (last_error == 0) {
		set_status_line("Saved buffer");
	} else {
		set_status_line("Error saving buffer");
	}
}

COMMAND_FUNCTION(save)
{
	save_buffer(editor);
}

COMMAND_FUNCTION(open)
{
	std::string filename = prompt("open file: ");
	if (filename.empty())
		return;

	if (editor.buffer.modified()) {
		User_response answer = prompt_yesno("save before leaving? (y/n)");
		if (answer == User_response::cancel)
			return;
		if (answer == User_response::yes)
			save_buffer(editor);
	}

	DWORD last_error = file_open(filename.c_str(), editor.buffer);
	if (last_error != 0) {
		set_status_line("Error loading file");
		return;
	}

	assert(editor.view.buffer == &editor.buffer);
	editor.view.cursor = editor.buffer.begin();
	editor.view.top_line = editor.buffer.begin();
	editor.view.first_column = 0;
	editor.view.column_desired = 0;
}

COMMAND_FUNCTION(search)
{
	std::string query = prompt("search: ");
	View& view = editor.view;
	auto iter = std::search(view.cursor, view.buffer->end(), query.begin(), query.end());
	if (iter != view.buffer->end())
		view.cursor = iter;
}

COMMAND_FUNCTION(forward_char)
{
	View& view = editor.view;
	if (view.cursor != view.buffer->end()) {
		++view.cursor;
		view.column_desired = -1;
	}
}

COMMAND_FUNCTION(backward_char)
{
	View& view = editor.view;
	if (view.cursor != view.buffer->begin()) {
		--view.cursor;
		view.column_desired = -1;
	}
}

static int get_column(Buffer::iterator first, Buffer::iterator last)
{
	first = find_backward(first, last, '\n');
	int column = 0;
	while (first != last) {
		if (*first == '\t')
			column += 8 - (column & 0x7);
		else
			++column;
		++first;
	}
	return column;
}

static Buffer::iterator set_column(Buffer::iterator first, Buffer::iterator last, int column)
{
	int current = 0;
	while (first != last && *first != '\n' && current < column) {
		if (*first == '\t')
			current += 8 - (current & 0x7);
		else
			++current;
		++first;
	}
	return first;
}

COMMAND_FUNCTION(forward_line)
{
	View& view = editor.view;
	if (view.column_desired == -1)
		view.column_desired = get_column(view.buffer->begin(), view.cursor);
	Buffer::iterator end_of_line = std::find(view.cursor, view.buffer->end(), '\n');
	if (end_of_line != view.buffer->end())
		view.cursor = set_column(std::next(end_of_line), view.buffer->end(), view.column_desired);
}

COMMAND_FUNCTION(backward_line)
{
	View& view = editor.view;
	if (view.column_desired == -1)
		view.column_desired = get_column(view.buffer->begin(), view.cursor);
	view.cursor = find_backward(view.buffer->begin(), view.cursor, '\n');
	if (view.cursor != view.buffer->begin()) {
		Buffer::iterator start_of_line = find_backward(view.buffer->begin(), std::prev(view.cursor), '\n');
		view.cursor = set_column(start_of_line, view.buffer->end(), view.column_desired);
	}
}

COMMAND_FUNCTION(quit)
{
	if (editor.buffer.modified()) {
		User_response answer = prompt_yesno("quit (y/n)");
		should_exit = answer == User_response::yes;
	} else {
		should_exit = true;
	}
}

COMMAND_FUNCTION(command_self_insert)
{
	char character = key.ascii;
	if (is_print(character)) {
		editor.buffer.insert(editor.view.cursor, character);
		++editor.view.cursor;
	}
}

COMMAND_FUNCTION(command_newline)
{
	editor.buffer.insert(editor.view.cursor, '\n');
	++editor.view.cursor;
}

COMMAND_FUNCTION(command_backspace)
{
	View& view = editor.view;
	if (view.cursor != view.buffer->begin()) {
		--view.cursor;
		view.buffer->erase(view.cursor);
	}
}

COMMAND_FUNCTION(start_insert_mode)
{
	commands = insert_mode;
	set_status_line("--INSERT--");
	screen_cursor_style(Cursor_style::underline);
}

COMMAND_FUNCTION(leave_insert_mode)
{
	commands = normal_mode;
	set_status_line("");
	screen_cursor_style(Cursor_style::block);
	editor.view.column_desired = -1;
}

