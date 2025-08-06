cmake_minimum_required(VERSION 4.0.3-dirty)
project(
	tsk
	VERSION 1.0
	LANGUAGES C
	DESCRIPTION "A library written in c"
)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(
	tsk 
	src/tsk/type.c
	src/tsk/value.c
	src/tsk/array.c
	src/tsk/list.c
	src/tsk/map.c
)
target_include_directories(tsk PUBLIC include)
target_compile_options(
	tsk
	PRIVATE -Werror
			-Wall
			-Wextra
			-pedantic
			-Wfloat-equal
			-Wundef
			-Wshadow
			-Wpointer-arith
			-Wcast-align
			-Wstrict-prototypes
			-Wstrict-overflow=5
			-Wwrite-strings
			-Wcast-qual
			-Wconversion
			-fsanitize=undefined
			-fsanitize=address
)
target_link_options(
	tsk
	PRIVATE -fsanitize=undefined
			-fsanitize=address
)

add_executable(main src/main.c)
target_link_libraries(main tsk)
target_compile_options(
	main 
	PRIVATE -fsanitize=undefined
			-fsanitize=address
)
target_link_options(
	main
	PRIVATE -fsanitize=undefined
			-fsanitize=address
)

if(UNIT_TESTING)
	list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmocka)

	find_package(cmocka REQUIRED)

	include(AddCMockaTest)

	add_subdirectory(tests)
endif(UNIT_TESTING)
