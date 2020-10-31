# zerotape

zerotape is a C library for loading data structures from, and saving them to, a structured text format. It was built to handle the loading and saving of games in my (The Great Escape in C)[https://github.com/dpt/The-Great-Escape-in-C] project.

To use it, you take the C data structure that you want to preserve and write an additional description of the structure’s fields using `ZT*` macros. You then call the zerotape library functions to load and/or save the structure to and from zerotape format.

By default zerotape provides support for a basic set of integer types, but this can be supplemented with custom functions to handle more complex cases.

The zerotape format is flexible. It’s a small language which supports a number of basic language features, including assignments, structures and basic expressions. It’s parsed using a language grammar which is fed through the [Lemon](https://www.sqlite.org/lemon.html) parser generator.

Currently the language has some limitations:
- It’s entirely integer focused (there are no string or floating point types supported)
- It’s assignment focused (the language has no types, variables, or references to symbols, etc.)

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

## Future Ideas

- A sensible next step would be to write a preprocessor which allows specially marked-up structures to be written once, and to build both the zerotape definitions and the structures by processing that (yes, like protobufs).
