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
CMAKE_SOURCE_DIR = /home/caijunyao/RDMA_DML/DML-EXP/dml/evaluation/src

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/caijunyao/RDMA_DML/DML-EXP/dml/evaluation/src/build

# Include any dependencies generated for this target.
include network/CMakeFiles/netsys.dir/depend.make

# Include the progress variables for this target.
include network/CMakeFiles/netsys.dir/progress.make

# Include the compile flags for this target's objects.
include network/CMakeFiles/netsys.dir/flags.make

network/CMakeFiles/netsys.dir/client.cpp.o: network/CMakeFiles/netsys.dir/flags.make
network/CMakeFiles/netsys.dir/client.cpp.o: ../network/client.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/caijunyao/RDMA_DML/DML-EXP/dml/evaluation/src/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object network/CMakeFiles/netsys.dir/client.cpp.o"
	cd /home/caijunyao/RDMA_DML/DML-EXP/dml/evaluation/src/build/network && /usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/netsys.dir/client.cpp.o -c /home/caijunyao/RDMA_DML/DML-EXP/dml/evaluation/src/network/client.cpp

network/CMakeFiles/netsys.dir/client.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/netsys.dir/client.cpp.i"
	cd /home/caijunyao/RDMA_DML/DML-EXP/dml/evaluation/src/build/network && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/caijunyao/RDMA_DML/DML-EXP/dml/evaluation/src/network/client.cpp > CMakeFiles/netsys.dir/client.cpp.i

network/CMakeFiles/netsys.dir/client.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/netsys.dir/client.cpp.s"
	cd /home/caijunyao/RDMA_DML/DML-EXP/dml/evaluation/src/build/network && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/caijunyao/RDMA_DML/DML-EXP/dml/evaluation/src/network/client.cpp -o CMakeFiles/netsys.dir/client.cpp.s

network/CMakeFiles/netsys.dir/client.cpp.o.requires:

.PHONY : network/CMakeFiles/netsys.dir/client.cpp.o.requires

network/CMakeFiles/netsys.dir/client.cpp.o.provides: network/CMakeFiles/netsys.dir/client.cpp.o.requires
	$(MAKE) -f network/CMakeFiles/netsys.dir/build.make network/CMakeFiles/netsys.dir/client.cpp.o.provides.build
.PHONY : network/CMakeFiles/netsys.dir/client.cpp.o.provides

network/CMakeFiles/netsys.dir/client.cpp.o.provides.build: network/CMakeFiles/netsys.dir/client.cpp.o


network/CMakeFiles/netsys.dir/server.cpp.o: network/CMakeFiles/netsys.dir/flags.make
network/CMakeFiles/netsys.dir/server.cpp.o: ../network/server.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/caijunyao/RDMA_DML/DML-EXP/dml/evaluation/src/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object network/CMakeFiles/netsys.dir/server.cpp.o"
	cd /home/caijunyao/RDMA_DML/DML-EXP/dml/evaluation/src/build/network && /usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/netsys.dir/server.cpp.o -c /home/caijunyao/RDMA_DML/DML-EXP/dml/evaluation/src/network/server.cpp

network/CMakeFiles/netsys.dir/server.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/netsys.dir/server.cpp.i"
	cd /home/caijunyao/RDMA_DML/DML-EXP/dml/evaluation/src/build/network && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/caijunyao/RDMA_DML/DML-EXP/dml/evaluation/src/network/server.cpp > CMakeFiles/netsys.dir/server.cpp.i

network/CMakeFiles/netsys.dir/server.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/netsys.dir/server.cpp.s"
	cd /home/caijunyao/RDMA_DML/DML-EXP/dml/evaluation/src/build/network && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/caijunyao/RDMA_DML/DML-EXP/dml/evaluation/src/network/server.cpp -o CMakeFiles/netsys.dir/server.cpp.s

network/CMakeFiles/netsys.dir/server.cpp.o.requires:

.PHONY : network/CMakeFiles/netsys.dir/server.cpp.o.requires

network/CMakeFiles/netsys.dir/server.cpp.o.provides: network/CMakeFiles/netsys.dir/server.cpp.o.requires
	$(MAKE) -f network/CMakeFiles/netsys.dir/build.make network/CMakeFiles/netsys.dir/server.cpp.o.provides.build
.PHONY : network/CMakeFiles/netsys.dir/server.cpp.o.provides

network/CMakeFiles/netsys.dir/server.cpp.o.provides.build: network/CMakeFiles/netsys.dir/server.cpp.o


network/CMakeFiles/netsys.dir/tower.cpp.o: network/CMakeFiles/netsys.dir/flags.make
network/CMakeFiles/netsys.dir/tower.cpp.o: ../network/tower.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/caijunyao/RDMA_DML/DML-EXP/dml/evaluation/src/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object network/CMakeFiles/netsys.dir/tower.cpp.o"
	cd /home/caijunyao/RDMA_DML/DML-EXP/dml/evaluation/src/build/network && /usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/netsys.dir/tower.cpp.o -c /home/caijunyao/RDMA_DML/DML-EXP/dml/evaluation/src/network/tower.cpp

network/CMakeFiles/netsys.dir/tower.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/netsys.dir/tower.cpp.i"
	cd /home/caijunyao/RDMA_DML/DML-EXP/dml/evaluation/src/build/network && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/caijunyao/RDMA_DML/DML-EXP/dml/evaluation/src/network/tower.cpp > CMakeFiles/netsys.dir/tower.cpp.i

network/CMakeFiles/netsys.dir/tower.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/netsys.dir/tower.cpp.s"
	cd /home/caijunyao/RDMA_DML/DML-EXP/dml/evaluation/src/build/network && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/caijunyao/RDMA_DML/DML-EXP/dml/evaluation/src/network/tower.cpp -o CMakeFiles/netsys.dir/tower.cpp.s

network/CMakeFiles/netsys.dir/tower.cpp.o.requires:

.PHONY : network/CMakeFiles/netsys.dir/tower.cpp.o.requires

network/CMakeFiles/netsys.dir/tower.cpp.o.provides: network/CMakeFiles/netsys.dir/tower.cpp.o.requires
	$(MAKE) -f network/CMakeFiles/netsys.dir/build.make network/CMakeFiles/netsys.dir/tower.cpp.o.provides.build
.PHONY : network/CMakeFiles/netsys.dir/tower.cpp.o.provides

network/CMakeFiles/netsys.dir/tower.cpp.o.provides.build: network/CMakeFiles/netsys.dir/tower.cpp.o


# Object files for target netsys
netsys_OBJECTS = \
"CMakeFiles/netsys.dir/client.cpp.o" \
"CMakeFiles/netsys.dir/server.cpp.o" \
"CMakeFiles/netsys.dir/tower.cpp.o"

# External object files for target netsys
netsys_EXTERNAL_OBJECTS =

network/libnetsys.so: network/CMakeFiles/netsys.dir/client.cpp.o
network/libnetsys.so: network/CMakeFiles/netsys.dir/server.cpp.o
network/libnetsys.so: network/CMakeFiles/netsys.dir/tower.cpp.o
network/libnetsys.so: network/CMakeFiles/netsys.dir/build.make
network/libnetsys.so: network/CMakeFiles/netsys.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/caijunyao/RDMA_DML/DML-EXP/dml/evaluation/src/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Linking CXX shared library libnetsys.so"
	cd /home/caijunyao/RDMA_DML/DML-EXP/dml/evaluation/src/build/network && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/netsys.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
network/CMakeFiles/netsys.dir/build: network/libnetsys.so

.PHONY : network/CMakeFiles/netsys.dir/build

network/CMakeFiles/netsys.dir/requires: network/CMakeFiles/netsys.dir/client.cpp.o.requires
network/CMakeFiles/netsys.dir/requires: network/CMakeFiles/netsys.dir/server.cpp.o.requires
network/CMakeFiles/netsys.dir/requires: network/CMakeFiles/netsys.dir/tower.cpp.o.requires

.PHONY : network/CMakeFiles/netsys.dir/requires

network/CMakeFiles/netsys.dir/clean:
	cd /home/caijunyao/RDMA_DML/DML-EXP/dml/evaluation/src/build/network && $(CMAKE_COMMAND) -P CMakeFiles/netsys.dir/cmake_clean.cmake
.PHONY : network/CMakeFiles/netsys.dir/clean

network/CMakeFiles/netsys.dir/depend:
	cd /home/caijunyao/RDMA_DML/DML-EXP/dml/evaluation/src/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/caijunyao/RDMA_DML/DML-EXP/dml/evaluation/src /home/caijunyao/RDMA_DML/DML-EXP/dml/evaluation/src/network /home/caijunyao/RDMA_DML/DML-EXP/dml/evaluation/src/build /home/caijunyao/RDMA_DML/DML-EXP/dml/evaluation/src/build/network /home/caijunyao/RDMA_DML/DML-EXP/dml/evaluation/src/build/network/CMakeFiles/netsys.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : network/CMakeFiles/netsys.dir/depend

