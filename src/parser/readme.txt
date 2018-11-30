Before compiling:
   sudo apt install flex bison

Compile compiler:
   bison -d parser.y
   flex lexer.l
   g++ -o testp test_parser.cpp parser.tab.c lex.yy.c parser_base.cpp



test_parser.cpp WILL BE REPLACED