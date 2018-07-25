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
    Dummy_world();

  public:
    void step(int iteration) {}
    void write(int step) {}

  public:
    void print() {}
    void fill(unsigned char value) {}
    void glider(int x, int y) {}

  private:
    void set_active_comm() {}
};

#endif