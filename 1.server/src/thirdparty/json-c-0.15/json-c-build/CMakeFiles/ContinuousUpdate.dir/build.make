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

# Utility rule file for ContinuousUpdate.

# Include the progress variables for this target.
include CMakeFiles/ContinuousUpdate.dir/progress.make

CMakeFiles/ContinuousUpdate:
	/usr/bin/ctest3 -D ContinuousUpdate

ContinuousUpdate: CMakeFiles/ContinuousUpdate
ContinuousUpdate: CMakeFiles/ContinuousUpdate.dir/build.make

.PHONY : ContinuousUpdate

# Rule to build all files generated by this target.
CMakeFiles/ContinuousUpdate.dir/build: ContinuousUpdate

.PHONY : CMakeFiles/ContinuousUpdate.dir/build

CMakeFiles/ContinuousUpdate.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/ContinuousUpdate.dir/cmake_clean.cmake
.PHONY : CMakeFiles/ContinuousUpdate.dir/clean

CMakeFiles/ContinuousUpdate.dir/depend:
	cd /home/bluexg/Blue-X-ray-Eraser/1.server/src/thirdparty/json-c-0.15/json-c-build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/bluexg/Blue-X-ray-Eraser/1.server/src/thirdparty/json-c-0.15 /home/bluexg/Blue-X-ray-Eraser/1.server/src/thirdparty/json-c-0.15 /home/bluexg/Blue-X-ray-Eraser/1.server/src/thirdparty/json-c-0.15/json-c-build /home/bluexg/Blue-X-ray-Eraser/1.server/src/thirdparty/json-c-0.15/json-c-build /home/bluexg/Blue-X-ray-Eraser/1.server/src/thirdparty/json-c-0.15/json-c-build/CMakeFiles/ContinuousUpdate.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/ContinuousUpdate.dir/depend

