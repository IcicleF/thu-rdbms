cmake_minimum_required(VERSION 3.0)

set(CMAKE_C_COMPILER "gcc")
set(CMAKE_CXX_COMPILER "g++")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(thu-rdbms)

include_directories(src/include)
aux_source_directory(src/rm DIR_SRCS_RM)
aux_source_directory(src/ix DIR_SRCS_IX)
aux_source_directory(src DIR_SRCS)

add_executable(dbmain ${DIR_SRCS} ${DIR_SRCS_RM} ${DIR_SRCS_IX})