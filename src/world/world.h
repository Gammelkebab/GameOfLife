#ifndef world_h
#define world_h

#include "block.h"

/**
 * Information on the Overall world
 */
class World
{
public:
  World(int width, int height, int proc_amt, int total_rounds, double worker_share);

public:
  int width, height;
  int width_byte;
  int rows, cols;

  int block_amt;
  int worker_amt;

  int total_rounds;

  Block ***blocks;

private:
  int writer_amt;
  int *writer_nums;

public:
  void set_blocks();
  bool is_worker(int proc_num);
  int get_writer_num(int round);
  void print();
};

#endif