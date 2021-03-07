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

A command such as `^x` means pressing Ctrl and x at the same time.

| Key | Command |
| --- | ------- |
| l | Forward character |
| h | Backward character |
| w | Next word |
| b | Previous word |
| 0 | Beginning of line |
| Home | Beginning of line |
|^Home | Beginning of file |
| $ | End of line |
| End | End of line |
| ^End | End of file |
| j | Next line |
| k | Previous line |
| i | Insert before cursor |
| I | Insert at beginning of line |
| a | Insert after cursor |
| A | Insert at end of line |
| o | Open line below |
| O | Open line above |
| S | Replace line |
| D | Delete to end of line |
| dd | Delete line |
| / | Search forward |
| ^e | Scroll down |
| ^y | Scroll up |
| ^x ^s | Write file |
| ^x ^c | Quit |
| ^x ^f | Find file |

## Insert Commands

| Key | Command |
| --- | ------- |
| Esc | Leave insert mode |
| ^[  | Leave insert mode |
| ^t  | Indent line |
| ^d  | Deindent line |

## Todo

### Undo/Redo

Support undo/redo operations. Keep a record of the modifications, including
cursor positioning.

### Cut/Copy/Paste

Interact with Windows clipboard so support cut, copy and paste. Be aware of
encoding issues.
