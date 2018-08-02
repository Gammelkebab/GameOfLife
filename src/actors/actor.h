#ifndef actor_h
#define actor_h

#include "../world/world.h"

class Actor
{
public:
  Actor(World *world, int proc_num);
  static Actor *create(int gridsize_x, int gridsize_y, int proc_amt, int proc_num, double worker_share, int total_iterations);

protected:
  World *world;
  int proc_num;

public:
  void tick(int round);
};

#endif
