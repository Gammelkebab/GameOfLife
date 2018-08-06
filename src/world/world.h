#ifndef world_h
#define world_h

/**
 * Information on the Overall world
 */
class World
{
public:
  World(int width, int height, int proc_amt, int total_rounds);

public:
  int width, height;
  int width_byte;
  int rows, cols;

  int block_amt;
  int worker_amt;

  int total_rounds;

public:
  void print();

public:
  bool is_worker(int proc_num);
};

#endif