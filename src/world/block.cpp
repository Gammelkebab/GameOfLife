#include "block.h"

#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

#include "../debug/debug.h"

#include "../helpers/figures.h"
#include "../helpers/min_max.h"

// TODO group nearby blocks on same cluster node

Block::Block(World *world, int proc_num) : world(world), block_num(proc_num)
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
    //max_width = width;

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

    // Calculate the maximum width of a block in the world
    if (remainder_x > 8)
    {
        //max_width += 8;
    }
    else
    {
        //max_width += remainder_x;
    }

    // the same for the height
    height = world->height / world->rows;
    height -= height % 8;
    //max_height = height;
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
        //max_height += 8;
    }
    else
    {
        //max_height += remainder_y;
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
    //height_byte = height / 8 + (height % 8 == 0 ? 0 : 1);

    //max_width_byte = max_width / 8 + (max_width % 8 == 0 ? 0 : 1);
    //max_height_byte = max_height / 8 + (max_height % 8 == 0 ? 0 : 1);

    debug3("Block %d done with setup calculations.\n", proc_num);

    grid = create_grid(width + 2, height + 2);
    next_grid = create_grid(width + 2, height + 2);

    debug3("Block %d done with creating grid buffers.\n", proc_num);

    randomize();

    debug3("Block %d done randomizing.\n", proc_num);
}

/**
 * @brief Fill the grid randomly with ~35% alive
 */
void Block::randomize()
{
    srand(time(NULL));
    for (int x = 1; x <= width; ++x)
    {
        for (int y = 1; y <= height; ++y)
        {
            debug4("Randomizing (%d, %d).\n", x, y);
            grid[y][x] = (int)rand() % 100 < 35 ? 1 : 0;
            debug4("done.\n");
        }
    }
}

void Block::compress(unsigned char **buffer)
{
    for (int row = 0; row < height; row++)
    {
        memset(buffer[row], 0, width_byte);
    }
    for (int grid_y = 1; grid_y <= height; grid_y++)
    {
        for (int grid_x = 1; grid_x <= width; grid_x++)
        {
            /* Information about 8 cells are packed into one byte.
             * If the number of pixels in a row is not divisible by 8, 
             * the remaining bits are left empty.
             */
            int buffer_x = (grid_x - 1) / 8;
            int buffer_y = grid_y - 1;
            int k = (grid_x - 1) % 8;
            if (grid[grid_y][grid_x] == 1)
            {
                buffer[buffer_y][buffer_x] |= 1 << (7 - k);
            }
        }
    }
}

int Block::get_border_size(Border_direction dir)
{
    if (dir == NORTH || dir == SOUTH)
    {
        return width;
    }
    else if (dir == EAST || dir == WEST)
    {
        return height;
    }
    else
    {
        return 1;
    }
}

int Block::get_neighbor_block_num(Border_direction dir)
{
    switch (dir)
    {
    case NORTH:
        return north();
    case SOUTH:
        return south();
    case WEST:
        return west();
    case EAST:
        return east();
    case NORTH_WEST:
        return north_west();
    case NORTH_EAST:
        return north_east();
    case SOUTH_WEST:
        return south_west();
    case SOUTH_EAST:
        return south_east();
    default:
        return -1;
    }
}

void Block::wrap_border(Border_direction dir, unsigned char *buffer)
{
    switch (dir)
    {
    case NORTH:
        wrap_row(1, buffer);
        break;
    case SOUTH:
        wrap_row(height, buffer);
        break;
    case WEST:
        wrap_col(1, buffer);
        break;
    case EAST:
        wrap_col(width, buffer);
        break;
    case NORTH_WEST:
        wrap_corner(1, 1, buffer);
        break;
    case NORTH_EAST:
        wrap_corner(width, 1, buffer);
        break;
    case SOUTH_WEST:
        wrap_corner(1, height, buffer);
        break;
    case SOUTH_EAST:
        wrap_corner(width, height, buffer);
        break;
    }
}

void Block::unwrap_border(Border_direction dir, unsigned char *buffer)
{
    switch (dir)
    {
    case NORTH:
        unwrap_row(0, buffer);
        break;
    case SOUTH:
        unwrap_row(height + 1, buffer);
        break;
    case WEST:
        unwrap_col(0, buffer);
        break;
    case EAST:
        unwrap_col(width + 1, buffer);
        break;
    case NORTH_WEST:
        unwrap_corner(0, 0, buffer);
        break;
    case NORTH_EAST:
        unwrap_corner(width + 1, 0, buffer);
        break;
    case SOUTH_WEST:
        unwrap_corner(0, height + 1, buffer);
        break;
    case SOUTH_EAST:
        unwrap_corner(width + 1, height + 1, buffer);
        break;
    }
}

void Block::step()
{
    for (int y = 1; y <= height; y++)
    {
        for (int x = 1; x <= width; x++)
        {
            int neighbours;
            neighbours = get_neighbors(x, y);
            next_grid[y][x] = is_alive(neighbours, grid[y][x]);
        }
    }

    //swap pointers
    unsigned char **swap = grid;
    grid = next_grid;
    next_grid = swap;
}

/**
 * @brief Count neighbors that are alive in a 2D grid.
 * There are 9 neighbors for each cell.
 * 
 * @param pixel_x 
 * @param pixel_y 
 * @return int 
 */
int Block::get_neighbors(int pixel_x, int pixel_y)
{
    int sum = 0;
    // Pixels in row above and below
    for (int i = 0; i < 3; ++i)
    {
        sum += grid[pixel_y - 1][pixel_x - 1 + i];
        sum += grid[pixel_y + 1][pixel_x - 1 + i];
    }
    // Pixels left and right
    sum += grid[pixel_y][pixel_x - 1];
    sum += grid[pixel_y][pixel_x + 1];
    return sum;
}

/**
 * @brief Rules for Game of Life
 * - Keep state with 2 living neighbors
 * - Come alive with 3 living neighbors
 * - Die otherwise
 * 
 * @param neighbours 
 * @param cell 
 * @return int 
 */
int Block::is_alive(int neighbours, unsigned char cell)
{
    switch (neighbours)
    {
    case (2):
        return cell;
    case (3):
        return 1;
    default:
        return 0;
    }
}

/* Dummy data */

void Block::fill(unsigned char value)
{
    for (int y = 1; y <= height; y++)
    {
        for (int x = 1; x <= width; x++)
        {
            grid[y][x] = value;
        }
    }
}

void Block::glider(int x, int y)
{
    debug4("Creating glider at (%d, %d) for block %d", x, y, block_num);
    using namespace figures;
    figures::glider(grid, x, y);
}

/* Helpers */

/* Functions calculating the surrounding block numbers */

int Block::position_to_block_num(int x, int y)
{
    return (y * world->cols + x);
}

int Block::north()
{
    if (first_row)
    {
        // above first row is last row
        return position_to_block_num(x, world->rows - 1);
    }
    else
    {
        return position_to_block_num(x, y - 1);
    }
}

int Block::south()
{
    if (last_row)
    {
        // below last row is first row
        return position_to_block_num(x, 0);
    }
    else
    {
        return position_to_block_num(x, y + 1);
    }
}

int Block::west()
{
    if (first_col)
    {
        // left from first col is last col
        return position_to_block_num(world->cols - 1, y);
    }
    else
    {
        return position_to_block_num(x - 1, y);
    }
}

int Block::east()
{
    if (last_col)
    {
        // right from last col is first col
        return position_to_block_num(0, y);
    }
    else
    {
        return position_to_block_num(x + 1, y);
    }
}

int Block::north_west()
{
    int other_x, other_y;
    if (first_col)
    {
        other_x = world->cols - 1;
    }
    else
    {
        other_x = x - 1;
    }
    if (first_row)
    {
        other_y = world->rows - 1;
    }
    else
    {
        other_y = y - 1;
    }
    return position_to_block_num(other_x, other_y);
}

int Block::north_east()
{
    int other_x, other_y;
    if (last_col)
    {
        other_x = 0;
    }
    else
    {
        other_x = x + 1;
    }
    if (first_row)
    {
        other_y = world->rows - 1;
    }
    else
    {
        other_y = y - 1;
    }
    return position_to_block_num(other_x, other_y);
}

int Block::south_west()
{
    int other_x, other_y;
    if (first_col)
    {
        other_x = world->cols - 1;
    }
    else
    {
        other_x = x - 1;
    }
    if (last_row)
    {
        other_y = 0;
    }
    else
    {
        other_y = y + 1;
    }
    return position_to_block_num(other_x, other_y);
}

int Block::south_east()
{
    int other_x, other_y;
    if (last_col)
    {
        other_x = 0;
    }
    else
    {
        other_x = x + 1;
    }
    if (last_row)
    {
        other_y = 0;
    }
    else
    {
        other_y = y + 1;
    }
    return position_to_block_num(other_x, other_y);
}

/* Wrap functions */

void Block::wrap_row(int row, unsigned char *buffer)
{
    for (int x = 1; x <= width; x++)
    {
        buffer[x - 1] = grid[row][x];
    }
}

void Block::wrap_col(int col, unsigned char *buffer)
{
    for (int y = 1; y <= height; y++)
    {
        buffer[y - 1] = grid[y][col];
    }
}

void Block::wrap_corner(int x, int y, unsigned char *buffer)
{
    buffer[0] = grid[y][x];
}

/* Unwrap functions */

void Block::unwrap_row(int row, unsigned char *buffer)
{
    for (int x = 1; x <= width; x++)
    {
        grid[row][x] = buffer[x - 1];
    }
}

void Block::unwrap_col(int col, unsigned char *buffer)
{
    for (int y = 1; y <= height; y++)
    {
        grid[y][col] = buffer[y - 1];
    }
}

void Block::unwrap_corner(int x, int y, unsigned char *buffer)
{
    grid[y][x] = buffer[0];
}

/* Print */

void Block::print()
{
    printf("Block:\n{\n");
    printf("\tPosition: \t(%d, %d)\n", x, y);
    printf("\tPixel size: \t%d x %d\n", width, height);
    printf("\tWidth Byte: \t%d\n", width_byte);
    printf("\tPixel start: \t(%d x %d)\n", starting_x, starting_y);
    printf("\tSpecial Info:\n");
    printf("\t{\n");
    printf("\t\tFirst Row: \t%c\n", first_row ? '+' : '0');
    printf("\t\tLast Row: \t%c\n", last_row ? '+' : '0');
    printf("\t\tFirst Col: \t%c\n", first_col ? '+' : '0');
    printf("\t\tLast Col: \t%c\n", last_col ? '+' : '0');
    printf("\t}\n");
    printf("}\n");
}
//console output, small grid is preferable
void Block::print_grid()
{
    // Print the current blocks adress
    printf("Block [%d, %d]:\n", x, y);

    // Print all the assigned pixels without the borders
    for (int y = 0; y < height + 2; y++)
    {
        if (y == width + 1)
        {
            printf("\n");
        }
        for (int x = 0; x < width + 2; x++)
        {
            if (x == width + 1)
            {
                printf("\t");
            }
            if (grid[y][x])
            {
                printf("X");
            }
            else
            {
                printf("-");
            }
            if (x == 0)
            {
                printf("\t");
            }
        }
        printf("\n");
        if (y == 0)
        {
            printf("\n");
        }
    }
    printf("\n\n");
}

void print_unsigned_char_array(unsigned char *arr, int size)
{
    printf("[");
    for (int i = 0; i < size; i++)
    {
        if (i > 0)
        {
            printf(", ");
        }
        printf("%d", arr[i]);
    }
    printf("]\n");
}
