#ifndef idler_h
#define idler_h

#include "actor.h"

class Idler : public Actor
{
public:
  Idler(World *world, int proc_num);

public:
  void tick(int round);
  void finalize();
};

#endif