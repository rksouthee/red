#include <cassert>
#include <algorithm>
#include "command.h"
#include "prompt.h"
#include "display.h"
#include "file.h"
#include "utility.h"
#include "screen.h"

struct Bind {
	SHORT key;
	Command_function cmd;
};

static Bind normal_binds[] = {
	{ VkKeyScanA('h'), backward_char },
	{ VkKeyScanA('j'), forward_line },
	{ VkKeyScanA('k'), backward_line },
	{ VkKeyScanA('l'), forward_char },
	{ VkKeyScanA('w'), forward_word },
	{ VkKeyScanA('i'), insert_before_cursor },
	{ VkKeyScanA('I'), insert_before_line },
	{ VkKeyScanA('a'), insert_after_cursor },
	{ VkKeyScanA('A'), insert_after_line },
	{ VkKeyScanA('o'), open_line_after },
	{ VkKeyScanA('O'), open_line_before },
	{ VkKeyScanA('d'), start_delete_mode },
	{ VkKeyScanA('D'), delete_line },
	{ VK_OEM_2, search_forward },
	{ VK_HOME, goto_beginning_of_line },
	{ CONTROL | VK_HOME, goto_beginning_of_file },
	{ VK_END, goto_end_of_line },
	{ CONTROL | VK_END, goto_end_of_file },
	{ CONTROL | VkKeyScanA('x'), ctrlx_command },
	{ VkKeyScanA('0'), goto_beginning_of_line },
	{ VkKeyScanA('$'), goto_end_of_line },
	{ VkKeyScanA('S'), replace_line },
};

static Bind ctrlx_binds[] = {
	{ CONTROL | VkKeyScanA('s'), write_file },
	{ CONTROL | VkKeyScanA('c'), quit },
	{ CONTROL | VkKeyScanA('f'), find_file },
};

COMMAND_FUNCTION(ctrlx_command)
{
	Key_input new_input = wait_for_key();
	auto iter = std::find_if(std::begin(ctrlx_binds), std::end(ctrlx_binds), [&new_input] (const Bind& bind) -> bool {
		return bind.key == new_input.key;
	});
	if (iter != std::end(ctrlx_binds))
		iter->cmd(editor, new_input, should_exit);
}

static Bind insert_binds[] = {
	{ VK_ESCAPE, leave_insert_mode },
	{ CONTROL | VkKeyScanA('['), leave_insert_mode },
	{ VK_RETURN, insert_newline },
	{ VK_TAB, insert_tab },
	{ CONTROL | VkKeyScanA('i'), insert_tab },
	{ VK_BACK, backspace },
	{ CONTROL | VkKeyScanA('t'), indent_line },
};

static Bind delete_binds[] = {
	{ VkKeyScanA('d'), delete_line },
};

bool evaluate(Editor_state& editor, const Key_input& input)
{
	auto iter = std::find_if(std::begin(normal_binds), std::end(normal_binds), [&input] (const Bind& bind) -> bool {
		return bind.key == input.key;
	});
	bool should_exit = false;
	if (iter != std::end(normal_binds)) {
		iter->cmd(editor, input, should_exit);
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

static void insert_mode(Editor_state& editor, bool& should_exit)
{
	set_status_line("--INSERT--");
	screen_cursor_style(Cursor_style::underline);
	display_refresh(editor.view);

	while (true) {
		Key_input input = wait_for_key();
		auto iter = std::find_if(std::begin(insert_binds), std::end(insert_binds), [&input] (const Bind& bind) -> bool {
			return bind.key == input.key;
		});
		if (iter == std::end(insert_binds)) {
			insert_self(editor, input, should_exit);
		} else if (iter->cmd == leave_insert_mode) {
			iter->cmd(editor, input, should_exit);
			break;
		} else {
			iter->cmd(editor, input, should_exit);
		}
		display_refresh(editor.view);
	}
}

COMMAND_FUNCTION(insert_before_cursor)
{
	insert_mode(editor, should_exit);
}

COMMAND_FUNCTION(insert_before_line)
{
	View& view = editor.view;
	view.cursor = find_backward(view.buffer->begin(), view.cursor, '\n');
	insert_mode(editor, should_exit);
}

COMMAND_FUNCTION(insert_after_cursor)
{
	View& view = editor.view;
	if (view.cursor != view.buffer->end())
		++view.cursor;
	insert_mode(editor, should_exit);
}

COMMAND_FUNCTION(insert_after_line)
{
	View& view = editor.view;
	view.cursor = std::find(view.cursor, view.buffer->end(), '\n');
	insert_mode(editor, should_exit);
}

COMMAND_FUNCTION(leave_insert_mode)
{
	set_status_line("");
	screen_cursor_style(Cursor_style::block);
	editor.view.column_desired = -1;
}

COMMAND_FUNCTION(open_line_after)
{
	View& view = editor.view;
	view.cursor = std::find(view.cursor, view.buffer->end(), '\n');
	editor.buffer.insert(view.cursor, '\n');
	++view.cursor;
	insert_mode(editor, should_exit);
}

COMMAND_FUNCTION(open_line_before)
{
	View& view = editor.view;
	view.cursor = find_backward(view.buffer->begin(), view.cursor, '\n');
	editor.buffer.insert(view.cursor, '\n');
	insert_mode(editor, should_exit);
}

COMMAND_FUNCTION(delete_line)
{
	View& view = editor.view;
	Buffer::iterator line_begin = find_backward(view.buffer->begin(), view.cursor, '\n');
	Buffer::iterator line_end = std::find(view.cursor, view.buffer->end(), '\n');
	view.buffer->erase(line_begin, line_end);
	view.cursor = line_begin;
	view.column_desired = 0;
}

COMMAND_FUNCTION(replace_line)
{
	View& view = editor.view;
	Buffer::iterator line_begin = find_backward(view.buffer->begin(), view.cursor, '\n');
	Buffer::iterator line_end = std::find(view.cursor, view.buffer->end(), '\n');
	view.buffer->erase(line_begin, line_end);
	view.cursor = line_begin;
	insert_mode(editor, should_exit);
}

COMMAND_FUNCTION(indent_line)
{
	View& view = editor.view;
	Buffer::iterator line_begin = find_backward(view.buffer->begin(), view.cursor, '\n');
	view.buffer->insert(line_begin, '\t');
	++view.cursor;
	view.column_desired = -1;
}

static void delete_mode(Editor_state& editor, bool& should_exit)
{
	Key_input input = wait_for_key();
	auto iter = std::find_if(std::begin(delete_binds), std::end(delete_binds), [&input] (const Bind& bind) -> bool {
		return bind.key == input.key;
	});

	if (iter != std::end(delete_binds)) {
		iter->cmd(editor, input, should_exit);
	}
}

COMMAND_FUNCTION(start_delete_mode)
{
	delete_mode(editor, should_exit);
}
