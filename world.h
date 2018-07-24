#ifndef world_h
#define world_h

#include "block.h"
#include "active_block.h"

/**
 * Information on the Overall world
 */
class World
{
public:
  int width, height;
  int rows, cols;

  int block_amt;
  Active_block *active_block;
  Block ***blocks;

  MPI_Comm active_comm;

private:
  World(int width, int height, int rows, int cols, int proc_amt, int proc_num);

public:
  static World *create(int width, int height, int proc_amt, int proc_num);
  void step(int iteration);
  void write(int step);

public:
  void print();
  void fill(unsigned char value);
  void glider(int x, int y);

private:
  void set_active_comm();
};

#endif