#ifndef writer_h
#define writer_h

#include "actor.h"

#include "../helpers/grid.h"

class Writer : public Actor
{
public:
  Writer(World *world, int proc_num);

private:
  char **file_write_buffers;        // Block write buffers for every round and every row
  MPI_Request *file_write_requests; // Block write requests for every round and every row
  MPI_File *file_handles;           // File handles for every round

public:
  void tick(int round);
  void finalize();

private:
  void recv_and_write(int round);
  MPI_File *open_file(int round);
  int iwrite_header(MPI_File *fh, int round);
  void iwrite_block(MPI_File *fh, int header_size, int round);
};

#endif