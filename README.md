# red

A text editor for experimenting with modal editing.

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
