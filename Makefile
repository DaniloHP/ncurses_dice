dice: build/DiceController.o build/DiceView.o build/DiceModel.o build/NcursesMenu.o
	g++ -g -ltinfo -lncurses -o bin/dice build/DiceView.o build/DiceController.o build/DiceModel.o build/NcursesMenu.o

build/DiceView.o: src/view/DiceView.cpp include/DiceController.h
	g++ -g -c -ltinfo -lncurses -o build/DiceView.o src/view/DiceView.cpp

build/DiceController.o: src/controller/DiceController.cpp include/DiceController.h include/DiceRoll.h
	g++ -g -c -o build/DiceController.o src/controller/DiceController.cpp

build/DiceModel.o: src/model/DiceModel.cpp include/DiceModel.h
	g++ -g -c -o build/DiceModel.o src/model/DiceModel.cpp

build/NcursesMenu.o: src/view/NcursesMenu.cpp
	g++ -g -c -o build/NcursesMenu.o src/view/NcursesMenu.cpp

clean:
	rm build/*
