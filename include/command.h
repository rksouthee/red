#ifndef RED_COMMAND_H
#define RED_COMMAND_H

#include "editor.h"
#include "input.h"

#define COMMAND_FUNCTION(name) void name(Editor_state& editor, const Key& key, bool& should_exit)
typedef COMMAND_FUNCTION((*Command_function));

/*
 * 11 bits for key state
 * A9876543210
 * sackkkkkkkk
 * s = shift
 * a = alt
 * c = ctrl
 * k = key code
 */
#define MAX_KEYS (1 << (8 + 3))
#define CONTROL_SHIFT (1 << 8)

inline unsigned control(unsigned key_code)
{
	return key_code | CONTROL_SHIFT;
}

void commands_initialize();
bool evaluate(Editor_state& editor, const Key& key);

COMMAND_FUNCTION(command_none);

COMMAND_FUNCTION(backward_char);
COMMAND_FUNCTION(forward_line);
COMMAND_FUNCTION(backward_line);
COMMAND_FUNCTION(forward_char);
COMMAND_FUNCTION(start_insert_mode);
COMMAND_FUNCTION(leave_insert_mode);
COMMAND_FUNCTION(save);
COMMAND_FUNCTION(quit);
COMMAND_FUNCTION(open);
COMMAND_FUNCTION(search);
COMMAND_FUNCTION(command_self_insert);
COMMAND_FUNCTION(command_newline);
COMMAND_FUNCTION(command_backspace);

#endif
