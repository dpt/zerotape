# Zerotape

Zerotape is a C library for loading and saving data structures. It was built to handle the saving and loading of games into my The Great Escape in C project.

To use it, you take the C data structure that you want to preserve and write a second description of the structure’s fields using `ZT*` macros. You then call the zero tape functions to load and/or save the structure to and from zerotape format.

The zerotape format is flexible. It’s a small language which supports a number of basic language features, including assignments, structures and basic expressions. It’s parsed using a language grammar which is fed through the [Lemon](https://www.sqlite.org/lemon.html) parser generator.

Currently it has some limitations:
- It’s entirely integer focused (there are no strings or floating point types supported)
- It’s assignment focused (the language has no variables, or references to symbols, etc.)

## File Format

zerotape files can be written by hand. The following is an example of one:

```
// C++ style comments

field = 42;  // assign single field
array = [ 42, 43 ];  // assign array
sparsearray = [ 0: 42, 10: 43 ];  // assign elements within array
upperfield = { field = 42; }  // scoped fields

// Values can be decimal, $hex or 0xhex.
field = $deadbeef;
field = 0xcafebabe;

// Values can use the basic expressions: +, -, * and /.
field = ($7f * 2);
```

## Future ideas

- A sensible next step would be to write a preprocessor which allows specially marked-up structures to be written once, and to build the zerotape definitions by processing that.
