# red

Red is a text editor for the Windows operating system using the command line interface.

![Welcome](welcome.png)

Like Vim, Red is designed as a modal editor, this means that Red is keyboard
focused, although I hope to add mouse support in the future.

## Building

Open the x64 Native Tools Command Prompt for VS 2019 and in the command prompt, execute
the command:

```sh
> nmake /nologo
```

## Using

Red is to be used on the command line, and requires a file to open or create as an argument.

```sh
red hello.txt
```

There are currently two modes supported, Normal and Insert. The editor initially starts in Normal mode
where commands can be entered.

### Normal Commands

| Key | Command | Description |
| --- | ------- | ----------- |
| H   | backward char | Moves the cursor to the previous character. Wraps to end of previous line if at the beginning |
| J   | forward line | Moves the cursor to next line. Does nothing when on the last line |
| K   | backward line | Moves the cursor to the previous line. Moves to the beginning when on the first line |
| L   | forward char | Moves the cursor forward a character. Wraps to the next line if at the end of a line |
| C-S | save file | Saves the current file. (Control-S) |
| C-Q | quit | Exits the application. Red will not exit if there are unsaved changes |

## Insert Commands

| Key | Command | Description |
| --- | ------- | ----------- |
| Esc | leave insert | Exits insert mode and returns to normal mode |
| Enter | newline | Inserts a newline character into the buffer |
| Backspace | backspace | Erases the previous character |

## Future work

* Different data structures for handling the text buffer such as a gap buffer, linked list of
lines, rope, etc.
* Regular expressions and searching.
* Modal editing workflow
* Syntax highlighting
* Line wrap, word wrap.
