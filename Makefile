CC=mpicxx
CFLAGS = -g -Wall -Wextra -O3

OBJ := $(wildcard src/*.cpp)
OBJ_MAIN := $(filter-out src/main.cpp, $(OBJ))
OBJ_TEST := $(filter-out src/test.cpp, $(OBJ))
BIN = main test

#first target
main : $(OBJ_MAIN)
	$(CC) $(CFLAGS) -o main $^
	
test_c : $(OBJ_TEST)
	$(CC) $(CFLAGS) -o test $^

run : main
	mpirun -np 4 ./main && ./create_video.sh

test : test_c
	./test

#each object depends on the c-file with the same name
%.o : %.cpp
	$(CC) $(CFLAGS) -c $<

clean :
	rm -rf $(BIN) $(OBJ)
	rm -rf ./images/frame_*
	rm -rf ./output.gif

full : clean main
