cmake_minimum_required(VERSION 3.6)

project(red LANGUAGES CXX)

add_executable(red
	src/main.cpp
	src/gap_buffer.cpp
	src/buffer.cpp
	src/screen.cpp
	)
target_include_directories(red PRIVATE include)
target_compile_definitions(red PUBLIC -DNOMINMAX)
