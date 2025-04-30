**Compiling and running a Flex scanner**

Let’s say our scanner specification lives in the file counter.l.  We can generate C/C++ code implementing our scanner using the flex command in the terminal.  For example, to generate a file counter.c from our specification we would run a command like this (the -o option specifies the output file):

flex -o counter.c counter.l

Since our scanner specification’s user code section contains a main() function, counter.c can be compiled directly into an executable scanner:

gcc counter.c -o counter

By default, yylex() reads input from stdin, so to run our scanner on a source file, we can use input redirection.  For example, we could run our counting scanner on its own specification like this:

./counter < counter.l

**Compiling and running a Bison parser**

Let’s say our scanner specification lives in the file scanner.l and the parser specification lives in the file parser.y, We can generate C/C++ code implementing our  parser using the following sequence of commands in the terminal.

bison -d -o parser.c parser.y

flex -o scanner.c scanner.l

Since our parser  specification’s user code section contains a main() function, parser.c and scanner.c can be compiled directly into an executable scanner:

g++ parser.c scanner.c -o parser

We could run our parser on the example input file source.py that contains python assignment statements.

./parser < source.py
