cmake_minimum_required(VERSION 3.0)

set(CMAKE_C_COMPILER "g++")
set(CMAKE_CXX_COMPILER "g++")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(thu-rdbms)

set(PARSER_TARGETS src/parser/lex.yy.cpp src/parser/parser.tab.hpp src/parser/parser.tab.cpp)
set_source_files_properties(${PARSER_TARGETS} GENERATED)
add_custom_target(parser)

add_custom_command(
    COMMAND bison -d src/parser/parser.y -o src/parser/parser.tab.cpp
    TARGET parser
    OUTPUTS src/parser/parser.tab.cpp src/parser/parser.tab.hpp
    WORKING_DIRECTORY .
)

add_custom_command(
    COMMAND flex -o src/parser/lex.yy.cpp src/parser/lexer.l
    TARGET parser
    OUTPUTS src/parser/lex.yy.cpp
    WORKING_DIRECTORY .
)

include_directories(src/include src/parser)
aux_source_directory(src/rm DIR_SRCS_RM)
aux_source_directory(src/ix DIR_SRCS_IX)
aux_source_directory(src DIR_SRCS)

add_executable(dbmain ${DIR_SRCS} ${DIR_SRCS_RM} ${DIR_SRCS_IX} ${PARSER_TARGETS} src/parser/parser_base.cpp)
add_dependencies(dbmain parser)