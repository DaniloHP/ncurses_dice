cmake_minimum_required(VERSION 3.15)
project(ncurses)

set(CMAKE_CXX_STANDARD 14)


add_executable(ncurses
        src/controller/DiceController.cpp include/DiceController.h src/view/DiceView.cpp src/controller/DiceRoll.cpp include/DiceRoll.h src/model/DiceModel.cpp include/DiceModel.h test/ModelTest.cpp test/ModelTest.cpp)
