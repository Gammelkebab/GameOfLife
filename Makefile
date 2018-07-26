CC=mpicxx
CFLAGS = -g -Wall -Wextra -O3

SRC_DIR = src
OBJ_DIR = obj

SRC = $(wildcard $(SRC_DIR)/*.cpp) $(wildcard $(SRC_DIR)/**/*.cpp)
OBJ = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRC))
OBJ_MAIN = $(filter-out obj/test.o, $(OBJ))
OBJ_TEST = $(filter-out obj/main.o, $(OBJ))
BIN = bin/main bin/test

#first target
main : $(OBJ_MAIN)
	$(CC) $(CFLAGS) -o bin/main $^
	
test_c : $(OBJ_TEST)
	$(CC) $(CFLAGS) -o bin/test $^

run : main
	mpirun -np 4 ./main && ./create_video.sh

test : test_c
	./test

#each object depends on the c-file with the same name
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CC) $(CFLAGS) -c -o $@ $<

clean :
	rm -rf $(BIN) $(OBJ)
	rm -rf ./images/frame_*
	rm -rf ./output.gif

full : clean main
