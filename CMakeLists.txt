cmake_minimum_required(VERSION 3.23)

project(Browser)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED 23)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -I${CMAKE_CURRENT_SOURCE_DIR}/src/headers")

add_executable(browser)

target_sources(browser
PRIVATE
    src/browser.cpp
    src/code/html-parser.cpp
    src/code/http-client.cpp
    src/code/user-interface.cpp
    src/code/error-handler.cpp
    src/code/input-handler.cpp
    src/headers/html-parser.hpp
    src/headers/http-client.hpp
    src/headers/user-interface.hpp
    src/headers/error-handler.hpp
    src/headers/input-handler.hpp
)

add_executable(parser)

target_sources(parser
PRIVATE
	src/parser.cpp
	src/code/html-parser.cpp
	src/code/error-handler.cpp
	src/code/input-handler.cpp
	src/headers/html-parser.hpp
	src/headers/error-handler.hpp
	src/headers/input-handler.hpp
)
