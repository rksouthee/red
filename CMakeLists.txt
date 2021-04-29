cmake_minimum_required(VERSION 3.6)

project(red LANGUAGES CXX)

add_executable(red
	src/main.cpp
	src/gap_buffer.cpp
	src/buffer.cpp
	src/screen.cpp
	src/input.cpp
	src/file.cpp
	src/display.cpp
	src/prompt.cpp
	src/command.cpp
	src/iterator.cpp

	include/buffer.h
	include/command.h
	include/display.h
	include/editor.h
	include/file.h
	include/gap_buffer.h
	include/input.h
	include/iterator.h
	include/prompt.h
	include/screen.h
	include/utility.h

	src/red.natvis
	)
target_include_directories(red PRIVATE include)
target_compile_definitions(red PUBLIC -DNOMINMAX)
target_compile_features(red PRIVATE cxx_std_17)

# Testing
add_executable(gap-buffer-test src/gap_buffer.test.cpp src/gap_buffer.cpp)
target_include_directories(gap-buffer-test PRIVATE include)
