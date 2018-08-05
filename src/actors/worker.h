#ifndef worker_h
#define worker_h

#include "actor.h"
#include "../world/block.h"

#include <mpi.h>

class Worker : public Actor
{
public:
  Worker(World *world, int proc_num); // Create a new Worker

private:
  Block *block;         // The block this worker works on
  MPI_Comm worker_comm; // The MPI Comm for all active workers

  char **header_write_buffers;          // Header write buffers for every round
  MPI_Request *header_write_requests;   // Header write requests for every round
  unsigned char ***block_write_buffers; // Block write buffers for every round and every row
  MPI_Request **block_write_requests;   // Block write requests for every round and every row
  unsigned char ***border_send_buffers; // Border send buffers for every round and every direction
  MPI_Request **border_send_requests;   // Border send requests for every round and every direction
  MPI_File **file_handles;              // File handles for every round

private:
  void set_worker_comm(); // Set the MPI Comm for all active workers

public:
  void tick(int round); // Do everything necessary for one iteration of Game of Life
  void finalize();

private:
  void store(int round);       // Store this block
  void communicate(int round); // Communicate with other blocks
  void step();                 // Calculate the next state of this block

private:
  MPI_File *open_file(int round);
  int iwrite_header(MPI_File *fh, int round, MPI_Request *write_request);
  void iwrite_block(MPI_File *fh, unsigned char **buffer_compressed, MPI_Request *write_requests, int header_size);

  void border_send(Border_direction dir, unsigned char *buffer, MPI_Request *send_request);
  void border_recv(Border_direction dir, unsigned char *buffer);

public:
  void fill(unsigned char value); // Fill the entire grid with one value
  void glider(int x, int y);      // Create a glider at (x, y)
};

#endif
