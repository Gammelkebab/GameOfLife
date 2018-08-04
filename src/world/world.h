#ifndef world_h
#define world_h

#include "block.h"

/**
 * Information on the Overall world
 */
class World
{
public:
  World(int width, int height, int proc_amt, int proc_num, int total_rounds);

public:
  int width, height;
  int rows, cols;

  int block_amt;
  int worker_amt, writer_amt;

  int total_rounds;

public:
  void print();

public:
  bool is_worker(int proc_num);
};

#endif