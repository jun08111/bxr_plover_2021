# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.17

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Disable VCS-based implicit rules.
% : %,v


# Disable VCS-based implicit rules.
% : RCS/%


# Disable VCS-based implicit rules.
% : RCS/%,v


# Disable VCS-based implicit rules.
% : SCCS/s.%


# Disable VCS-based implicit rules.
% : s.%


.SUFFIXES: .hpux_make_needs_suffix_list


# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

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
CMAKE_COMMAND = /usr/bin/cmake3

# The command to remove a file.
RM = /usr/bin/cmake3 -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/bluexg/Blue-X-ray-Eraser/1.server/src/thirdparty/json-c-0.15

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/bluexg/Blue-X-ray-Eraser/1.server/src/thirdparty/json-c-0.15/json-c-build

# Include any dependencies generated for this target.
include tests/CMakeFiles/test_compare.dir/depend.make

# Include the progress variables for this target.
include tests/CMakeFiles/test_compare.dir/progress.make

# Include the compile flags for this target's objects.
include tests/CMakeFiles/test_compare.dir/flags.make

tests/CMakeFiles/test_compare.dir/test_compare.c.o: tests/CMakeFiles/test_compare.dir/flags.make
tests/CMakeFiles/test_compare.dir/test_compare.c.o: ../tests/test_compare.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/bluexg/Blue-X-ray-Eraser/1.server/src/thirdparty/json-c-0.15/json-c-build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object tests/CMakeFiles/test_compare.dir/test_compare.c.o"
	cd /home/bluexg/Blue-X-ray-Eraser/1.server/src/thirdparty/json-c-0.15/json-c-build/tests && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/test_compare.dir/test_compare.c.o   -c /home/bluexg/Blue-X-ray-Eraser/1.server/src/thirdparty/json-c-0.15/tests/test_compare.c

tests/CMakeFiles/test_compare.dir/test_compare.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/test_compare.dir/test_compare.c.i"
	cd /home/bluexg/Blue-X-ray-Eraser/1.server/src/thirdparty/json-c-0.15/json-c-build/tests && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/bluexg/Blue-X-ray-Eraser/1.server/src/thirdparty/json-c-0.15/tests/test_compare.c > CMakeFiles/test_compare.dir/test_compare.c.i

tests/CMakeFiles/test_compare.dir/test_compare.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/test_compare.dir/test_compare.c.s"
	cd /home/bluexg/Blue-X-ray-Eraser/1.server/src/thirdparty/json-c-0.15/json-c-build/tests && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/bluexg/Blue-X-ray-Eraser/1.server/src/thirdparty/json-c-0.15/tests/test_compare.c -o CMakeFiles/test_compare.dir/test_compare.c.s

# Object files for target test_compare
test_compare_OBJECTS = \
"CMakeFiles/test_compare.dir/test_compare.c.o"

# External object files for target test_compare
test_compare_EXTERNAL_OBJECTS =

tests/test_compare: tests/CMakeFiles/test_compare.dir/test_compare.c.o
tests/test_compare: tests/CMakeFiles/test_compare.dir/build.make
tests/test_compare: libjson-c.a
tests/test_compare: tests/CMakeFiles/test_compare.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/bluexg/Blue-X-ray-Eraser/1.server/src/thirdparty/json-c-0.15/json-c-build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable test_compare"
	cd /home/bluexg/Blue-X-ray-Eraser/1.server/src/thirdparty/json-c-0.15/json-c-build/tests && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/test_compare.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
tests/CMakeFiles/test_compare.dir/build: tests/test_compare

.PHONY : tests/CMakeFiles/test_compare.dir/build

tests/CMakeFiles/test_compare.dir/clean:
	cd /home/bluexg/Blue-X-ray-Eraser/1.server/src/thirdparty/json-c-0.15/json-c-build/tests && $(CMAKE_COMMAND) -P CMakeFiles/test_compare.dir/cmake_clean.cmake
.PHONY : tests/CMakeFiles/test_compare.dir/clean

tests/CMakeFiles/test_compare.dir/depend:
	cd /home/bluexg/Blue-X-ray-Eraser/1.server/src/thirdparty/json-c-0.15/json-c-build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/bluexg/Blue-X-ray-Eraser/1.server/src/thirdparty/json-c-0.15 /home/bluexg/Blue-X-ray-Eraser/1.server/src/thirdparty/json-c-0.15/tests /home/bluexg/Blue-X-ray-Eraser/1.server/src/thirdparty/json-c-0.15/json-c-build /home/bluexg/Blue-X-ray-Eraser/1.server/src/thirdparty/json-c-0.15/json-c-build/tests /home/bluexg/Blue-X-ray-Eraser/1.server/src/thirdparty/json-c-0.15/json-c-build/tests/CMakeFiles/test_compare.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : tests/CMakeFiles/test_compare.dir/depend

