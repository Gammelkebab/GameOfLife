#ifndef worker_h
#define worker_h

#include "actor.h"

#include <mpi.h>

class Worker : public Actor
{
public:
  Worker(World *world, int proc_num);

private:
  Block *block;
  unsigned char **write_buffers;
  unsigned char **border_send_buffers;
  MPI_Request *border_send_requests;
  MPI_Comm worker_comm;

private:
  void set_worker_comm();

public:
  void tick(int round, int total_rounds);

private:
  void store(int round);
  void communicate();
  void step();

private:
  void border_send(Border_direction dir, unsigned char *buffer); // Send a border, unblocking
  void border_recv(Border_direction dir, unsigned char *buffer); // Receive a border, blocking

public:
  void fill(unsigned char value);
  void glider(int x, int y);
};

#endif
