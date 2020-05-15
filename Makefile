dice: build/DiceController.o build/DiceView.o build/DiceRoll.o build/DiceModel.o
	g++ -g -ltinfo -lncurses -o bin/dice build/DiceView.o build/DiceController.o build/DiceRoll.o build/DiceModel.o

build/DiceView.o: src/view/DiceView.cpp include/DiceController.h
	g++ -c -g -ltinfo -lncurses -o build/DiceView.o src/view/DiceView.cpp

build/DiceController.o: src/controller/DiceController.cpp include/DiceController.h include/DiceRoll.h
	g++ -c -g -o build/DiceController.o src/controller/DiceController.cpp

build/DiceRoll.o: src/controller/DiceRoll.cpp include/DiceRoll.h
	g++ -c -g -o build/DiceRoll.o src/controller/DiceRoll.cpp

build/DiceModel.o: src/model/DiceModel.cpp include/DiceModel.h
	g++ -c -g -o build/DiceModel.o src/model/DiceModel.cpp
clean:
	rm build/*
