#include "block.h"
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

int min(int a, int b)
{
    return a <= b ? a : b;
}

int max(int a, int b)
{
    return a >= b ? a : b;
}

Border_direction opposite_direction(Border_direction dir)
{
    switch (dir)
    {
    case NORTH:
        return SOUTH;
    case SOUTH:
        return NORTH;
    case EAST:
        return WEST;
    case WEST:
        return EAST;
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
    int sum = 0;
    // rows above and below
    for (int i = 0; i < 3; ++i)
    {
        sum += grid[(y + 1) % height][(x - 1 + i + width) % width];
        sum += grid[(y - 1 + height) % height][(x - 1 + i + width) % width];
    }
    // pixels left and right
    sum += grid[y][(x - 1 + width) % width];
    sum += grid[y][(x + 1) % width];
    return sum;
}

Block::Block(int block_num, int block_amt)
{
    world = new World(GRIDSIZE_X, GRIDSIZE_Y, block_amt);

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

    // calculate the pixels assigned to this block
    // takes care of additional pixels from remainder of splitting
    width = world->width / world->cols;
    if (x < world->width % world->cols)
    {
        width++;
    }
    height = world->height / world->rows;
    if (y < world->height % world->rows)
    {
        height++;
    }

    // calculate the position of the first (upper left) pixel assigned to this block
    // takes care of additional pixels from remainder of splitting
    starting_x = x * (world->width / world->cols);
    starting_x += min(x - 1, world->width % world->cols); // add all remainder pixels before this block
    starting_y = y * (world->height / world->rows);
    starting_y += min(y - 1, world->height % world->rows);

    grid = array2D(width + 2, height + 2);
    next_grid = array2D(width + 2, height + 2);

    first_row = x == 0;
    first_col = y == 0;
    last_row = x == world->cols - 1;
    last_col = y == world->rows - 1;

    randomize();
}

// TODO
void Block::deleteBlock()
{
    delete[] grid;
    delete[] next_grid;
}

//console output, small grid is preferable
void Block::printGrid(unsigned char **grid)
{
    // Print the current blocks adress
    printf("Block [%d, %d]:\n", x, y);

    // Print all the assigned pixels without the borders
    for (int y = 1; y <= height; ++y)
    {
        for (int x = 1; x <= width; ++x)
        {
            if (grid[y][x])
                printf("X ");
            else
                printf("- ");
        }
        printf("\n");
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

void Block::write_grid(MPI_File fh)
{
    // TODO
    /*
    for (int w_row = 1; w_row <= height; w_row++)
    {
        MPI_File_write_at_all(fh, offset, buffer, count, MPI_UNSIGNED_CHAR, MPI_STATUS_IGNORE);
    }
    */
}

void Block::write(int step_number)
{
    char *filename = new char[100];
    sprintf(filename, "./images/frame_%03d.pbm", step_number);

    MPI_File fh;
    MPI_File_open(MPI_COMM_WORLD, filename, MPI_MODE_WRONLY | MPI_MODE_CREATE, MPI_INFO_NULL, &fh);
    write_grid(fh);
    MPI_Barrier(MPI_COMM_WORLD); // TODO remove barrier
    MPI_File_close(&fh);
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
    if (first_row)
    {
        if (first_col)
        {
            return position_to_block_number(world->cols, world->rows);
        }
        else
        {
            return position_to_block_number(x - 1, world->rows);
        }
    }
    else
    {
        return position_to_block_number(x - 1, y - 1);
    }
}

int Block::north_east()
{
    if (first_row)
    {
        if (last_col)
        {
            return position_to_block_number(0, world->rows);
        }
        else
        {
            return position_to_block_number(x + 1, world->rows);
        }
    }
    else
    {
        return position_to_block_number(x + 1, y - 1);
    }
}

int Block::south_west()
{
    if (last_row)
    {
        if (first_col)
        {
            return position_to_block_number(world->cols, 0);
        }
        else
        {
            return position_to_block_number(x - 1, 0);
        }
    }
    else
    {
        return position_to_block_number(x - 1, y + 1);
    }
}

int Block::south_east()
{
    if (last_row)
    {
        if (last_col)
        {
            return position_to_block_number(0, 0);
        }
        else
        {
            return position_to_block_number(x + 1, 0);
        }
    }
    else
    {
        return position_to_block_number(x + 1, y + 1);
    }
}

int Block::neighbour_number(Border_direction dir)
{
    switch (dir)
    {
    case NORTH:
        return north();
    case SOUTH:
        return south();
    case EAST:
        return east();
    case WEST:
        return west();
    case NORTH_WEST:
        return north_west();
    case NORTH_EAST:
        return north_east();
    case SOUTH_WEST:
        return south_west();
    case SOUTH_EAST:
        return south_west();
    default:
        return -1;
    }
}

/* Wrap functions */

void Block::wrap_row(char *buffer, int row)
{
    for (int x = 1; x <= width; x++)
    {
        buffer[x - 1] = grid[row][x];
    }
}

void Block::wrap_col(char *buffer, int col)
{
    for (int y = 1; y <= height; y++)
    {
        buffer[y - 1] = grid[y][col];
    }
}

void Block::wrap_corner(char *buffer, int x, int y)
{
    buffer[0] = grid[y][x];
}

void Block::wrap(char *buffer, Border_direction dir)
{
    switch (dir)
    {
    case NORTH:
        wrap_row(buffer, 0);
        break;
    case SOUTH:
        wrap_row(buffer, height + 1);
        break;
    case EAST:
        wrap_col(buffer, 0);
        break;
    case WEST:
        wrap_col(buffer, width + 1);
        break;
    case NORTH_WEST:
        wrap_corner(buffer, 0, 0);
        break;
    case NORTH_EAST:
        wrap_corner(buffer, width + 1, 0);
        break;
    case SOUTH_WEST:
        wrap_corner(buffer, 0, height + 1);
        break;
    case SOUTH_EAST:
        wrap_corner(buffer, width + 1, height + 1);
        break;
    }
}

/* Unwrap functions */

void Block::unwrap_row(char *buffer, int row)
{
    for (int x = 1; x <= width; x++)
    {
        grid[row][x] = buffer[x - 1];
    }
}

void Block::unwrap_col(char *buffer, int col)
{
    for (int y = 1; y <= height; y++)
    {
        grid[y][col] = buffer[y - 1];
    }
}

void Block::unwrap_corner(char *buffer, int x, int y)
{
    grid[y][x] = buffer[0];
}

void Block::unwrap(char *buffer, Border_direction dir)
{
    switch (dir)
    {
    case NORTH:
        unwrap_row(buffer, 0);
        break;
    case SOUTH:
        unwrap_row(buffer, height + 1);
        break;
    case EAST:
        unwrap_col(buffer, 0);
        break;
    case WEST:
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

void Block::send(Border_direction target_dir, char *buffer, int element_count)
{
    int target_block = neighbour_number(target_dir);
    int tag = target_dir;
    MPI_Send(buffer, element_count, MPI_UNSIGNED_CHAR, target_block, tag, MPI_COMM_WORLD);
}

void Block::recv(Border_direction source_dir, char *buffer, int element_count)
{
    int source_block = neighbour_number(source_dir);
    int tag = opposite_direction(source_dir);
    MPI_Recv(buffer, element_count, MPI_UNSIGNED_CHAR, source_block, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
}

/**
     * TODO comment
     */
// REMINDER! Use tags for directions of communication, so barriers can be avoided
void Block::communicate()
{
    // create communication buffers
    int buffer_size = max(width, height);
    char buffer[buffer_size];

    // TODO make unblocking send possible
    for (int i = NORTH; i <= SOUTH_EAST; i++)
    {
        Border_direction dir = (Border_direction)i;
        Border_direction opp = opposite_direction(dir);

        if (dir == NORTH || dir == SOUTH)
        {
            if (y % 2 == 0)
            {
                int count = count_by_direction(dir);
                wrap(buffer, dir);
                send(dir, buffer, count);
                recv(opp, buffer, count);
                unwrap(buffer, opp);
            }
            else
            {
                int count = count_by_direction(dir);
                recv(opp, buffer, count);
                unwrap(buffer, opp);
                wrap(buffer, dir);
                send(dir, buffer, count);
            }
        }
        else
        {
            if (x % 2 == 0)
            {
                int count = count_by_direction(dir);
                wrap(buffer, dir);
                send(dir, buffer, count);
                recv(opp, buffer, count);
                unwrap(buffer, opp);
            }
            else
            {
                int count = count_by_direction(dir);
                recv(opp, buffer, count);
                unwrap(buffer, opp);
                wrap(buffer, dir);
                send(dir, buffer, count);
            }
        }
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
    for (int y = 1; y <= height; ++y)
    {
        for (int x = 1; x <= width; ++x)
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

/* 
     * MPI Additions to the normal step:
     * There are 3 parts to every step:
     * 1. write the current grid to memory (first step because it has to be done for the initial random grid)
     * 2. communicate all border pixels
     * 3. do the actual step algorithm
     */
void Block::step_mpi(int step_number)
{
    write(step_number);
    communicate();
    step();
}

//creates a .pbm file displaying the current grid
//pbm in mode P4 reads bitwise, 1 black, 0 white
void writeFrame(unsigned char **grid, int i)
{
    char *filename = new char[100];
    sprintf(filename, "./images/frame_%03d.pbm", i);
    FILE *outfile = fopen(filename, "w");
    //Create first lines of .pbm layout:
    //mode: P4 (bitwise black white)
    //number of pixels in a row
    //number of rows
    fprintf(outfile, "P4\n%d %d\n", GRIDSIZE_X, GRIDSIZE_Y);

    //determine number of bytes in a row
    int gridsize_x_byte = GRIDSIZE_X / 8;
    //if not all bits of a full byte are used (e.g. 10x16 grid)
    //the last bits are ignored
    if (GRIDSIZE_X % 8 != 0)
    {
        gridsize_x_byte++;
    }
    //array holding all bit-information packed in bytes
    //char out[gridsize_x_byte*GRIDSIZE_Y] = {0};
    char out[gridsize_x_byte * GRIDSIZE_Y];
    memset(out, 0, gridsize_x_byte * GRIDSIZE_Y * sizeof(char));

    for (int y = 0; y < GRIDSIZE_Y; ++y)
    {
        for (int x = 0; x < gridsize_x_byte; ++x)
        {
            //information about 8 cells are packed in one byte
            for (int k = 0; k < 8; ++k)
            {
                if (x * 8 + k >= GRIDSIZE_X)
                    break;
                if (grid[y][x * 8 + k] == 1)
                {
                    out[y * gridsize_x_byte + x] += pow(2, 7 - k);
                }
            }
        }
    }
    fwrite(&out[0], sizeof(char), gridsize_x_byte * GRIDSIZE_Y, outfile);
    fclose(outfile);
}
