#ifndef actor_h
#define actor_h

#include "../world/world.h"
#include <mpi.h>

class Actor
{
public:
  Actor(World *world, int proc_num);
  static Actor *create(int gridsize_x, int gridsize_y, int proc_amt, int proc_num, int total_rounds, double writer_share);

protected:
  World *world;
  int proc_num;
  MPI_Comm worker_comm; // The MPI Comm for all active workers

private:
  void set_worker_comm(); // Set the MPI Comm for all active workers

public:
  virtual void tick(int round) = 0;
  virtual void finalize() = 0;
};

#endif
