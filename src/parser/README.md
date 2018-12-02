# Before compiling

```sh
sudo apt install flex bison
```

# Compile and test compiler

```sh
bison -d parser.y
flex lexer.l
g++ -o testp parser_test.cpp parser.tab.c lex.yy.c parser_base.cpp identprinter.cpp
```

test_parser.cpp WILL BE REPLACED