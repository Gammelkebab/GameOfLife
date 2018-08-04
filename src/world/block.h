#ifndef block_h
#define block_h

#include "world.h"
#include "border_direction.h"

class Block
{
public:
  Block(World *world);

public:
  int compress(unsigned char **buffer_place);
  int get_border_size_byte(Border_direction dir);
  int get_neighbor_block_num(Border_direction dir);
  void wrap_border(Border_direction dir, unsigned char *buffer);
  void unwrap_border(Border_direction dir, unsigned char *buffer);
};

#endif