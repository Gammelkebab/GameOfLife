#ifndef block_h
#define block_h

#include "border_direction.h"
#include "../helpers/grid.h"

class World;

class Block
{
public:
  Block(World *world, int block_num);
  Block(World *world, int x, int y);

protected:
  World *world;

public:
  int block_num;              // The number of this block (corresponds to the processor)
  int x, y;                   // x and y position of this block in the global block grid
  int starting_x, starting_y; // global x and y position of the first (top left) pixel
  int width, height;          // width and height of this block in pixels
  int width_byte;             // width of this block in byte (compressed pixels)

protected:
  bool first_row, first_col, last_row, last_col; // booleans indicating specific block positions
};

#endif