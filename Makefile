dice: build/DiceController.o build/DiceView.o
	g++ -g -ltinfo -lncurses -o bin/dice build/DiceView.o build/DiceController.o
build/DiceController.o: src/controller/DiceController.cpp include/DiceController.h
	g++ -c -g -o build/DiceController.o src/controller/DiceController.cpp
build/DiceView.o: src/view/DiceView.cpp include/DiceController.h
	g++ -c -g -ltinfo -lncurses -o build/DiceView.o src/view/DiceView.cpp
clean:
	rm build/*
