#ifndef active_block_h
#define active_block_h

#include "world.h"
#include "block.h"
#include "border_direction.h"

#include <mpi.h>

class Active_block : public Block
{
public:
  Active_block(World *world, int block_num);
  ~Active_block();

private:
  int getNeighbours(unsigned char **grid, int x, int y);
  int isAlive(int neighbours, unsigned char cell);

private:
  Grid next_grid; // Next grid used to calculate step

private:
  Grid *send_block_buffers; // Grid used to store data to be send
  Grid grid_to_write;       // Grid used to store data to be written

private:
  MPI_Comm active_comm;

public:
  void printBlock(bool print_world = true);
  void printGrid();

  //fill the grid randomly with ~35% alive
  void randomize();

  void buffer_row(unsigned char *buffer, int row);
  void write_grid(MPI_File fh, int header_size);

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
  void Active_block::wrap_row(unsigned char *buffer, int row);
  void Active_block::wrap_col(unsigned char *buffer, int col);
  void Active_block::wrap_corner(unsigned char *buffer, int x, int y);
  void wrap(unsigned char *buffer, Border_direction dir);

  /* Unwrap functions */
  void Active_block::unwrap_row(unsigned char *buffer, int row);
  void Active_block::unwrap_col(unsigned char *buffer, int col);
  void Active_block::unwrap_corner(unsigned char *buffer, int x, int y);
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
  void communicate_for_write(int round);
  void communicate(int round);

  /**
     * next evolution
     * iterate over each cell, count neighbours and apply the rules
     * if a cell is alive/dies depends only on the previous step
     */
  void step();

  /* 
     * MPI Additions to the normal step:
     * There are 3 parts to every step:
     * 1. write the current grid to memory (first step because it has to be done for the initial random grid)
     * 2. communicate all border pixels
     * 3. do the actual step algorithm
     */
  void step_mpi(int round);

  void set_bit(unsigned char *data, int position, int val);
  void set_active_comm(MPI_Comm active_comm);

public:
  void fill(unsigned char val);
  void glider(int x, int y);
};

#endif