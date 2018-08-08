#include "block.h"

#include "../debug/debug.h"

#include "../helpers/figures.h"
#include "../helpers/min_max.h"
#include "world.h"

// TODO group nearby blocks on same cluster node

Block::Block(World *world, int block_num) : world(world), block_num(block_num)
{
    x = block_num % world->cols;
    y = block_num / world->cols;

    first_row = y == 0;
    first_col = x == 0;
    last_row = y == world->rows - 1;
    last_col = x == world->cols - 1;

    /* Calculate the pixels assigned to this block
     * - Takes care of 8 bit allignment
     * - Takes care of additional pixels from remainder of splitting
     */
    width = world->width / world->cols;
    width -= width % 8;

    /*
     * The remainder after assigning full 8 bit blocks
     * This value is always < world->cols * 8
     * distribute all full 8 bit blocks of the remainder to the first blocks, 
     * then the remaining bits to the last block
     */
    int remainder_x = world->width % (world->cols * 8);
    if (x < remainder_x / 8)
    {
        width += 8;
    }
    if (last_col)
    {
        width += remainder_x % 8;
    }

    // the same for the height
    height = world->height / world->rows;
    height -= height % 8;
    int remainder_y = world->height % (world->rows * 8);
    if (y < remainder_y / 8)
    {
        height += 8;
    }
    if (last_row)
    {
        height += remainder_y % 8;
    }

    // calculate the position of the first (upper left) pixel assigned to this block
    // takes care of additional pixels from remainder of splitting
    starting_x = world->width / world->cols;
    starting_x -= starting_x % 8;
    starting_x *= x;
    starting_x += min(remainder_x / 8, x) * 8; // add all horizontal remainder pixels before this block
    starting_y = world->height / world->rows;
    starting_y -= starting_y % 8;
    starting_y *= y;
    starting_y += min(remainder_y / 8, y) * 8; // add all vertical remainder pixels before this block

    // Create the write buffer grid, containing compressed information about the block
    // 8 Pixels are compressed into one byte
    width_byte = width / 8 + (width % 8 == 0 ? 0 : 1);
}
