# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 2.8

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
CMAKE_SOURCE_DIR = /home/pi/opel

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/pi/opel

# Include any dependencies generated for this target.
include opel_cmfw/CMakeFiles/tmp_control.dir/depend.make

# Include the progress variables for this target.
include opel_cmfw/CMakeFiles/tmp_control.dir/progress.make

# Include the compile flags for this target's objects.
include opel_cmfw/CMakeFiles/tmp_control.dir/flags.make

opel_cmfw/CMakeFiles/tmp_control.dir/src/tmp_control.cpp.o: opel_cmfw/CMakeFiles/tmp_control.dir/flags.make
opel_cmfw/CMakeFiles/tmp_control.dir/src/tmp_control.cpp.o: opel_cmfw/src/tmp_control.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report /home/pi/opel/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object opel_cmfw/CMakeFiles/tmp_control.dir/src/tmp_control.cpp.o"
	cd /home/pi/opel/opel_cmfw && /usr/bin/g++-4.8   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/tmp_control.dir/src/tmp_control.cpp.o -c /home/pi/opel/opel_cmfw/src/tmp_control.cpp

opel_cmfw/CMakeFiles/tmp_control.dir/src/tmp_control.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/tmp_control.dir/src/tmp_control.cpp.i"
	cd /home/pi/opel/opel_cmfw && /usr/bin/g++-4.8  $(CXX_DEFINES) $(CXX_FLAGS) -E /home/pi/opel/opel_cmfw/src/tmp_control.cpp > CMakeFiles/tmp_control.dir/src/tmp_control.cpp.i

opel_cmfw/CMakeFiles/tmp_control.dir/src/tmp_control.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/tmp_control.dir/src/tmp_control.cpp.s"
	cd /home/pi/opel/opel_cmfw && /usr/bin/g++-4.8  $(CXX_DEFINES) $(CXX_FLAGS) -S /home/pi/opel/opel_cmfw/src/tmp_control.cpp -o CMakeFiles/tmp_control.dir/src/tmp_control.cpp.s

opel_cmfw/CMakeFiles/tmp_control.dir/src/tmp_control.cpp.o.requires:
.PHONY : opel_cmfw/CMakeFiles/tmp_control.dir/src/tmp_control.cpp.o.requires

opel_cmfw/CMakeFiles/tmp_control.dir/src/tmp_control.cpp.o.provides: opel_cmfw/CMakeFiles/tmp_control.dir/src/tmp_control.cpp.o.requires
	$(MAKE) -f opel_cmfw/CMakeFiles/tmp_control.dir/build.make opel_cmfw/CMakeFiles/tmp_control.dir/src/tmp_control.cpp.o.provides.build
.PHONY : opel_cmfw/CMakeFiles/tmp_control.dir/src/tmp_control.cpp.o.provides

opel_cmfw/CMakeFiles/tmp_control.dir/src/tmp_control.cpp.o.provides.build: opel_cmfw/CMakeFiles/tmp_control.dir/src/tmp_control.cpp.o

tmp_control: opel_cmfw/CMakeFiles/tmp_control.dir/src/tmp_control.cpp.o
tmp_control: opel_cmfw/CMakeFiles/tmp_control.dir/build.make
.PHONY : tmp_control

# Rule to build all files generated by this target.
opel_cmfw/CMakeFiles/tmp_control.dir/build: tmp_control
.PHONY : opel_cmfw/CMakeFiles/tmp_control.dir/build

opel_cmfw/CMakeFiles/tmp_control.dir/requires: opel_cmfw/CMakeFiles/tmp_control.dir/src/tmp_control.cpp.o.requires
.PHONY : opel_cmfw/CMakeFiles/tmp_control.dir/requires

opel_cmfw/CMakeFiles/tmp_control.dir/clean:
	cd /home/pi/opel/opel_cmfw && $(CMAKE_COMMAND) -P CMakeFiles/tmp_control.dir/cmake_clean.cmake
.PHONY : opel_cmfw/CMakeFiles/tmp_control.dir/clean

opel_cmfw/CMakeFiles/tmp_control.dir/depend:
	cd /home/pi/opel && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/pi/opel /home/pi/opel/opel_cmfw /home/pi/opel /home/pi/opel/opel_cmfw /home/pi/opel/opel_cmfw/CMakeFiles/tmp_control.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : opel_cmfw/CMakeFiles/tmp_control.dir/depend

