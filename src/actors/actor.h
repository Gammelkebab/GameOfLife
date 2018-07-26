#ifndef actor_h
#define actor_h

#include "../world/world.h"

class Actor
{
public:
  Actor();
  static Actor *create(int gridsize_x, int gridsize_y, int proc_amt, int proc_num, double worker_share);

private:
  World *world;

public:
  void tick(int round);
};

#endif