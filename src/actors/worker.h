#ifndef worker_h
#define worker_h

#include "actor.h"

#include <mpi.h>

class Worker : public Actor
{
public:
  Worker(World *world, int proc_num);
  void Worker::set_worker_comm();

private:
  Block *block;
  unsigned char **write_send_buffers;
  MPI_Request

private:
  MPI_Comm worker_comm;

public:
  void tick(int round, int total_iterations);

private:
  void write(int iteration, int total_iterations);
  void communicate();
  void step();

public:
  void fill(unsigned char value);
  void glider(int x, int y);
};

#endif
