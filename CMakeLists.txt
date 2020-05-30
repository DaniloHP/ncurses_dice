cmake_minimum_required(VERSION 3.7.2)
project(ncurses)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
include_directories(${PROJECT_SOURCE_DIR}/include)

file(MAKE_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

find_package(Curses REQUIRED)
include_directories(${NCURSES_INCLUDE_DIR})

add_executable(dice
        src/DiceController.cpp
        include/DiceController.h
        src/DiceView.cpp
        include/DiceRoll.h
        src/DiceModel.cpp
        include/DiceModel.h
        )

target_link_libraries(dice -lncurses)
