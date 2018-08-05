#ifndef block_h
#define block_h

#include "world.h"
#include "border_direction.h"
#include "../helpers/grid.h"

class Block
{
public:
  Block(World *world, int proc_num);

private:
  /**
   * @brief A grid of unsigned chars to hold all pixels assigned to this block.
   * 
   * Each pixel has a full unsigned char,
   * although there are only 2 possible values for each pixel:
   * 1 = alive, 0 = dead
   * 
   * Although the overall grid has no borders and instead just wraps around, 
   * the grid for this block does have borders to store information about the surrounding blocks.
   * 
   *     |<--width-->|
   *   0 | 1 1 0 1 0 | 1
   *   --+-----------+-------
   *   1 | 0 1 1 1 0 | 1   ^
   *   1 | 0 1 0 1 0 | 1   |
   *   1 | 1 0 1 0 1 | 1 height
   *   1 | 1 1 0 0 1 | 1   |
   *   1 | 1 0 1 0 1 | 1   V
   *   --+-----------+-------
   *   1 | 0 0 1 1 0 | 0
   */
  Grid grid;
  Grid next_grid; // Used for caluclating the next step and then swapping

  World *world;

public:
  int block_num;              // The number of this block (corresponds to the processor)
  int x, y;                   // x and y position of this block in the global block grid
  int starting_x, starting_y; // global x and y position of the first (top left) pixel
  int width, height;          // width and height of this block in pixels
  int width_byte;             // width of this block in byte (compressed pixels)

private:
  bool first_row, first_col, last_row, last_col; // booleans indicating specific block positions

public:
  void randomize();

  void compress(unsigned char **buffer);

  int get_border_size_byte(Border_direction dir);
  int get_neighbor_block_num(Border_direction dir);
  void wrap_border(Border_direction dir, unsigned char *buffer);
  void unwrap_border(Border_direction dir, unsigned char *buffer);

  void step();

private:
  int position_to_block_num(int x, int y);
  int neighbor_num(Border_direction dir);

  int north();
  int south();
  int west();
  int east();
  int north_west();
  int north_east();
  int south_west();
  int south_east();

  void wrap_row(int row, unsigned char *buffer);
  void wrap_col(int col, unsigned char *buffer);
  void wrap_corner(int x, int y, unsigned char *buffer);
  void unwrap_row(int row, unsigned char *buffer);
  void unwrap_col(int col, unsigned char *buffer);
  void unwrap_corner(int x, int y, unsigned char *buffer);

  int get_neighbors(int pixel_x, int pixel_y);
  int is_alive(int neighbours, unsigned char cell);

public:
  void fill(unsigned char value);
  void glider(int x, int y);

public:
  void print();
  void print_grid();
  void print_unsigned_char_array(unsigned char *arr, int size);
};

#endif