#ifndef RED_COMMAND_H
#define RED_COMMAND_H

#include "editor.h"
#include "input.h"

#define COMMAND_FUNCTION(name) void name(Editor_state& editor, const Key_input& input, bool& should_exit)
typedef COMMAND_FUNCTION((*Command_function));

/*
 * Using the Microsoft convention for a key code. The low-order byte contains the virtual-key code and
 * the high-order byte contains the shift state, which can be a combination of the following
 * 1 Either SHIFT key is pressed
 * 2 Either CTRL key is pressed
 * 4 Either ALT key is pressed
 */
#define MAX_KEYS (1 << (8 + 3))
#define CONTROL_SHIFT (1 << 9)

inline unsigned control(unsigned key_code)
{
	return key_code | CONTROL_SHIFT;
}

void commands_initialize();
bool evaluate(Editor_state& editor, const Key_input& input);

COMMAND_FUNCTION(none);

COMMAND_FUNCTION(backward_char);
COMMAND_FUNCTION(forward_line);
COMMAND_FUNCTION(backward_line);
COMMAND_FUNCTION(goto_beginning_of_line);
COMMAND_FUNCTION(goto_beginning_of_file);
COMMAND_FUNCTION(goto_end_of_line);
COMMAND_FUNCTION(goto_end_of_file);
COMMAND_FUNCTION(forward_char);
COMMAND_FUNCTION(forward_word);
COMMAND_FUNCTION(insert_before_cursor);
COMMAND_FUNCTION(insert_before_line);
COMMAND_FUNCTION(insert_after_cursor);
COMMAND_FUNCTION(insert_after_line);
COMMAND_FUNCTION(leave_insert_mode);
COMMAND_FUNCTION(write_file);
COMMAND_FUNCTION(quit);
COMMAND_FUNCTION(find_file);
COMMAND_FUNCTION(search_forward);
COMMAND_FUNCTION(insert_self);
COMMAND_FUNCTION(insert_newline);
COMMAND_FUNCTION(backspace);
COMMAND_FUNCTION(insert_tab);
COMMAND_FUNCTION(ctrlx_command);
COMMAND_FUNCTION(open_line_after);
COMMAND_FUNCTION(open_line_before);
COMMAND_FUNCTION(delete_line);

#endif
