#ifndef dummy_world_h
#define dummy_world_h

#include "block.h"
#include "active_block.h"

/**
 * Information on the Overall world
 */
class Dummy_world : public World
{
public:
  Dummy_world() : World() {}
  void step() {}
  void print() {}
  void fill(unsigned char value)
  {
    (void)value;
  }
  void glider(int x, int y)
  {
    (void)x;
    (void)y;
  }
};

#endif