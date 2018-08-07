#include "active_block.h"

#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

#include "world.h"
#include "../helpers/figures.h"

Active_block::Active_block(World *world, int proc_num) : Block(world, proc_num)
{
    grid = create_grid(width + 2, height + 2);
    next_grid = create_grid(width + 2, height + 2);

    randomize();
}

/**
 * @brief Fill the grid randomly with ~35% alive
 */
void Active_block::randomize()
{
    srand(time(NULL) * block_num);
    for (int x = 1; x <= width; x++)
    {
        for (int y = 1; y <= height; y++)
        {

            grid[y][x] = (int)rand() % 100 < 35 ? 1 : 0;
        }
    }
}

void Active_block::compress(unsigned char *buffer)
{
    memset(buffer, 0, width * height);
    for (int grid_y = 1; grid_y <= height; grid_y++)
    {
        for (int grid_x = 1; grid_x <= width; grid_x++)
        {
            /* Information about 8 cells are packed into one byte.
             * If the number of pixels in a row is not divisible by 8, 
             * the remaining bits are left empty.
             */
            if (grid[grid_y][grid_x] == 1)
            {
                int buffer_x = (grid_x - 1) / 8;
                int buffer_y = grid_y - 1;
                int k = (grid_x - 1) % 8;
                int buffer_pos = buffer_y * width_byte + buffer_x;
                buffer[buffer_pos] |= 1 << (7 - k);
            }
        }
    }
}

int Active_block::get_border_size(Border_direction dir)
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

int Active_block::get_neighbor_block_num(Border_direction dir)
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

void Active_block::wrap_border(Border_direction dir, unsigned char *buffer)
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

void Active_block::unwrap_border(Border_direction dir, unsigned char *buffer)
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

void Active_block::step()
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
int Active_block::get_neighbors(int pixel_x, int pixel_y)
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
int Active_block::is_alive(int neighbours, unsigned char cell)
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

void Active_block::fill(unsigned char value)
{
    for (int y = 1; y <= height; y++)
    {
        for (int x = 1; x <= width; x++)
        {
            grid[y][x] = value;
        }
    }
}

void Active_block::glider(int x, int y)
{

    using namespace figures;
    figures::glider(grid, x, y);
}

/* Helpers */

/* Functions calculating the surrounding block numbers */

int Active_block::position_to_block_num(int x, int y)
{
    return (y * world->cols + x);
}

int Active_block::north()
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

int Active_block::south()
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

int Active_block::west()
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

int Active_block::east()
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

int Active_block::north_west()
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

int Active_block::north_east()
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

int Active_block::south_west()
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

int Active_block::south_east()
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

void Active_block::wrap_row(int row, unsigned char *buffer)
{
    for (int x = 1; x <= width; x++)
    {
        buffer[x - 1] = grid[row][x];
    }
}

void Active_block::wrap_col(int col, unsigned char *buffer)
{
    for (int y = 1; y <= height; y++)
    {
        buffer[y - 1] = grid[y][col];
    }
}

void Active_block::wrap_corner(int x, int y, unsigned char *buffer)
{
    buffer[0] = grid[y][x];
}

/* Unwrap functions */

void Active_block::unwrap_row(int row, unsigned char *buffer)
{
    for (int x = 1; x <= width; x++)
    {
        grid[row][x] = buffer[x - 1];
    }
}

void Active_block::unwrap_col(int col, unsigned char *buffer)
{
    for (int y = 1; y <= height; y++)
    {
        grid[y][col] = buffer[y - 1];
    }
}

void Active_block::unwrap_corner(int x, int y, unsigned char *buffer)
{
    grid[y][x] = buffer[0];
}

/* Print */

void Active_block::print()
{
    printf("Active_block:\n{\n");
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
void Active_block::print_grid()
{
    // Print the current blocks adress
    printf("Active_block [%d, %d]:\n", x, y);

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