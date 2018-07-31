#include "block.h"
#include "timing.h"

#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int min(int a, int b)
{
    return a <= b ? a : b;
}

int max(int a, int b)
{
    return a >= b ? a : b;
}

void print_border_direction(Border_direction dir)
{
    switch (dir)
    {
    case NORTH:
        printf("NORTH");
        break;
    case SOUTH:
        printf("SOUTH");
        break;
    case WEST:
        printf("WEST");
        break;
    case EAST:
        printf("EAST");
        break;
    case NORTH_WEST:
        printf("NORTH_WEST");
        break;
    case NORTH_EAST:
        printf("NORTH_EAST");
        break;
    case SOUTH_WEST:
        printf("SOUTH_WEST");
        break;
    case SOUTH_EAST:
        printf("SOUTH_EAST");
        break;
    default:
        printf("NONE");
    }
}

Border_direction opposite_direction(Border_direction dir)
{
    switch (dir)
    {
    case NORTH:
        return SOUTH;
    case SOUTH:
        return NORTH;
    case WEST:
        return EAST;
    case EAST:
        return WEST;
    case NORTH_WEST:
        return SOUTH_EAST;
    case NORTH_EAST:
        return SOUTH_WEST;
    case SOUTH_WEST:
        return NORTH_EAST;
    case SOUTH_EAST:
        return NORTH_WEST;
    default:
        return NORTH;
    }
}

//allocate a 2D array
unsigned char **array2D(int width, int height)
{
    unsigned char **array = new unsigned char *[height]();
    for (int i = 0; i < height; ++i)
    {
        array[i] = new unsigned char[width]();
    }
    return array;
}

//rules for Game of Life
//you can change this function, but the rules have to remain the same
int Block::isAlive(int neighbours, unsigned char cell)
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
int Block::getNeighbours(unsigned char **grid, int x, int y)
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

// TODO the blocks have to be 8-pixel alligned
Block::Block(int block_num, int block_amt, int gridsize_x, int gridsize_y)
{
    world = new World(gridsize_x, gridsize_y, block_amt);

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
    next_grid = array2D(width + 2, height + 2);

    randomize();
}

// TODO
void Block::deleteBlock()
{
    delete[] grid;
    delete[] next_grid;
}

void Block::printBlock(bool print_world)
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
void Block::printGrid()
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
void Block::randomize()
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

void Block::set_bit(unsigned char *data, int position, int value)
{
    *data ^= (-value ^ *data) & (1UL << (7 - position));
}

void Block::buffer_row(unsigned char *buffer, int y)
{
    for (int x = 1; x <= width; x++)
    {
        // Pixel (x, y)
        // write a 0 or 1 into the buffer at the x'th bit
        // x - 1 because of the border

        unsigned char val = grid[y][x] > 0 ? 1 : 0;
        int pos = (x - 1) / 8;
        int bit_pos = (x - 1) % 8;
        set_bit(&buffer[pos], bit_pos, val);
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

void Block::write_grid(MPI_File fh, int header_size)
{
    struct timeval begin;
    gettimeofday(&begin, NULL);

    // Buffer stores one bit for every char -> 1/8 of size, but ceiled
    int buffer_size = ceil(width / 8.0);
    unsigned char buffer[buffer_size];
    buffer[buffer_size - 1] = 0; // Clear last otherwise unwritten pixels in the buffer

    if (x == 0 && y == 0)
    {
        printf("Write Grid - Setup: ");
        print_time_since(begin);
    }
    gettimeofday(&begin, NULL);

    for (int row = 1; row <= height; row++)
    {
        buffer_row(buffer, row);
        // The position of the first pixel of the current row
        // y - 1 because of the border


        int row_width_byte = ceil(world->width / 8.0);
        //printf("rwb: %d\n", row_width_byte);
        int current_row_global = (starting_y + row - 1);
        //printf("crg: %d\n", current_row_global);
        int offset = header_size + ((current_row_global * row_width_byte) + starting_x / 8);

        //printf("starting_x/y: %d, %d\n", starting_x, starting_y);
        //printf("offset: %d\n", offset);
        //printf("Buffer: %d\n", buffer_size);
        //print_unsigned_char_array(buffer, buffer_size);

        /*MPI_File_write_at_all_begin(fh, offset, buffer, buffer_size, MPI_UNSIGNED_CHAR);
        if (y == height)
        {
            MPI_File_write_at_all_end(fh, buffer, MPI_STATUS_IGNORE);
        }*/
        MPI_File_write_at_all(fh, offset, buffer, buffer_size, MPI_UNSIGNED_CHAR, MPI_STATUS_IGNORE);
    }

    if (x == 0 && y == 0)
    {
        printf("Write Grid - Main: ");
        print_time_since(begin);
    }

    for (int y = height + 1; y <= max_height; y++)
    {
        // Empty write so collective write does not block for remaining pixels
        MPI_File_write_at_all(fh, 0, buffer, 0, MPI_UNSIGNED_CHAR, MPI_STATUS_IGNORE);
    }
}

void Block::write(int step_number)
{
    struct timeval begin;
    gettimeofday(&begin, NULL);

    char *filename = new char[100];
    sprintf(filename, "./images/frame_%03d.pbm", step_number);

    MPI_File fh;
    MPI_File_open(active_comm, filename, MPI_MODE_WRONLY | MPI_MODE_CREATE, MPI_INFO_NULL, &fh);

    if (x == 0 && y == 0)
    {
        printf("File open: ");
        print_time_since(begin);
    }

    gettimeofday(&begin, NULL);

    // Create the header lines
    char *header = new char[100];
    sprintf(header, "P4\n%d %d\n", world->width, world->height);
    int header_size = strlen(header);

    // Let thread 0 write the header to file
    if (x == 0 && y == 0)
    {
        MPI_File_write_at(fh, 0, header, header_size, MPI_CHAR, MPI_STATUS_IGNORE);
    }

    if (x == 0 && y == 0)
    {
        printf("Before write: ");
        print_time_since(begin);
    }

    gettimeofday(&begin, NULL);
    write_grid(fh, header_size);

    if (x == 0 && y == 0)
    {
        printf("Write_grid: ");
        print_time_since(begin);
    }

    gettimeofday(&begin, NULL);

    MPI_Barrier(active_comm); // TODO remove barrier
    MPI_File_close(&fh);

    if (x == 0 && y == 0)
    {
        printf("After write: ");
        print_time_since(begin);
    }
}

/* Functions calculating the surrounding block numbers */

int Block::position_to_block_number(int x, int y)
{
    return y * world->cols + x;
}

int Block::north()
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

int Block::south()
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

int Block::west()
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

int Block::east()
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
    return position_to_block_number(other_x, other_y);
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
    return position_to_block_number(other_x, other_y);
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
    return position_to_block_number(other_x, other_y);
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
    return position_to_block_number(other_x, other_y);
}

int Block::neighbour_number(Border_direction dir)
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

void Block::wrap_row(unsigned char *buffer, int row)
{
    for (int x = 1; x <= width; x++)
    {
        buffer[x - 1] = grid[row][x];
    }
    /*printf("row %d Wrapped:\n", row);
    print_unsigned_char_array(buffer, width);
    printf("\n");*/
}

void Block::wrap_col(unsigned char *buffer, int col)
{
    for (int y = 1; y <= height; y++)
    {
        buffer[y - 1] = grid[y][col];
    }
}

void Block::wrap_corner(unsigned char *buffer, int x, int y)
{
    buffer[0] = grid[y][x];
}

void Block::wrap(unsigned char *buffer, Border_direction dir)
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

void Block::unwrap_row(unsigned char *buffer, int row)
{
    for (int x = 1; x <= width; x++)
    {
        grid[row][x] = buffer[x - 1];
    }
}

void Block::unwrap_col(unsigned char *buffer, int col)
{
    for (int y = 1; y <= height; y++)
    {
        grid[y][col] = buffer[y - 1];
    }
}

void Block::unwrap_corner(unsigned char *buffer, int x, int y)
{
    grid[y][x] = buffer[0];
}

void Block::unwrap(unsigned char *buffer, Border_direction dir)
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

int Block::count_by_direction(Border_direction dir)
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

/* send und receive */

void Block::send(Border_direction target_dir, unsigned char *buffer, int element_count)
{
    /*
    printf("(%d, %d) trying to send ", x, y);
    print_border_direction(target_dir);
    printf("\n");
    */
    int target_block = neighbour_number(target_dir);
    int tag = target_dir;
    // TODO blocking send does not work so far, as there might be an uneven amount of blocks in a row/col
    //MPI_Send(buffer, element_count, MPI_UNSIGNED_CHAR, target_block, tag, active_comm);
    MPI_Request request;
    MPI_Isend(buffer, element_count, MPI_UNSIGNED_CHAR, target_block, tag, active_comm, &request);
}

void Block::recv(Border_direction source_dir, unsigned char *buffer, int element_count)
{
    int source_block = neighbour_number(source_dir);
    int tag = opposite_direction(source_dir);
    MPI_Recv(buffer, element_count, MPI_UNSIGNED_CHAR, source_block, tag, active_comm, MPI_STATUS_IGNORE);
}

/**
     * TODO comment
     */
// REMINDER! Use tags for directions of communication, so barriers can be avoided
void Block::communicate()
{
    // create communication buffers
    int buffer_size = max(width, height);
    // TODO as soon as send is blocking again
    //unsigned char buffer[buffer_size];

    unsigned char send_buffer[buffer_size];
    unsigned char recv_buffer[buffer_size];

    // TODO make unblocking send possible
    for (int i = NORTH; i <= SOUTH_EAST; i++)
    {
        Border_direction dir = (Border_direction)i;
        Border_direction opp = opposite_direction(dir);

        int count = count_by_direction(dir);
        wrap(send_buffer, dir);
        send(dir, send_buffer, count);
        recv(opp, recv_buffer, count);
        unwrap(recv_buffer, opp);

        // TODO as soon as send is blocking again
        MPI_Barrier(active_comm);
    }
}

/**
     * next evolution
     * iterate over each cell, count neighbours and apply the rules
     * if a cell is alive/dies depends only on the previous step
     */
void Block::step()
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

void Block::fill(unsigned char val)
{
    {
        for (int x = 1; x <= width; x++)
        {
            for (int y = 1; y <= height; y++)
            {
                grid[y][x] = val;
            }
        }
    }
}

void Block::set_active_comm(MPI_Comm active_comm)
{
    this->active_comm = active_comm;
}

/* 
     * MPI Additions to the normal step:
     * There are 3 parts to every step:
     * 1. write the current grid to memory (first step because it has to be done for the initial random grid)
     * 2. communicate all border pixels
     * 3. do the actual step algorithm
     */
void Block::step_mpi(int step_number)
{
    if (x == -1 || y == -1)
        return;
    struct timeval begin;
    gettimeofday(&begin, NULL);
    write(step_number);
    MPI_Barrier(active_comm);
    if (x == 0 && y == 0)
    {
        printf("Write %03d \t- ", step_number + 1);
        print_time_since(begin);
    }
    gettimeofday(&begin, NULL);
    communicate();
    MPI_Barrier(active_comm);
    if (x == 0 && y == 0)
    {
        printf("Comm. %03d \t- ", step_number + 1);
        print_time_since(begin);
    }
    gettimeofday(&begin, NULL);
    step();
    MPI_Barrier(active_comm);
    if (x == 0 && y == 0)
    {
        printf("Step  %03d \t- ", step_number + 1);
        print_time_since(begin);
    }
}
