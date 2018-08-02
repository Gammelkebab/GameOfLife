#ifndef writer_h
#define writer_h

#include "actor.h"
#include "mpi.h"

class Writer : public Actor
{
public:
  Writer(World *world, int proc_num);
  void set_writer_comm();

private:
  MPI_Comm writer_comm;

public:
  void tick(int round);

private:
  void load_for_write(char *write_buffer);
  void write_to_file(char *write_buffer, int round);
};

#endif