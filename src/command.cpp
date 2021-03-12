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
	{ VkKeyScanA('b'), backward_word },
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
	{ VkKeyScanA('D'), delete_to_end_of_line },
	{ VkKeyScanA('/'), search_forward },
	{ VK_HOME, goto_beginning_of_line },
	{ CONTROL | VK_HOME, goto_beginning_of_file },
	{ VK_END, goto_end_of_line },
	{ CONTROL | VK_END, goto_end_of_file },
	{ CONTROL | VkKeyScanA('x'), ctrlx_command },
	{ VkKeyScanA('0'), goto_beginning_of_line },
	{ VkKeyScanA('$'), goto_end_of_line },
	{ VkKeyScanA('S'), replace_line },
	{ CONTROL | VkKeyScanA('e'), scroll_down },
	{ CONTROL | VkKeyScanA('y'), scroll_up },
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
		iter->cmd(editor, new_input, should_exit, 0);
}

static Bind insert_binds[] = {
	{ VK_ESCAPE, leave_insert_mode },
	{ CONTROL | VkKeyScanA('['), leave_insert_mode },
	{ VK_RETURN, insert_newline },
	{ VK_TAB, insert_tab },
	{ CONTROL | VkKeyScanA('i'), insert_tab },
	{ VK_BACK, backspace },
	{ CONTROL | VkKeyScanA('t'), indent_line },
	{ CONTROL | VkKeyScanA('d'), deindent_line },
};

static Bind delete_binds[] = {
	{ VkKeyScanA('d'), delete_line },
};

bool evaluate(Editor_state& editor, Key_input input)
{
	int count = 0;
	if (input.ascii >= '1' && input.ascii <= '9') {
		count = (input.ascii - '0');
		while (true) {
			input = wait_for_key();
			if (input.ascii >= '0' && input.ascii <= '9') {
				// TODO: guard against overflow
				count *= 10;
				count += (input.ascii - '0');
			} else {
				break;
			}
		}
	}

	auto iter = std::find_if(std::begin(normal_binds), std::end(normal_binds), [&input] (const Bind& bind) -> bool {
		return bind.key == input.key;
	});
	bool should_exit = false;
	if (iter != std::end(normal_binds)) {
		iter->cmd(editor, input, should_exit, count);
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
		do {
			++view.cursor;
		} while (count-- && view.cursor != view.buffer->end());
		view.column_desired = -1;
	}
}

COMMAND_FUNCTION(forward_word)
{
	View& view = editor.view;
	const auto pred = [] (char x) -> bool { return std::isalnum(x); };
	view.cursor = std::find_if_not(view.cursor, view.buffer->end(), pred);
	view.cursor = std::find_if(view.cursor, view.buffer->end(), pred);
	view.column_desired = -1;
}

COMMAND_FUNCTION(backward_char)
{
	if (count == 0)
		count = 1;
	View& view = editor.view;
	if (view.cursor != view.buffer->begin()) {
		do {
			--view.cursor;
		} while (--count && view.cursor != view.buffer->begin());
		view.column_desired = -1;
	}
}

template <typename I, typename P>
// requires BidirectionalIterator(I) && UnaryPredicate(P, ValueType(I))
I find_if_backward(I f, I l, P p)
{
	while (l != f) {
		--l;
		if (p(*l))
			return std::next(l);
	}
	return f;
}

template <typename I, typename P>
// requires BidirectionalIterator(I) && UnaryPredicate(P, ValueType(I))
I find_if_not_backward(I f, I l, P p)
{
	while (l != f) {
		--l;
		if (!p(*l))
			return std::next(l);
	}
	return f;
}

COMMAND_FUNCTION(backward_word)
{
	View& view = editor.view;
	const auto pred = [] (char x) -> bool { return std::isalnum(x); };
	view.cursor = ::find_if_backward(view.buffer->begin(), view.cursor, pred);
	view.cursor = ::find_if_not_backward(view.buffer->begin(), view.cursor, pred);
	view.column_desired = -1;
}

/*
 * get_column
 *
 * Finds the 0-based value of the column of the line that `last` points to.
 * `first` could point to the beginning of the buffer to prevent going out of
 * range.  When counting the column we take into consideration tabs, a tab
 * character rounds the column to the next multiple of 8. Other characters in
 * the buffer are considered to have a width of 1.
 */
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

/*
 * set_column
 *
 * Returns the iterator that would point to the desired `column` on the given
 * line starting at `first`. We need want to make sure that we find the column
 * for the current line and that we don't go out bounds. Similar to
 * `get_column` tabs round to the next multiple of 8, and other characters have
 * a width of 1.
 */
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
	View& view = editor.view;
	Buffer::iterator start_of_line = ::find_backward(view.buffer->begin(), view.cursor, '\n');
	int tab_count = 0;
	while (start_of_line != view.buffer->end() && *start_of_line == '\t') {
		++tab_count;
		++start_of_line;
	}

	view.buffer->insert(view.cursor, '\n');
	++editor.view.cursor;

	while (tab_count) {
		view.buffer->insert(view.cursor, '\t');
		++view.cursor;
		--tab_count;
	}
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
			insert_self(editor, input, should_exit, 0);
		} else if (iter->cmd == leave_insert_mode) {
			iter->cmd(editor, input, should_exit, 0);
			break;
		} else {
			iter->cmd(editor, input, should_exit, 0);
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

COMMAND_FUNCTION(delete_to_end_of_line)
{
	View& view = editor.view;
	Buffer::iterator line_end = std::find(view.cursor, view.buffer->end(), '\n');
	view.buffer->erase(view.cursor, line_end);
}

COMMAND_FUNCTION(delete_line)
{
	View& view = editor.view;
	Buffer::iterator line_begin = find_backward(view.buffer->begin(), view.cursor, '\n');
	Buffer::iterator line_end = std::find(view.cursor, view.buffer->end(), '\n');
	if (line_end != view.buffer->end())
		++line_end;
	view.cursor = view.buffer->erase(line_begin, line_end);
	assert(view.cursor == view.buffer->begin() || *(std::prev(view.cursor)) == '\n');
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

COMMAND_FUNCTION(deindent_line)
{
	View& view = editor.view;
	Buffer::iterator line_begin = ::find_backward(view.buffer->begin(), view.cursor, '\n');
	if (line_begin != view.buffer->end() && *line_begin == '\t') {
		view.buffer->erase(line_begin);
		--view.cursor;
		view.column_desired = -1;
	}
}

static void delete_mode(Editor_state& editor, bool& should_exit)
{
	Key_input input = wait_for_key();
	auto iter = std::find_if(std::begin(delete_binds), std::end(delete_binds), [&input] (const Bind& bind) -> bool {
		return bind.key == input.key;
	});

	if (iter != std::end(delete_binds)) {
		iter->cmd(editor, input, should_exit, 0);
	}
}

COMMAND_FUNCTION(start_delete_mode)
{
	delete_mode(editor, should_exit);
}

/*
 * scroll_down (similar to Vim command)
 *
 * We first count the number of lines between the line at the top of the screen
 * and the cursor position. We then attempt to move the top line forward by one
 * line, this might not be possible if we are on the last line of the file in
 * which case we don't need to do anything. Once we've advanced the top line we
 * then try to keep the cursor in the same position by moving n - 1 lines where
 * n is the number of lines we counted before.
 */
COMMAND_FUNCTION(scroll_down)
{
	using N = typename std::iterator_traits<Buffer::iterator>::difference_type;

	View& view = editor.view;
	N lines = std::count(view.top_line, view.cursor, '\n');
	view.top_line = std::find(view.top_line, view.buffer->end(), '\n');
	if (view.top_line == view.buffer->end())
		return;
	++view.top_line;
	view.cursor = view.top_line;

	for (N i = 0; i < lines - 1; ++i) {
		view.cursor = std::find(view.cursor, view.buffer->end(), '\n');
		if (view.cursor == view.buffer->end())
			break;
		++view.cursor;
	}
}

/*
 * scroll_up (similar to Vim command)
 *
 * We count the number of lines between the line at the top of screen and the
 * cursor position. We then move the top line to the previous line, (might not
 * be possible if we are on the first line of the file). Once we've moved the
 * top line, we want to maintain the cursor position so we we'll try to move
 * the cursor n + 1 lines where n is the number of lines we counted earlier.
 */
COMMAND_FUNCTION(scroll_up)
{
	using N = typename std::iterator_traits<Buffer::iterator>::difference_type;

	View& view = editor.view;
	N lines = std::count(view.top_line, view.cursor, '\n');
	view.top_line = ::find_backward(view.buffer->begin(), view.top_line, '\n');
	if (view.top_line == view.buffer->begin())
		return;
	--view.top_line;
	view.top_line = ::find_backward(view.buffer->begin(), view.top_line, '\n');
	view.cursor = view.top_line;

	// Make sure we stay on the screen
	lines = std::min(static_cast<N>(view.height - 1), lines + 1);

	for (N i = 0; i < lines; ++i) {
		view.cursor = std::find(view.cursor, view.buffer->end(), '\n');
		if (view.cursor == view.buffer->end())
			break;
		++view.cursor;
	}
}
