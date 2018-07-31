#ifndef active_block_h
#define active_block_h

#include "block.h"
#include "border_direction.h"

#include <mpi.h>

class World;

class Active_block : public Block
{
public:
  Active_block(World *world, int block_num);
  ~Active_block();

private:
  int getNeighbours(unsigned char **grid, int x, int y);
  int isAlive(int neighbours, unsigned char cell);

private:
  Grid grid;      // The actual data of all the assigned pixels
                  // REMEMBER! this grid has the borders stored as well
                  // => it has size (width + 2, height + 2)
                  // => the 'real' pxiels go from (1, 1) up to (width, height)
  Grid next_grid; // Next grid used to calculate step

private:
  char **send_block_buffers; // Grid used to store data to be send

public:
  void printGrid();

  //fill the grid randomly with ~35% alive
  void randomize();

  void buffer_row(unsigned char *buffer, int row);

  /* Functions calculating the surrounding block numbers */
  int position_to_block_number(int x, int y);
  int neighbour_number(Border_direction dir);
  int north();
  int south();
  int west();
  int east();
  int north_west();
  int north_east();
  int south_west();
  int south_east();

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
  void send_border(Border_direction target_dir, unsigned char *buffer);
  void recv_border(Border_direction source_dir, unsigned char *buffer);

  /**
     * TODO comment
     */
  // REMINDER! Use tags for directions of communication, so barriers can be avoided
  void communicate_borders();
  void communicate();

  /**
     * next evolution
     * iterate over each cell, count neighbours and apply the rules
     * if a cell is alive/dies depends only on the previous step
     */
  void step();

  void store_grid_compressed(char *target);                  // Loads the information of this block into another grid
                                                             // The information about 8 Pixels is compressed into 1 byte
  void load_for_write();                                     // Loads the information of this block into the write_grid
                                                             // The information about 8 Pixels is compressed into 1 byte
  void send_for_write(int target_num, MPI_Request *request); // Sends the information of this block to another thread
                                                             // The information about 8 Pixels is compressed into 1 byte

  /* 
     * MPI Additions to the normal step:
     * There are 3 parts to every step:
     * 1. write the current grid to memory (first step because it has to be done for the initial random grid)
     * 2. communicate all border pixels
     * 3. do the actual step algorithm
     */
  void step_mpi(int round);

  void set_bit(unsigned char *data, int position, int val);

public:
  void fill(unsigned char val);
  void glider(int x, int y);
};

#endif