CC=mpicxx
CFLAGS = -g -Wall -Wextra -O3

OBJ = main.o block.o world.o test.o
OBJ_MAIN = main.o block.o world.o
OBJ_TEST = test.o block.o world.o
BIN = main

#first target
main : $(OBJ_MAIN)
	$(CC) $(CFLAGS) -o main $^
	
test_c : $(OBJ_TEST)
	$(CC) $(CFLAGS) -o test $^

run : main
	mpirun -np 1 ./main && ./create_video.sh

test : test_c
	./test

#each object depends on the c-file with the same name
%.o : %.cpp
	$(CC) $(CFLAGS) -c $<

clean :
	rm -rf $(BIN) $(OBJ)

full : clean main
	rm -rf ./output.gif
	rm -rf ./images/frame_*
