# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.12

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/icyf/Development/database/thu-rdbms

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/icyf/Development/database/thu-rdbms

# Include any dependencies generated for this target.
include src/rm/CMakeFiles/dbrm.dir/depend.make

# Include the progress variables for this target.
include src/rm/CMakeFiles/dbrm.dir/progress.make

# Include the compile flags for this target's objects.
include src/rm/CMakeFiles/dbrm.dir/flags.make

src/rm/CMakeFiles/dbrm.dir/rm_file.cpp.o: src/rm/CMakeFiles/dbrm.dir/flags.make
src/rm/CMakeFiles/dbrm.dir/rm_file.cpp.o: src/rm/rm_file.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/icyf/Development/database/thu-rdbms/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object src/rm/CMakeFiles/dbrm.dir/rm_file.cpp.o"
	cd /home/icyf/Development/database/thu-rdbms/src/rm && /usr/bin/g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/dbrm.dir/rm_file.cpp.o -c /home/icyf/Development/database/thu-rdbms/src/rm/rm_file.cpp

src/rm/CMakeFiles/dbrm.dir/rm_file.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/dbrm.dir/rm_file.cpp.i"
	cd /home/icyf/Development/database/thu-rdbms/src/rm && /usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/icyf/Development/database/thu-rdbms/src/rm/rm_file.cpp > CMakeFiles/dbrm.dir/rm_file.cpp.i

src/rm/CMakeFiles/dbrm.dir/rm_file.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/dbrm.dir/rm_file.cpp.s"
	cd /home/icyf/Development/database/thu-rdbms/src/rm && /usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/icyf/Development/database/thu-rdbms/src/rm/rm_file.cpp -o CMakeFiles/dbrm.dir/rm_file.cpp.s

src/rm/CMakeFiles/dbrm.dir/rm_manager.cpp.o: src/rm/CMakeFiles/dbrm.dir/flags.make
src/rm/CMakeFiles/dbrm.dir/rm_manager.cpp.o: src/rm/rm_manager.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/icyf/Development/database/thu-rdbms/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object src/rm/CMakeFiles/dbrm.dir/rm_manager.cpp.o"
	cd /home/icyf/Development/database/thu-rdbms/src/rm && /usr/bin/g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/dbrm.dir/rm_manager.cpp.o -c /home/icyf/Development/database/thu-rdbms/src/rm/rm_manager.cpp

src/rm/CMakeFiles/dbrm.dir/rm_manager.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/dbrm.dir/rm_manager.cpp.i"
	cd /home/icyf/Development/database/thu-rdbms/src/rm && /usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/icyf/Development/database/thu-rdbms/src/rm/rm_manager.cpp > CMakeFiles/dbrm.dir/rm_manager.cpp.i

src/rm/CMakeFiles/dbrm.dir/rm_manager.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/dbrm.dir/rm_manager.cpp.s"
	cd /home/icyf/Development/database/thu-rdbms/src/rm && /usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/icyf/Development/database/thu-rdbms/src/rm/rm_manager.cpp -o CMakeFiles/dbrm.dir/rm_manager.cpp.s

src/rm/CMakeFiles/dbrm.dir/rm_record.cpp.o: src/rm/CMakeFiles/dbrm.dir/flags.make
src/rm/CMakeFiles/dbrm.dir/rm_record.cpp.o: src/rm/rm_record.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/icyf/Development/database/thu-rdbms/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object src/rm/CMakeFiles/dbrm.dir/rm_record.cpp.o"
	cd /home/icyf/Development/database/thu-rdbms/src/rm && /usr/bin/g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/dbrm.dir/rm_record.cpp.o -c /home/icyf/Development/database/thu-rdbms/src/rm/rm_record.cpp

src/rm/CMakeFiles/dbrm.dir/rm_record.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/dbrm.dir/rm_record.cpp.i"
	cd /home/icyf/Development/database/thu-rdbms/src/rm && /usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/icyf/Development/database/thu-rdbms/src/rm/rm_record.cpp > CMakeFiles/dbrm.dir/rm_record.cpp.i

src/rm/CMakeFiles/dbrm.dir/rm_record.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/dbrm.dir/rm_record.cpp.s"
	cd /home/icyf/Development/database/thu-rdbms/src/rm && /usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/icyf/Development/database/thu-rdbms/src/rm/rm_record.cpp -o CMakeFiles/dbrm.dir/rm_record.cpp.s

src/rm/CMakeFiles/dbrm.dir/rm_scanner.cpp.o: src/rm/CMakeFiles/dbrm.dir/flags.make
src/rm/CMakeFiles/dbrm.dir/rm_scanner.cpp.o: src/rm/rm_scanner.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/icyf/Development/database/thu-rdbms/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object src/rm/CMakeFiles/dbrm.dir/rm_scanner.cpp.o"
	cd /home/icyf/Development/database/thu-rdbms/src/rm && /usr/bin/g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/dbrm.dir/rm_scanner.cpp.o -c /home/icyf/Development/database/thu-rdbms/src/rm/rm_scanner.cpp

src/rm/CMakeFiles/dbrm.dir/rm_scanner.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/dbrm.dir/rm_scanner.cpp.i"
	cd /home/icyf/Development/database/thu-rdbms/src/rm && /usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/icyf/Development/database/thu-rdbms/src/rm/rm_scanner.cpp > CMakeFiles/dbrm.dir/rm_scanner.cpp.i

src/rm/CMakeFiles/dbrm.dir/rm_scanner.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/dbrm.dir/rm_scanner.cpp.s"
	cd /home/icyf/Development/database/thu-rdbms/src/rm && /usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/icyf/Development/database/thu-rdbms/src/rm/rm_scanner.cpp -o CMakeFiles/dbrm.dir/rm_scanner.cpp.s

# Object files for target dbrm
dbrm_OBJECTS = \
"CMakeFiles/dbrm.dir/rm_file.cpp.o" \
"CMakeFiles/dbrm.dir/rm_manager.cpp.o" \
"CMakeFiles/dbrm.dir/rm_record.cpp.o" \
"CMakeFiles/dbrm.dir/rm_scanner.cpp.o"

# External object files for target dbrm
dbrm_EXTERNAL_OBJECTS =

src/rm/libdbrm.a: src/rm/CMakeFiles/dbrm.dir/rm_file.cpp.o
src/rm/libdbrm.a: src/rm/CMakeFiles/dbrm.dir/rm_manager.cpp.o
src/rm/libdbrm.a: src/rm/CMakeFiles/dbrm.dir/rm_record.cpp.o
src/rm/libdbrm.a: src/rm/CMakeFiles/dbrm.dir/rm_scanner.cpp.o
src/rm/libdbrm.a: src/rm/CMakeFiles/dbrm.dir/build.make
src/rm/libdbrm.a: src/rm/CMakeFiles/dbrm.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/icyf/Development/database/thu-rdbms/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Linking CXX static library libdbrm.a"
	cd /home/icyf/Development/database/thu-rdbms/src/rm && $(CMAKE_COMMAND) -P CMakeFiles/dbrm.dir/cmake_clean_target.cmake
	cd /home/icyf/Development/database/thu-rdbms/src/rm && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/dbrm.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
src/rm/CMakeFiles/dbrm.dir/build: src/rm/libdbrm.a

.PHONY : src/rm/CMakeFiles/dbrm.dir/build

src/rm/CMakeFiles/dbrm.dir/clean:
	cd /home/icyf/Development/database/thu-rdbms/src/rm && $(CMAKE_COMMAND) -P CMakeFiles/dbrm.dir/cmake_clean.cmake
.PHONY : src/rm/CMakeFiles/dbrm.dir/clean

src/rm/CMakeFiles/dbrm.dir/depend:
	cd /home/icyf/Development/database/thu-rdbms && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/icyf/Development/database/thu-rdbms /home/icyf/Development/database/thu-rdbms/src/rm /home/icyf/Development/database/thu-rdbms /home/icyf/Development/database/thu-rdbms/src/rm /home/icyf/Development/database/thu-rdbms/src/rm/CMakeFiles/dbrm.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : src/rm/CMakeFiles/dbrm.dir/depend

