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
	)
target_include_directories(red PRIVATE include)
target_compile_definitions(red PUBLIC -DNOMINMAX)
target_compile_features(red PRIVATE cxx_std_17)

# Testing
add_executable(gap-buffer-test src/gap_buffer.test.cpp src/gap_buffer.cpp)
target_include_directories(gap-buffer-test PRIVATE include)
