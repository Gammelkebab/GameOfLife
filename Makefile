CC=mpic++
CFLAGS = -g -Wall -Wextra -O3

OBJ = main.o block.o world.o
BIN = main

#first target
main : $(OBJ)
	$(CC) $(CFLAGS) -o main $^

#each object depends on the c-file with the same name
%.o : %.cpp
	$(CC) $(CFLAGS) -c $<

clean :
	rm -rf $(BIN) $(OBJ)
