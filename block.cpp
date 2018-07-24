#include "block.h"
#include "min_max.h"
#include "array2d.h"

// TODO the blocks have to be 8-pixel alligned
Block::Block(World *world, int block_num)
{
    this->world = world;

    this->block_num = block_num;

    // TODO group nearby blocks on same cluster node
    if (block_num >= world->rows * world->cols)
    {
        // this block does not have any pixels assigned to it
        x = -1;
        y = -1;
    }
    else
    {
        x = block_num % world->cols;
        y = block_num / world->cols;
    }

    first_row = y == 0;
    first_col = x == 0;
    last_row = y == world->rows - 1;
    last_col = x == world->cols - 1;

    // calculate the pixels assigned to this block
    // takes care of 8 bit allignment
    // takes care of additional pixels from remainder of splitting
    width = world->width / world->cols;
    width -= width % 8;
    int remainder_x = world->width % (world->cols * 8); // the remainder after assigning full 8 bit blocks
                                                        // this value is always < world->cols * 8
    // distribute all full 8 bit blocks of the remainder to the first blocks,
    // then the remaining bits to the last block
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
    max_height = height;
    int remainder_y = world->height % (world->rows * 8);
    if (y < remainder_y / 8)
    {
        height += 8;
    }
    if (last_row)
    {
        height += remainder_y % 8;
    }

    if (remainder_y > 8)
    {
        max_height += 8;
    }
    else
    {
        max_height += remainder_y;
    }

    // calculate the position of the first (upper left) pixel assigned to this block
    // takes care of additional pixels from remainder of splitting
    starting_x = world->width / world->cols;
    starting_x -= starting_x % 8;
    starting_x *= x;
    starting_x += min(remainder_x / 8, x) * 8; // add all remainder pixels before this block
    starting_y = world->height / world->rows;
    starting_y -= starting_y % 8;
    starting_y *= y;
    starting_y += min(remainder_y / 8, y) * 8;

    grid = array2D(width + 2, height + 2);
}

void Block::write(char *grid_out, int bytes_per_row)
{
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            //information about 8 cells are packed in one byte
            for (int k = 0; k < 8; ++k)
            {
                if ((x + starting_x) * 8 + k >= bytes_per_row)
                {
                    break;
                }
                if (grid[y + starting_y][(x + starting_x) * 8 + k] == 1)
                {
                    grid_out[(y + starting_y) * bytes_per_row + (x + starting_x)] += pow(2, 7 - k);
                }
            }
        }
    }
}

void Block::send_block()
{
    MPI_Isend(grid, );
}

void Block::recv_block()
{
    MPI_Irecv(grid, )
}
