# zerotape

*Work In Progress*

zerotape is a small C library for capturing hierarchical data structures to a text file format, then restoring them later. You take the C data structures you want to preserve and write out a description of the structures. zerotape then gives you functions to save and load the structure to and from its standard file format.

- zerotape’s file format is a small language: it allows basic expressions
- It's integer oriented at the moment (no support for strings or floating point types, yet)
- It's assignment focused (although it's a small language it has no variables yet, or references to symbols, etc.)

zerotape uses the [lemon parser generator](https://sqlite.org/src/doc/trunk/doc/lemon.html). It was built to handle the saving and loading of games into my [The Great Escape in C](https://github.com/dpt/The-Great-Escape-in-C) project.

## File Format

Quick overview:

```
// zerotape has C++ style comments

field = 42;  // assign a single field
array = [ 42, 43 ];  // assign an array
sparsearray = [ 0: 42, 10: 43 ];  // assign numbered elements within an array
outerfield = { innerfield = 42; }  // nesting/scoped fields

// Values can be decimal, $hex or 0xhex
field = 1.23;
field = $deadbeef;
field = 0xcafebabe;

// Values can use the basic expressions +, -, * and /
field = ($7f * 2);
```

## Probable Next Stages

- Write a preprocessor to build the structure descriptions by scanning the C structure definitions for specially marked-up structures.
