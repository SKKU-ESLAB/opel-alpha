# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.5

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
CMAKE_SOURCE_DIR = /home/ubuntu/opel-alpha

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/ubuntu/opel-alpha/build

# Include any dependencies generated for this target.
include CMakeFiles/cam_fw.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/cam_fw.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/cam_fw.dir/flags.make

CMakeFiles/cam_fw.dir/src/main.o: CMakeFiles/cam_fw.dir/flags.make
CMakeFiles/cam_fw.dir/src/main.o: ../src/main.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ubuntu/opel-alpha/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/cam_fw.dir/src/main.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/cam_fw.dir/src/main.o -c /home/ubuntu/opel-alpha/src/main.cpp

CMakeFiles/cam_fw.dir/src/main.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/cam_fw.dir/src/main.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ubuntu/opel-alpha/src/main.cpp > CMakeFiles/cam_fw.dir/src/main.i

CMakeFiles/cam_fw.dir/src/main.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/cam_fw.dir/src/main.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ubuntu/opel-alpha/src/main.cpp -o CMakeFiles/cam_fw.dir/src/main.s

CMakeFiles/cam_fw.dir/src/main.o.requires:

.PHONY : CMakeFiles/cam_fw.dir/src/main.o.requires

CMakeFiles/cam_fw.dir/src/main.o.provides: CMakeFiles/cam_fw.dir/src/main.o.requires
	$(MAKE) -f CMakeFiles/cam_fw.dir/build.make CMakeFiles/cam_fw.dir/src/main.o.provides.build
.PHONY : CMakeFiles/cam_fw.dir/src/main.o.provides

CMakeFiles/cam_fw.dir/src/main.o.provides.build: CMakeFiles/cam_fw.dir/src/main.o


CMakeFiles/cam_fw.dir/src/OPELgstElement.o: CMakeFiles/cam_fw.dir/flags.make
CMakeFiles/cam_fw.dir/src/OPELgstElement.o: ../src/OPELgstElement.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ubuntu/opel-alpha/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/cam_fw.dir/src/OPELgstElement.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/cam_fw.dir/src/OPELgstElement.o -c /home/ubuntu/opel-alpha/src/OPELgstElement.cpp

CMakeFiles/cam_fw.dir/src/OPELgstElement.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/cam_fw.dir/src/OPELgstElement.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ubuntu/opel-alpha/src/OPELgstElement.cpp > CMakeFiles/cam_fw.dir/src/OPELgstElement.i

CMakeFiles/cam_fw.dir/src/OPELgstElement.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/cam_fw.dir/src/OPELgstElement.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ubuntu/opel-alpha/src/OPELgstElement.cpp -o CMakeFiles/cam_fw.dir/src/OPELgstElement.s

CMakeFiles/cam_fw.dir/src/OPELgstElement.o.requires:

.PHONY : CMakeFiles/cam_fw.dir/src/OPELgstElement.o.requires

CMakeFiles/cam_fw.dir/src/OPELgstElement.o.provides: CMakeFiles/cam_fw.dir/src/OPELgstElement.o.requires
	$(MAKE) -f CMakeFiles/cam_fw.dir/build.make CMakeFiles/cam_fw.dir/src/OPELgstElement.o.provides.build
.PHONY : CMakeFiles/cam_fw.dir/src/OPELgstElement.o.provides

CMakeFiles/cam_fw.dir/src/OPELgstElement.o.provides.build: CMakeFiles/cam_fw.dir/src/OPELgstElement.o


CMakeFiles/cam_fw.dir/src/OPELgstElementTx1.o: CMakeFiles/cam_fw.dir/flags.make
CMakeFiles/cam_fw.dir/src/OPELgstElementTx1.o: ../src/OPELgstElementTx1.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ubuntu/opel-alpha/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CMakeFiles/cam_fw.dir/src/OPELgstElementTx1.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/cam_fw.dir/src/OPELgstElementTx1.o -c /home/ubuntu/opel-alpha/src/OPELgstElementTx1.cpp

CMakeFiles/cam_fw.dir/src/OPELgstElementTx1.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/cam_fw.dir/src/OPELgstElementTx1.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ubuntu/opel-alpha/src/OPELgstElementTx1.cpp > CMakeFiles/cam_fw.dir/src/OPELgstElementTx1.i

CMakeFiles/cam_fw.dir/src/OPELgstElementTx1.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/cam_fw.dir/src/OPELgstElementTx1.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ubuntu/opel-alpha/src/OPELgstElementTx1.cpp -o CMakeFiles/cam_fw.dir/src/OPELgstElementTx1.s

CMakeFiles/cam_fw.dir/src/OPELgstElementTx1.o.requires:

.PHONY : CMakeFiles/cam_fw.dir/src/OPELgstElementTx1.o.requires

CMakeFiles/cam_fw.dir/src/OPELgstElementTx1.o.provides: CMakeFiles/cam_fw.dir/src/OPELgstElementTx1.o.requires
	$(MAKE) -f CMakeFiles/cam_fw.dir/build.make CMakeFiles/cam_fw.dir/src/OPELgstElementTx1.o.provides.build
.PHONY : CMakeFiles/cam_fw.dir/src/OPELgstElementTx1.o.provides

CMakeFiles/cam_fw.dir/src/OPELgstElementTx1.o.provides.build: CMakeFiles/cam_fw.dir/src/OPELgstElementTx1.o


CMakeFiles/cam_fw.dir/src/OPELcamProperty.o: CMakeFiles/cam_fw.dir/flags.make
CMakeFiles/cam_fw.dir/src/OPELcamProperty.o: ../src/OPELcamProperty.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ubuntu/opel-alpha/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object CMakeFiles/cam_fw.dir/src/OPELcamProperty.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/cam_fw.dir/src/OPELcamProperty.o -c /home/ubuntu/opel-alpha/src/OPELcamProperty.cpp

CMakeFiles/cam_fw.dir/src/OPELcamProperty.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/cam_fw.dir/src/OPELcamProperty.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ubuntu/opel-alpha/src/OPELcamProperty.cpp > CMakeFiles/cam_fw.dir/src/OPELcamProperty.i

CMakeFiles/cam_fw.dir/src/OPELcamProperty.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/cam_fw.dir/src/OPELcamProperty.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ubuntu/opel-alpha/src/OPELcamProperty.cpp -o CMakeFiles/cam_fw.dir/src/OPELcamProperty.s

CMakeFiles/cam_fw.dir/src/OPELcamProperty.o.requires:

.PHONY : CMakeFiles/cam_fw.dir/src/OPELcamProperty.o.requires

CMakeFiles/cam_fw.dir/src/OPELcamProperty.o.provides: CMakeFiles/cam_fw.dir/src/OPELcamProperty.o.requires
	$(MAKE) -f CMakeFiles/cam_fw.dir/build.make CMakeFiles/cam_fw.dir/src/OPELcamProperty.o.provides.build
.PHONY : CMakeFiles/cam_fw.dir/src/OPELcamProperty.o.provides

CMakeFiles/cam_fw.dir/src/OPELcamProperty.o.provides.build: CMakeFiles/cam_fw.dir/src/OPELcamProperty.o


CMakeFiles/cam_fw.dir/src/OPELcamRequest.o: CMakeFiles/cam_fw.dir/flags.make
CMakeFiles/cam_fw.dir/src/OPELcamRequest.o: ../src/OPELcamRequest.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ubuntu/opel-alpha/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building CXX object CMakeFiles/cam_fw.dir/src/OPELcamRequest.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/cam_fw.dir/src/OPELcamRequest.o -c /home/ubuntu/opel-alpha/src/OPELcamRequest.cpp

CMakeFiles/cam_fw.dir/src/OPELcamRequest.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/cam_fw.dir/src/OPELcamRequest.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ubuntu/opel-alpha/src/OPELcamRequest.cpp > CMakeFiles/cam_fw.dir/src/OPELcamRequest.i

CMakeFiles/cam_fw.dir/src/OPELcamRequest.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/cam_fw.dir/src/OPELcamRequest.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ubuntu/opel-alpha/src/OPELcamRequest.cpp -o CMakeFiles/cam_fw.dir/src/OPELcamRequest.s

CMakeFiles/cam_fw.dir/src/OPELcamRequest.o.requires:

.PHONY : CMakeFiles/cam_fw.dir/src/OPELcamRequest.o.requires

CMakeFiles/cam_fw.dir/src/OPELcamRequest.o.provides: CMakeFiles/cam_fw.dir/src/OPELcamRequest.o.requires
	$(MAKE) -f CMakeFiles/cam_fw.dir/build.make CMakeFiles/cam_fw.dir/src/OPELcamRequest.o.provides.build
.PHONY : CMakeFiles/cam_fw.dir/src/OPELcamRequest.o.provides

CMakeFiles/cam_fw.dir/src/OPELcamRequest.o.provides.build: CMakeFiles/cam_fw.dir/src/OPELcamRequest.o


CMakeFiles/cam_fw.dir/src/OPELcommonUtil.o: CMakeFiles/cam_fw.dir/flags.make
CMakeFiles/cam_fw.dir/src/OPELcommonUtil.o: ../src/OPELcommonUtil.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ubuntu/opel-alpha/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Building CXX object CMakeFiles/cam_fw.dir/src/OPELcommonUtil.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/cam_fw.dir/src/OPELcommonUtil.o -c /home/ubuntu/opel-alpha/src/OPELcommonUtil.cpp

CMakeFiles/cam_fw.dir/src/OPELcommonUtil.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/cam_fw.dir/src/OPELcommonUtil.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ubuntu/opel-alpha/src/OPELcommonUtil.cpp > CMakeFiles/cam_fw.dir/src/OPELcommonUtil.i

CMakeFiles/cam_fw.dir/src/OPELcommonUtil.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/cam_fw.dir/src/OPELcommonUtil.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ubuntu/opel-alpha/src/OPELcommonUtil.cpp -o CMakeFiles/cam_fw.dir/src/OPELcommonUtil.s

CMakeFiles/cam_fw.dir/src/OPELcommonUtil.o.requires:

.PHONY : CMakeFiles/cam_fw.dir/src/OPELcommonUtil.o.requires

CMakeFiles/cam_fw.dir/src/OPELcommonUtil.o.provides: CMakeFiles/cam_fw.dir/src/OPELcommonUtil.o.requires
	$(MAKE) -f CMakeFiles/cam_fw.dir/build.make CMakeFiles/cam_fw.dir/src/OPELcommonUtil.o.provides.build
.PHONY : CMakeFiles/cam_fw.dir/src/OPELcommonUtil.o.provides

CMakeFiles/cam_fw.dir/src/OPELcommonUtil.o.provides.build: CMakeFiles/cam_fw.dir/src/OPELcommonUtil.o


CMakeFiles/cam_fw.dir/src/OPELglobalRequest.o: CMakeFiles/cam_fw.dir/flags.make
CMakeFiles/cam_fw.dir/src/OPELglobalRequest.o: ../src/OPELglobalRequest.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ubuntu/opel-alpha/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_7) "Building CXX object CMakeFiles/cam_fw.dir/src/OPELglobalRequest.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/cam_fw.dir/src/OPELglobalRequest.o -c /home/ubuntu/opel-alpha/src/OPELglobalRequest.cpp

CMakeFiles/cam_fw.dir/src/OPELglobalRequest.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/cam_fw.dir/src/OPELglobalRequest.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ubuntu/opel-alpha/src/OPELglobalRequest.cpp > CMakeFiles/cam_fw.dir/src/OPELglobalRequest.i

CMakeFiles/cam_fw.dir/src/OPELglobalRequest.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/cam_fw.dir/src/OPELglobalRequest.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ubuntu/opel-alpha/src/OPELglobalRequest.cpp -o CMakeFiles/cam_fw.dir/src/OPELglobalRequest.s

CMakeFiles/cam_fw.dir/src/OPELglobalRequest.o.requires:

.PHONY : CMakeFiles/cam_fw.dir/src/OPELglobalRequest.o.requires

CMakeFiles/cam_fw.dir/src/OPELglobalRequest.o.provides: CMakeFiles/cam_fw.dir/src/OPELglobalRequest.o.requires
	$(MAKE) -f CMakeFiles/cam_fw.dir/build.make CMakeFiles/cam_fw.dir/src/OPELglobalRequest.o.provides.build
.PHONY : CMakeFiles/cam_fw.dir/src/OPELglobalRequest.o.provides

CMakeFiles/cam_fw.dir/src/OPELglobalRequest.o.provides.build: CMakeFiles/cam_fw.dir/src/OPELglobalRequest.o


# Object files for target cam_fw
cam_fw_OBJECTS = \
"CMakeFiles/cam_fw.dir/src/main.o" \
"CMakeFiles/cam_fw.dir/src/OPELgstElement.o" \
"CMakeFiles/cam_fw.dir/src/OPELgstElementTx1.o" \
"CMakeFiles/cam_fw.dir/src/OPELcamProperty.o" \
"CMakeFiles/cam_fw.dir/src/OPELcamRequest.o" \
"CMakeFiles/cam_fw.dir/src/OPELcommonUtil.o" \
"CMakeFiles/cam_fw.dir/src/OPELglobalRequest.o"

# External object files for target cam_fw
cam_fw_EXTERNAL_OBJECTS =

cam_fw: CMakeFiles/cam_fw.dir/src/main.o
cam_fw: CMakeFiles/cam_fw.dir/src/OPELgstElement.o
cam_fw: CMakeFiles/cam_fw.dir/src/OPELgstElementTx1.o
cam_fw: CMakeFiles/cam_fw.dir/src/OPELcamProperty.o
cam_fw: CMakeFiles/cam_fw.dir/src/OPELcamRequest.o
cam_fw: CMakeFiles/cam_fw.dir/src/OPELcommonUtil.o
cam_fw: CMakeFiles/cam_fw.dir/src/OPELglobalRequest.o
cam_fw: CMakeFiles/cam_fw.dir/build.make
cam_fw: CMakeFiles/cam_fw.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/ubuntu/opel-alpha/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_8) "Linking CXX executable cam_fw"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/cam_fw.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/cam_fw.dir/build: cam_fw

.PHONY : CMakeFiles/cam_fw.dir/build

CMakeFiles/cam_fw.dir/requires: CMakeFiles/cam_fw.dir/src/main.o.requires
CMakeFiles/cam_fw.dir/requires: CMakeFiles/cam_fw.dir/src/OPELgstElement.o.requires
CMakeFiles/cam_fw.dir/requires: CMakeFiles/cam_fw.dir/src/OPELgstElementTx1.o.requires
CMakeFiles/cam_fw.dir/requires: CMakeFiles/cam_fw.dir/src/OPELcamProperty.o.requires
CMakeFiles/cam_fw.dir/requires: CMakeFiles/cam_fw.dir/src/OPELcamRequest.o.requires
CMakeFiles/cam_fw.dir/requires: CMakeFiles/cam_fw.dir/src/OPELcommonUtil.o.requires
CMakeFiles/cam_fw.dir/requires: CMakeFiles/cam_fw.dir/src/OPELglobalRequest.o.requires

.PHONY : CMakeFiles/cam_fw.dir/requires

CMakeFiles/cam_fw.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/cam_fw.dir/cmake_clean.cmake
.PHONY : CMakeFiles/cam_fw.dir/clean

CMakeFiles/cam_fw.dir/depend:
	cd /home/ubuntu/opel-alpha/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/ubuntu/opel-alpha /home/ubuntu/opel-alpha /home/ubuntu/opel-alpha/build /home/ubuntu/opel-alpha/build /home/ubuntu/opel-alpha/build/CMakeFiles/cam_fw.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/cam_fw.dir/depend

