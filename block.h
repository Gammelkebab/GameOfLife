#ifndef block_h
#define block_h

#include "world.h"
#include <mpi.h>

typedef unsigned char **Grid;

class Block
{
  public:
	Block(World *world, int block_num);

  protected:
	World *world;

  protected:
	int block_num;
	int x, y;									   // x and y position of the block (in blocks not pixels)
	int width, height;							   // height and width of the block (without borders)
	int max_height;								   // The maximum height between all blocks (used for collective write)
	int starting_x, starting_y;					   // upper left corner x and y position (pixels)
	bool first_row, first_col, last_row, last_col; // booleans indicating specific block positions

  protected:
	Grid grid; // The actual data of all the assigned pixels
		// REMEMBER! this grid has the borders stored as well
		// => it has size (width + 2, height + 2)
		// => the 'real' pxiels go from (1, 1) up to (width, height)

  public:
	// Delegate methods
	void write(char *grid, int bytes_per_row);
	void send_block();
	void recv_block();
};

#endif