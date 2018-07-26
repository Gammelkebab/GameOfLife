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
  int proc_num;

  Active_block *active_block;
  Block ***blocks;

  MPI_Comm active_comm;

private:
  bool master;

private:
  MPI_Request *send_requests;
  MPI_Request *recv_requests;
  bool first_write;

private:
  World(int width, int height, int rows, int cols, int proc_num);

protected:
  World() {}

public:
  static World *create(int width, int height, int proc_amt, int proc_num);

private:
  void set_active_comm();

public:
  void tick(int iteration, int total_iterations);

private:
  void write(int iterationint, int total_iterations);
  void communicate();
  void step();

  void write_to_file(int iteration);
  void load_for_write();

public:
  void print();
  void fill(unsigned char value);
  void glider(int x, int y);
};

#endif