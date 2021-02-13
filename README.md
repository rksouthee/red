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

## Future work

* Different data structures for handling the text buffer such as a gap buffer, linked list of
lines, rope, etc.
* Regular expressions and searching.
* Modal editing workflow
* Syntax highlighting
* Line wrap, word wrap.
