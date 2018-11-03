cmake_minimum_required(VERSION 3.10)
project(midicontrol)
set(midicontrol_VERSION_MAJOR 0)
set(midicontrol_VERSION_MINOR 1)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

set(PROJECT_SOURCE_DIR ${CMAKE_SOURCE_DIR}/src)
set(MIDICONTROL_SOURCES ${PROJECT_SOURCE_DIR}/keytest.cpp ${PROJECT_SOURCE_DIR}/entry.cpp ${PROJECT_SOURCE_DIR}/settings.cpp)

if(MSVC)
	add_compile_options(/W4)
else()
	add_compile_options(-Wall -Wextra -Wpedantic)
endif()

add_executable(midicontrol ${MIDICONTROL_SOURCES})
target_link_libraries(midicontrol rtmidi boost_regex yaml-cpp pthread)
