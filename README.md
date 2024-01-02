# pipa language

This is a programming language whose goal is to bootstrap itself,
as in to write the compiler in the language itself.
Also, it should be more pleasant to write the compiler in the
new language.

## Features

Just enough to write a compiler.

* integers
* strings
* if statements
* loops
* structs
* functions
* compiles to assembly (gas)
* syntax style is pleasant and is like Python (to me)

## TODO

* ==, >=, <= operators (done)
* expr
    * parse string (done)
    * parse nested fun calls (done)
    * binary operations (done)
        * precedence (done)
* parser error diagnostics
    * maybe EOF needs to be a token so that we can use same logic to point to its location in file? Although we can also figure it out with file content (done)
    * if tried multiple different paths and both failed, should report the one that got further
    * show parse context, which probably should be a stack, but not clear how much of
    the stack to display, doesn't hurt to collect them all just in case
    * implement display of the token location with source snippet (done)
* if statements (done)
* loops (done)
* structs
* function definitions
