#ifndef block_h
#define block_h

#include "world.h"
#include <mpi.h>

typedef enum Border_direction
{
	NORTH,
	SOUTH,
	EAST,
	WEST,
	NORTH_WEST,
	NORTH_EAST,
	SOUTH_WEST,
	SOUTH_EAST
} Border_direction;

Border_direction opposite_direction(Border_direction dir);

unsigned char **array2D(int width, int height);

class Block
{
  private:
	typedef unsigned char **Grid;

  private:
	int getNeighbours(unsigned char **grid, int x, int y);
	int isAlive(int neighbours, unsigned char cell);

  public:
	World *world;

  public:
	int x, y;									   // x and y position of the block (in blocks not pixels)
	int width, height;							   // height and width of the block (without borders)
	int max_height;								   // The maximum height between all blocks (used for collective write)
	int starting_x, starting_y;					   // upper left corner x and y position (pixels)
	bool first_row, first_col, last_row, last_col; // booleans indicating specific block positions

  public:
	Grid grid; // The actual data of all the assigned pixels
			   // REMEMBER! this grid has the borders stored as well
			   // => it has size (width + 2, height + 2)
			   // => the 'real' pxiels go from (1, 1) up to (width, height)
	Grid next_grid;

  public:
	MPI_Comm active_comm;

  public:
	Block(int block_num, int block_amt, int gridsize_x, int gridsize_y);

	// TODO
	void deleteBlock();

  public:
	void printBlock(bool print_world = true);
	//console output, small grid is preferable
	void printGrid();

	//fill the grid randomly with ~35% alive
	void randomize();

	void buffer_row(unsigned char *buffer, int row);
	void write_grid(MPI_File fh, int header_size);
	void write(int step_number);

	/* Functions calculating the surrounding block numbers */

	int position_to_block_number(int x, int y);

	int north();
	int south();
	int west();
	int east();
	int north_west();
	int north_east();
	int south_west();
	int south_east();

	int neighbour_number(Border_direction dir);

	/* Wrap functions */
	void wrap_row(unsigned char *buffer, int row);
	void wrap_col(unsigned char *buffer, int col);
	void wrap_corner(unsigned char *buffer, int x, int y);
	void wrap(unsigned char *buffer, Border_direction dir);

	/* Unwrap functions */
	void unwrap_row(unsigned char *buffer, int row);
	void unwrap_col(unsigned char *buffer, int col);
	void unwrap_corner(unsigned char *buffer, int x, int y);
	void unwrap(unsigned char *buffer, Border_direction dir);

	/* element count */
	int count_by_direction(Border_direction dir);

	/* send und receive */
	void send(Border_direction target_dir, unsigned char *buffer, int element_count);
	void recv(Border_direction source_dir, unsigned char *buffer, int element_count);

	/**
     * TODO comment
     */
	// REMINDER! Use tags for directions of communication, so barriers can be avoided
	void communicate();

	/**
     * next evolution
     * iterate over each cell, count neighbours and apply the rules
     * if a cell is alive/dies depends only on the previous step
     */
	void step();

	void fill(unsigned char val);

	/* 
     * MPI Additions to the normal step:
     * There are 3 parts to every step:
     * 1. write the current grid to memory (first step because it has to be done for the initial random grid)
     * 2. communicate all border pixels
     * 3. do the actual step algorithm
     */
	void step_mpi(int step_number);

	void set_bit(unsigned char *data, int position, int val);
	void set_active_comm(MPI_Comm active_comm);
};

#endif