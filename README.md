# pipa language

This is a programming language whose goal is to bootstrap itself,
as in to write the compiler in the language itself.
Also, it should be more pleasant to write the compiler in the
new language.

## Features

Just enough to write a compiler.

* integers
* strings
* string formatters
* if statements
* loops
* structs
* tagged unions??
* pointers?
* functions
* python-style indent-based blocks
* minimal commas?
* compiles to assembly (gas)
* maybe we write our own assembler
* no auto memory management (you use malloc)
* syntax style is pleasant and is like Python (to me)
* do we need modules? no
* iterators?
* protocols?

## TODO

* expr
    * parse string (done)
    * parse nested fun calls (done)
    * binary operations (done)
        * precedence (done)
* parser error diagnostics
* if statements
* loops
* structs
* function definitions
* maybe make generic linked list
