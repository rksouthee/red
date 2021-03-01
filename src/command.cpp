#include <cassert>
#include <algorithm>
#include "command.h"
#include "prompt.h"
#include "display.h"
#include "file.h"
#include "utility.h"
#include "screen.h"

static Command_function normal_mode[MAX_KEYS];
static Command_function ctrlx_mode[MAX_KEYS];

static unsigned key_to_index(unsigned index)
{
	assert(index < MAX_KEYS);
	return index;
}

COMMAND_FUNCTION(ctrlx_command)
{
	Key_input new_input = wait_for_key();
	unsigned index = key_to_index(new_input.key);
	Command_function cmd = ctrlx_mode[index];
	cmd(editor, new_input, should_exit);
}

static void normal_mode_initialize()
{
	for (int i = 0; i < MAX_KEYS; ++i) {
		normal_mode[i] = none;
		ctrlx_mode[i] = none;
	}

	normal_mode['H'] = backward_char;
	normal_mode['J'] = forward_line;
	normal_mode['K'] = backward_line;
	normal_mode[VK_HOME] = goto_beginning_of_line;
	normal_mode[control(VK_HOME)] = goto_beginning_of_file;
	normal_mode[VK_END] = goto_end_of_line;
	normal_mode[control(VK_END)] = goto_end_of_file;
	normal_mode['L'] = forward_char;
	normal_mode['W'] = forward_word;
	normal_mode['I'] = start_insert_mode;
	normal_mode[VK_OEM_2] = search_forward;

	normal_mode[control('X')] = ctrlx_command;
	ctrlx_mode[control('S')] = write_file;
	ctrlx_mode[control('C')] = quit;
	ctrlx_mode[control('F')] = find_file;
}

static Command_function insert_mode[MAX_KEYS];

static void insert_mode_initialize()
{
	for (int i = 0; i < MAX_KEYS; ++i) {
		insert_mode[i] = insert_self;
	}

	insert_mode[VK_ESCAPE] = leave_insert_mode;
	insert_mode[VK_RETURN] = insert_newline;
	insert_mode[VK_BACK] = backspace;
	insert_mode[control(0xDB)] = leave_insert_mode;
	insert_mode[VK_TAB] = insert_tab;
}

static Command_function* commands = normal_mode;

void commands_initialize()
{
	normal_mode_initialize();
	insert_mode_initialize();
}

bool evaluate(Editor_state& editor, const Key_input& input)
{
	unsigned index = key_to_index(input.key);
	Command_function cmd = commands[index];
	bool should_exit = false;
	assert(cmd);
	if (cmd != none) {
		cmd(editor, input, should_exit);
		display_refresh(editor.view);
	}
	return should_exit;
}

COMMAND_FUNCTION(none)
{
}

static void save_buffer(Editor_state& editor)
{
	if (editor.buffer.name.empty()) {
		std::string filename = prompt("Write file: ");
		if (filename.empty())
			return;
		editor.buffer.name = std::move(filename);
	}

	DWORD last_error = file_save(editor.buffer);
	if (last_error == 0) {
		set_status_line("Wrote file");
	} else {
		set_status_line("Error writing file");
	}
}

COMMAND_FUNCTION(write_file)
{
	save_buffer(editor);
}

COMMAND_FUNCTION(find_file)
{
	std::string filename = prompt("Find file: ");
	if (filename.empty())
		return;

	if (editor.buffer.modified) {
		User_response answer = prompt_yesno("Buffer modified. Leave anyway (y/n)? ");
		if (answer != User_response::yes)
			return;
	}

	DWORD last_error = file_open(filename.c_str(), editor.buffer);
	if (last_error != 0) {
		set_status_line("Error reading file");
		return;
	}

	assert(editor.view.buffer == &editor.buffer);
	editor.view.cursor = editor.buffer.begin();
	editor.view.top_line = editor.buffer.begin();
	editor.view.first_column = 0;
	editor.view.column_desired = 0;
}

COMMAND_FUNCTION(search_forward)
{
	std::string query = prompt("Search forward: ");
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

COMMAND_FUNCTION(forward_word)
{
	View& view = editor.view;
	if (view.cursor == view.buffer->end())
		return;

	const auto pred = [] (char x) -> bool { return std::isalpha(x); };
	Buffer::iterator iter = std::find_if_not(view.cursor, view.buffer->end(), pred);
	iter = std::find_if(iter, view.buffer->end(), pred);
	view.cursor = iter;
	view.column_desired = -1;
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

COMMAND_FUNCTION(goto_beginning_of_line)
{
	View& view = editor.view;
	view.cursor = find_backward(view.buffer->begin(), view.cursor, '\n');
	view.column_desired = 0;
}

COMMAND_FUNCTION(goto_beginning_of_file)
{
	View& view = editor.view;
	view.cursor = view.buffer->begin();
	view.column_desired = 0;
}

COMMAND_FUNCTION(goto_end_of_line)
{
	View& view = editor.view;
	view.cursor = std::find(view.cursor, view.buffer->end(), '\n');
	view.column_desired = get_column(view.buffer->begin(), view.cursor);
}

COMMAND_FUNCTION(goto_end_of_file)
{
	View& view = editor.view;
	view.cursor = view.buffer->end();
	view.column_desired = get_column(view.buffer->begin(), view.cursor);
}

COMMAND_FUNCTION(quit)
{
	if (editor.buffer.modified) {
		User_response answer = prompt_yesno("Buffer modified. Leave anyway (y/n)? ");
		should_exit = answer == User_response::yes;
	} else {
		should_exit = true;
	}
}

COMMAND_FUNCTION(insert_self)
{
	char character = input.ascii;
	if (is_print(character)) {
		editor.buffer.insert(editor.view.cursor, character);
		++editor.view.cursor;
	}
}

COMMAND_FUNCTION(insert_newline)
{
	editor.buffer.insert(editor.view.cursor, '\n');
	++editor.view.cursor;
}

COMMAND_FUNCTION(backspace)
{
	View& view = editor.view;
	if (view.cursor != view.buffer->begin()) {
		--view.cursor;
		view.buffer->erase(view.cursor);
	}
}

COMMAND_FUNCTION(insert_tab)
{
	editor.buffer.insert(editor.view.cursor, '\t');
	++editor.view.cursor;
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
