**Compiling and running a Flex scanner**

Let’s say our scanner specification lives in the file counter.l.  We can generate C/C++ code implementing our scanner using the flex command in the terminal.  For example, to generate a file counter.c from our specification we would run a command like this (the -o option specifies the output file):

flex -o counter.c counter.l

Since our scanner specification’s user code section contains a main() function, counter.c can be compiled directly into an executable scanner:

gcc counter.c -o counter

By default, yylex() reads input from stdin, so to run our scanner on a source file, we can use input redirection.  For example, we could run our counting scanner on its own specification like this:

./counter < counter.l
