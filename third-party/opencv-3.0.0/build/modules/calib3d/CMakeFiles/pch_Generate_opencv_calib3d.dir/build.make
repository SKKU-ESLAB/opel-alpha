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
CMAKE_SOURCE_DIR = /home/ubuntu/opel-alpha/third-party/opencv-3.0.0

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/ubuntu/opel-alpha/third-party/opencv-3.0.0/build

# Utility rule file for pch_Generate_opencv_calib3d.

# Include the progress variables for this target.
include modules/calib3d/CMakeFiles/pch_Generate_opencv_calib3d.dir/progress.make

modules/calib3d/CMakeFiles/pch_Generate_opencv_calib3d: modules/calib3d/precomp.hpp.gch/opencv_calib3d_RELEASE.gch


modules/calib3d/precomp.hpp.gch/opencv_calib3d_RELEASE.gch: ../modules/calib3d/src/precomp.hpp
modules/calib3d/precomp.hpp.gch/opencv_calib3d_RELEASE.gch: modules/calib3d/precomp.hpp
modules/calib3d/precomp.hpp.gch/opencv_calib3d_RELEASE.gch: lib/libopencv_calib3d_pch_dephelp.a
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/ubuntu/opel-alpha/third-party/opencv-3.0.0/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Generating precomp.hpp.gch/opencv_calib3d_RELEASE.gch"
	cd /home/ubuntu/opel-alpha/third-party/opencv-3.0.0/build/modules/calib3d && /usr/bin/cmake -E make_directory /home/ubuntu/opel-alpha/third-party/opencv-3.0.0/build/modules/calib3d/precomp.hpp.gch
	cd /home/ubuntu/opel-alpha/third-party/opencv-3.0.0/build/modules/calib3d && /usr/bin/c++ -O3 -DNDEBUG -DNDEBUG -fPIC -DCVAPI_EXPORTS -isystem"/home/ubuntu/opel-alpha/third-party/opencv-3.0.0/3rdparty/include/opencl/1.2" -isystem"/home/ubuntu/opel-alpha/third-party/opencv-3.0.0/build" -isystem"/home/ubuntu/opel-alpha/third-party/opencv-3.0.0/3rdparty/include/opencl/1.2" -isystem"/home/ubuntu/opel-alpha/third-party/opencv-3.0.0/build" -I"/home/ubuntu/opel-alpha/third-party/opencv-3.0.0/modules/calib3d/include" -I"/home/ubuntu/opel-alpha/third-party/opencv-3.0.0/modules/calib3d/src" -isystem"/home/ubuntu/opel-alpha/third-party/opencv-3.0.0/build/modules/calib3d" -I"/home/ubuntu/opel-alpha/third-party/opencv-3.0.0/modules/hal/include" -I"/home/ubuntu/opel-alpha/third-party/opencv-3.0.0/modules/core/include" -I"/home/ubuntu/opel-alpha/third-party/opencv-3.0.0/modules/flann/include" -I"/home/ubuntu/opel-alpha/third-party/opencv-3.0.0/modules/imgproc/include" -I"/home/ubuntu/opel-alpha/third-party/opencv-3.0.0/modules/ml/include" -I"/home/ubuntu/opel-alpha/third-party/opencv-3.0.0/modules/imgcodecs/include" -I"/home/ubuntu/opel-alpha/third-party/opencv-3.0.0/modules/videoio/include" -I"/home/ubuntu/opel-alpha/third-party/opencv-3.0.0/modules/highgui/include" -I"/home/ubuntu/opel-alpha/third-party/opencv-3.0.0/modules/features2d/include" -D__OPENCV_BUILD=1 -D__OPENCV_BUILD=1 -fsigned-char -W -Wall -Werror=return-type -Werror=non-virtual-dtor -Werror=address -Werror=sequence-point -Wformat -Werror=format-security -Wmissing-declarations -Wundef -Winit-self -Wpointer-arith -Wshadow -Wsign-promo -Wno-narrowing -Wno-delete-non-virtual-dtor -fdiagnostics-show-option -pthread -fomit-frame-pointer -ffunction-sections -fvisibility=hidden -fvisibility-inlines-hidden -DCVAPI_EXPORTS -x c++-header -o /home/ubuntu/opel-alpha/third-party/opencv-3.0.0/build/modules/calib3d/precomp.hpp.gch/opencv_calib3d_RELEASE.gch /home/ubuntu/opel-alpha/third-party/opencv-3.0.0/build/modules/calib3d/precomp.hpp

modules/calib3d/precomp.hpp: ../modules/calib3d/src/precomp.hpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/ubuntu/opel-alpha/third-party/opencv-3.0.0/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Generating precomp.hpp"
	cd /home/ubuntu/opel-alpha/third-party/opencv-3.0.0/build/modules/calib3d && /usr/bin/cmake -E copy /home/ubuntu/opel-alpha/third-party/opencv-3.0.0/modules/calib3d/src/precomp.hpp /home/ubuntu/opel-alpha/third-party/opencv-3.0.0/build/modules/calib3d/precomp.hpp

pch_Generate_opencv_calib3d: modules/calib3d/CMakeFiles/pch_Generate_opencv_calib3d
pch_Generate_opencv_calib3d: modules/calib3d/precomp.hpp.gch/opencv_calib3d_RELEASE.gch
pch_Generate_opencv_calib3d: modules/calib3d/precomp.hpp
pch_Generate_opencv_calib3d: modules/calib3d/CMakeFiles/pch_Generate_opencv_calib3d.dir/build.make

.PHONY : pch_Generate_opencv_calib3d

# Rule to build all files generated by this target.
modules/calib3d/CMakeFiles/pch_Generate_opencv_calib3d.dir/build: pch_Generate_opencv_calib3d

.PHONY : modules/calib3d/CMakeFiles/pch_Generate_opencv_calib3d.dir/build

modules/calib3d/CMakeFiles/pch_Generate_opencv_calib3d.dir/clean:
	cd /home/ubuntu/opel-alpha/third-party/opencv-3.0.0/build/modules/calib3d && $(CMAKE_COMMAND) -P CMakeFiles/pch_Generate_opencv_calib3d.dir/cmake_clean.cmake
.PHONY : modules/calib3d/CMakeFiles/pch_Generate_opencv_calib3d.dir/clean

modules/calib3d/CMakeFiles/pch_Generate_opencv_calib3d.dir/depend:
	cd /home/ubuntu/opel-alpha/third-party/opencv-3.0.0/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/ubuntu/opel-alpha/third-party/opencv-3.0.0 /home/ubuntu/opel-alpha/third-party/opencv-3.0.0/modules/calib3d /home/ubuntu/opel-alpha/third-party/opencv-3.0.0/build /home/ubuntu/opel-alpha/third-party/opencv-3.0.0/build/modules/calib3d /home/ubuntu/opel-alpha/third-party/opencv-3.0.0/build/modules/calib3d/CMakeFiles/pch_Generate_opencv_calib3d.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : modules/calib3d/CMakeFiles/pch_Generate_opencv_calib3d.dir/depend
