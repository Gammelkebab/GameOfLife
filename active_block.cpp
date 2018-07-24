#include "active_block.h"
#include "timing.h"
#include "array2d.h"

#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

Active_block::Active_block(World *world, int block_num) : Block(world, block_num)
{
    next_grid = array2D(width + 2, height + 2);
    grid_to_write = array2D(world->width, world->height);
    send_block_buffers = malloc((world->block_amt - 1) * sizeof(Grid));
    for (int i = 0; i < world->block_amt; i++)
    {
        send_block_buffers[i] = array2D(width, height);
    }

    randomize();
}

Active_block::~Active_block()
{
    // TODO
    delete[] grid;
    delete[] next_grid;
}

//rules for Game of Life
//you can change this function, but the rules have to remain the same
int Active_block::isAlive(int neighbours, unsigned char cell)
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

//count neighbours that are alive in a 2D grid
//there are 9 neighbours for each cell
//the grid has no real border, neighbours above top row is the bottom row
//alive = 1, dead = 0
int Active_block::getNeighbours(unsigned char **grid, int x, int y)
{
    if (x == 0 || x == width + 1 || y == 0 || y == height + 1)
    {
        printf("Trying to get neighbours of border pixel (%d, %d). Returning 2.", x, y);
        return 2;
    }
    int sum = 0;
    // pixels in row above and below
    for (int i = 0; i < 3; ++i)
    {
        sum += grid[y - 1][x - 1 + i];
        sum += grid[y + 1][x - 1 + i];
    }
    // pixels left and right
    sum += grid[y][x - 1];
    sum += grid[y][x + 1];
    return sum;
}

void Active_block::printBlock(bool print_world)
{
    if (print_world)
    {
        world->print();
    }

    printf("Block:\n{\n");
    printf("\tPosition: \t(%d, %d)\n", x, y);
    printf("\tPixel size: \t%d x %d\n", width, height);
    printf("\tMaximum height: \t%d\n", max_height);
    printf("\tPixel start: \t(%d x %d)\n", starting_x, starting_y);
    printf("\tSpecial Info:\n");
    printf("\t{\n");
    printf("\t\tFirst Row: \t%c\n", first_row ? '+' : '0');
    printf("\t\tLast Row: \t%c\n", last_row ? '+' : '0');
    printf("\t\tFirst Col: \t%c\n", first_col ? '+' : '0');
    printf("\t\tLast Col: \t%c\n", last_col ? '+' : '0');
    printf("\t}\n");
    printf("\n}\n");
}

//console output, small grid is preferable
void Active_block::printGrid()
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

//fill the grid randomly with ~35% alive
void Active_block::randomize()
{
    srand(time(NULL));
    for (int x = 1; x <= width; ++x)
    {
        for (int y = 1; y <= height; ++y)
        {
            grid[y][x] = (int)rand() % 100 < 35 ? 1 : 0;
        }
    }
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

/* Functions calculating the surrounding block numbers */

int Active_block::position_to_block_number(int x, int y)
{
    return y * world->cols + x;
}

int Active_block::north()
{
    if (first_row)
    {
        // above first row is last row
        return position_to_block_number(x, world->rows - 1);
    }
    else
    {
        return position_to_block_number(x, y - 1);
    }
}

int Active_block::south()
{
    if (last_row)
    {
        // below last row is first row
        return position_to_block_number(x, 0);
    }
    else
    {
        return position_to_block_number(x, y + 1);
    }
}

int Active_block::west()
{
    if (first_col)
    {
        // left from first col is last col
        return position_to_block_number(world->cols - 1, y);
    }
    else
    {
        return position_to_block_number(x - 1, y);
    }
}

int Active_block::east()
{
    if (last_col)
    {
        // right from last col is first col
        return position_to_block_number(0, y);
    }
    else
    {
        return position_to_block_number(x + 1, y);
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
    return position_to_block_number(other_x, other_y);
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
    return position_to_block_number(other_x, other_y);
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
    return position_to_block_number(other_x, other_y);
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
    return position_to_block_number(other_x, other_y);
}

int Active_block::neighbour_number(Border_direction dir)
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

/* Wrap functions */

void Active_block::wrap_row(unsigned char *buffer, int row)
{
    for (int x = 1; x <= width; x++)
    {
        buffer[x - 1] = grid[row][x];
    }
}

void Active_block::wrap_col(unsigned char *buffer, int col)
{
    for (int y = 1; y <= height; y++)
    {
        buffer[y - 1] = grid[y][col];
    }
}

void Active_block::wrap_corner(unsigned char *buffer, int x, int y)
{
    buffer[0] = grid[y][x];
}

void Active_block::wrap(unsigned char *buffer, Border_direction dir)
{
    switch (dir)
    {
    case NORTH:
        wrap_row(buffer, 1);
        break;
    case SOUTH:
        wrap_row(buffer, height);
        break;
    case WEST:
        wrap_col(buffer, 1);
        break;
    case EAST:
        wrap_col(buffer, width);
        break;
    case NORTH_WEST:
        wrap_corner(buffer, 1, 1);
        break;
    case NORTH_EAST:
        wrap_corner(buffer, width, 1);
        break;
    case SOUTH_WEST:
        wrap_corner(buffer, 1, height);
        break;
    case SOUTH_EAST:
        wrap_corner(buffer, width, height);
        break;
    }
}

/* Unwrap functions */

void Active_block::unwrap_row(unsigned char *buffer, int row)
{
    for (int x = 1; x <= width; x++)
    {
        grid[row][x] = buffer[x - 1];
    }
}

void Active_block::unwrap_col(unsigned char *buffer, int col)
{
    for (int y = 1; y <= height; y++)
    {
        grid[y][col] = buffer[y - 1];
    }
}

void Active_block::unwrap_corner(unsigned char *buffer, int x, int y)
{
    grid[y][x] = buffer[0];
}

void Active_block::unwrap(unsigned char *buffer, Border_direction dir)
{
    switch (dir)
    {
    case NORTH:
        unwrap_row(buffer, 0);
        break;
    case SOUTH:
        unwrap_row(buffer, height + 1);
        break;
    case WEST:
        unwrap_col(buffer, 0);
        break;
    case EAST:
        unwrap_col(buffer, width + 1);
        break;
    case NORTH_WEST:
        unwrap_corner(buffer, 0, 0);
        break;
    case NORTH_EAST:
        unwrap_corner(buffer, width + 1, 0);
        break;
    case SOUTH_WEST:
        unwrap_corner(buffer, 0, height + 1);
        break;
    case SOUTH_EAST:
        unwrap_corner(buffer, width + 1, height + 1);
        break;
    }
}

/* element count */

int Active_block::count_by_direction(Border_direction dir)
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

/* send and receive */

void Active_block::send_border(Border_direction target_dir, unsigned char *buffer)
{
    int element_count = count_by_direction(target_dir);
    int target_block = neighbour_number(target_dir);
    int tag = target_dir;
    // TODO blocking send does not work so far, as there might be an uneven amount of blocks in a row/col
    //MPI_Send(buffer, element_count, MPI_UNSIGNED_CHAR, target_block, tag, active_comm);
    MPI_Request request;
    MPI_Isend(buffer, element_count, MPI_UNSIGNED_CHAR, target_block, tag, active_comm, &request);
}

void Active_block::recv_border(Border_direction source_dir, unsigned char *buffer)
{
    int element_count = count_by_direction(source_dir);
    int source_block = neighbour_number(source_dir);
    int tag = opposite_direction(source_dir);
    MPI_Recv(buffer, element_count, MPI_UNSIGNED_CHAR, source_block, tag, active_comm, MPI_STATUS_IGNORE);
}

/**
 * TODO comment
 */
void Active_block::communicate_borders()
{
    // create communication buffers
    int buffer_size = max(width, height);

    // TODO as soon as send is blocking again
    //unsigned char buffer[buffer_size];

    unsigned char send_buffer[buffer_size];
    unsigned char recv_buffer[buffer_size];

    // TODO make blocking send possible
    for (int i = NORTH; i <= SOUTH_EAST; i++)
    {
        Border_direction dir = (Border_direction)i;
        Border_direction opp = opposite_direction(dir);

        wrap(send_buffer, dir);
        send_border(dir, send_buffer);
        recv_border(opp, recv_buffer);
        unwrap(recv_buffer, opp);

        // TODO as soon as send is blocking again
        MPI_Barrier(active_comm);
    }
}

void Active_block::communicate_for_write(int round)
{
    int writer_block = round % world->block_amt;

    if (writer_block == block_num)
    {
        // It's this threads turn to write
        for (int sender = 0; sender < active_blocks; sender++)
        {
            receive_block(send_block);
        }
        write(round);
    }
    else
    {
        // It's someone elses turn to write
        // Someone needs the info about this block
        send_block(writer_block);
    }
}

void Active_block::communicate(int round)
{
    communicate_borders();
    communicate_for_write(round);
}

/**
     * next evolution
     * iterate over each cell, count neighbours and apply the rules
     * if a cell is alive/dies depends only on the previous step
     */
void Active_block::step()
{
    int neighbours;
    for (int y = 1; y <= height; y++)
    {
        for (int x = 1; x <= width; x++)
        {
            neighbours = getNeighbours(grid, x, y);
            next_grid[y][x] = isAlive(neighbours, grid[y][x]);
        }
    }

    //swap pointers
    unsigned char **swap = grid;
    grid = next_grid;
    next_grid = swap;
}

void Active_block::fill(unsigned char value)
{
    {
        for (int x = 1; x <= width; x++)
        {
            for (int y = 1; y <= height; y++)
            {
                grid[y][x] = value;
            }
        }
    }
}

void Active_block::glider(int x, int y)
{
    using namespace figures;
    figures::glider(grid, x, y);
}
