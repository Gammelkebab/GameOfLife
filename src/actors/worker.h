#ifndef worker_h
#define worker_h

#include "actor.h"
#include "../world/active_block.h"

#include <mpi.h>

class Worker : public Actor
{
public:
  Worker(World *world, int proc_num); // Create a new Worker

private:
  Active_block *block; // The block this worker works on

private:
  unsigned char **block_send_buffers;
  MPI_Request *block_send_requests;
  unsigned char ***border_send_buffers; // Border send buffers for every round and every direction
  MPI_Request **border_send_requests;   // Border send requests for every round and every direction

public:
  void tick(int round); // Do everything necessary for one iteration of Game of Life
  void finalize();

private:
  void store(int round);       // Store this block
  void communicate(int round); // Communicate with other blocks
  void step();                 // Calculate the next state of this block

private:
  void border_send(Border_direction dir, unsigned char *buffer, MPI_Request *send_request);
  void border_recv(Border_direction dir, unsigned char *buffer);

public:
  void fill(unsigned char value); // Fill the entire grid with one value
  void glider(int x, int y);      // Create a glider at (x, y)
};

#endif
