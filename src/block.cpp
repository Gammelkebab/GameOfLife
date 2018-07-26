#include "block.h"
#include "min_max.h"
#include "array2d.h"
#include "world.h"

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
    max_width = width;
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

    // Calculate the maximum width of a block in the world
    if (remainder_x > 8)
    {
        max_width += 8;
    }
    else
    {
        max_width += remainder_x;
    }

    max_width_byte = max_width / 8 + max_width % 8 == 0 ? 0 : 1;

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

    // Calculate the maximum height of a block in the world
    if (remainder_y > 8)
    {
        max_height += 8;
    }
    else
    {
        max_height += remainder_y;
    }

    max_height_byte = max_height / 8 + max_height % 8 == 0 ? 0 : 1;

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
    width_byte = width / 8 + remainder_x == 0 ? 0 : 1;
    height_byte = height / 8 + remainder_y == 0 ? 0 : 1;
    write_grid = array2D(width_byte, height_byte);
}

void Block::write(char *grid_out, int bytes_per_row)
{
    for (int y = 0; y < height_byte; y++)
    {
        for (int x = 0; x < width_byte; x++)
        {
            grid_out[(x + starting_x / 8) * bytes_per_row + (y + starting_y / 8)] = write_grid[y][x];
            // write the compressed and packed data of the write_grid into the compressed but scattered grid_out
            // Pixels are still compressed into bytes, but the data of one block has holes in between, 
            // where parts of the other blocks go
        }
    }
}

void Block::load_for_write(MPI_Request *request)
{
    MPI_Irecv(write_grid, width_byte * height_byte, MPI_UNSIGNED_CHAR, block_num, block_num, world->active_comm, request);
}
