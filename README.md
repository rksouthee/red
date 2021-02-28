# red

Red is a mimimalist text editor for the Windows operating system using the command line interface.

![Welcome](welcome.png)

[YouTube Demo](https://youtu.be/eNP3JyuvI1I)

## Building

Currently tested using Visual Studio 2019

```bat
red>md build
red>cmake-gui .
red>rem configure and generate build
red>cmake --build build
```

## Using

Red is to be used on the command line, and requires a file to open or create as an argument.

```sh
Usage: red <filename>
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
| W | forward word | Moves forward a word |
| ^X ^S | save | Saves the current file. |
| ^X ^C | quit | Exits the application. If unsaved changes use multiple times to force quit |
| ^X ^F | open | Opens a new file |
| / | search | Searches for a query in the buffer |
| Home | | Moves to the beginning of the line |
| ^Home | | Moves to the beginning of the file |
| End | | Moves to the end of the line |
| ^End | | Moves to the end of the file |

## Insert Commands

| Key | Command | Description |
| --- | ------- | ----------- |
| Esc | leave insert | Exits insert mode and returns to normal mode |
| Enter | newline | Inserts a newline character into the buffer |
| Backspace | backspace | Erases the previous character |
| Tab | insert tab | Inserts a tab character |

## Todo

### Undo/Redo

Support undo/redo operations. Keep a record of the modifications, including
cursor positioning.

### Cut/Copy/Paste

Interact with Windows clipboard so support cut, copy and paste. Be aware of
encoding issues.
