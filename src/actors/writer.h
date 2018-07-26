#ifndef writer_h
#define writer_h

#include "actor.h"

class Writer : public Actor
{
  public:
    Writer();

  public:
    void tick(int round);
};

#endif