a.out: dice_controller.o dice_view.o
	g++ -g -ltinfo -lncurses dice_controller.o dice_view.o
dice_controller.o:
	g++ -c -g dice_controller.cpp
dice_view.o:
	g++ -c -g -ltinfo -lncurses dice_view.cpp
clean:
	rm *.o 
