#ifndef worker_h
#define worker_h

#include "actor.h"

class Worker : public Actor
{
  public:
    Worker();

  public:
    void tick(int round);
};

#endif