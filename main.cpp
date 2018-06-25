#include <stdlib.h>
#include <stdio.h>
#include <algorithm>
#include <time.h>
#include <sys/time.h>
#include <math.h>

#include <cstring>

#include <mpi.h>

#define SMALL //uses small resolution, less iterations for testing

#ifndef SMALL
#define GRIDSIZE_X 1920
#define GRIDSIZE_Y 1080
#define FRAMES 900
#else
#define GRIDSIZE_X 202
#define GRIDSIZE_Y 200
#define FRAMES 100
#endif

using namespace std;

unsigned char **array2D(int width, int height);
int getNeighbours(unsigned char **grid, int x, int y);
int isAlive(int neighbours, unsigned char cell);

typedef unsigned char **Grid;

/**
 * Information on the Overall world
 */
class World
{
  public:
    int width, height;
    int rows, cols;

  public:
    World(int width, int height, int block_amt) : width(width), height(height)
    {
        // Distributes rows and columns proportionally to the overall pixel aspect ratio
        float rows_tmp = sqrt(block_amt * ((float)height / width));
        rows = floor(rows_tmp);
        cols = floor(block_amt / rows_tmp);

        // Try to create one more row or column (Might be possible due to flooring)
        int r1 = (rows + 1) * cols;
        int c1 = rows * (cols + 1);
        if (r1 <= block_amt && c1 <= block_amt)
        {
            if (r1 > c1)
            {
                rows++;
            }
            else
            {
                cols++;
            }
        }
        else if (r1 <= block_amt)
        {
            rows++;
        }
        else if (c1 <= block_amt)
        {
            cols++;
        }
    }
};

class Block
{
  public:
    World *world;

  public:
    int x, y;                   // x and y position of the block (in blocks not pixels)
    int width, height;          // height and width of the block
    int starting_x, starting_y; // upper left corner x and y position (pixels)

  public:
    Grid grid; // The actual data of all the assigned pixels
               // REMEMBER! this grid has the borders stored as well
               // => it has size (width + 2, height + 2)
               // => the 'real' pxiels go from (1, 1) up to (width, height)
    Grid next_grid;

  public:
    Block(int block_num, int block_amt)
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

        grid = array2D(GRIDSIZE_X, GRIDSIZE_Y);
        next_grid = array2D(GRIDSIZE_X, GRIDSIZE_Y);

        randomize();
    }

    // TODO
    void deleteBlock()
    {
        delete[] grid;
        delete[] next_grid;
    }

  public:
    //console output, small grid is preferable
    void printGrid(unsigned char **grid)
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
    void randomize()
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

    void write() {}

    void send(int target_block, int tag, char *buffer) {}
    void recv(int source_block, int tag, char *buffer) {}
    void wrap(char *buffer) {}

    /**
     * 
     */
    // REMINDER! Use tags for directions of communication, so barriers can be avoided
    void communicate()
    {
        int buffer_size = 0;
        char buffer[buffer_size];
        wrap(buffer);
        send(x + 1, 0, buffer);
    }

    /**
     * next evolution
     * iterate over each cell, count neighbours and apply the rules
     * if a cell is alive/dies depends only on the previous step
     */
    void step()
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
    void step_mpi()
    {
        write();
        communicate();
        step();
    }
};

/*
int min(int x, int y)
{
    return x < y ? x : y;
}
*/

//rules for Game of Life
//you can change this function, but the rules have to remain the same
int isAlive(int neighbours, unsigned char cell)
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
int getNeighbours(unsigned char **grid, int x, int y)
{
    int sum = 0;
    for (int i = 0; i < 3; ++i)
    {
        sum += grid[(y + 1) % GRIDSIZE_Y][(x - 1 + i + GRIDSIZE_X) % GRIDSIZE_X];
        sum += grid[(y - 1 + GRIDSIZE_Y) % GRIDSIZE_Y][(x - 1 + i + GRIDSIZE_X) % GRIDSIZE_X];
    }
    sum += grid[y][(x - 1 + GRIDSIZE_X) % GRIDSIZE_X];
    sum += grid[y][(x + 1) % GRIDSIZE_X];
    return sum;
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

/**
the figures are ignoring the size of the grid, offset has to be checked manually
**/

//figure with a 2-step period, stationary
void blinker(unsigned char **grid, int offset_X = 0, int offset_Y = 0)
{
    grid[3 + offset_Y][4 + offset_X] = 1;
    grid[4 + offset_Y][4 + offset_X] = 1;
    grid[5 + offset_Y][4 + offset_X] = 1;
}

//figure with a 8-step period, moves
void glider(unsigned char **grid, int offset_X = 0, int offset_Y = 0)
{
    grid[4 + offset_Y][4 + offset_X] = 1;
    grid[4 + offset_Y][5 + offset_X] = 1;
    grid[4 + offset_Y][6 + offset_X] = 1;
    grid[3 + offset_Y][6 + offset_X] = 1;
    grid[2 + offset_Y][5 + offset_X] = 1;
}

//figure
void pentadecathlon(unsigned char **grid, int offset_X = 0, int offset_Y = 0)
{
    for (int x = 0; x < 8; ++x)
    {
        for (int y = 0; y < 3; ++y)
        {
            grid[y + offset_Y][x + offset_X] = 1;
        }
    }
    grid[1 + offset_Y][1 + offset_X] = 0;
    grid[1 + offset_Y][6 + offset_X] = 0;
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

int main(int argc, char **argv)
{
    //time measurement
    double elapsed = 0;
    struct timeval begin, end;

    gettimeofday(&begin, NULL);

    MPI_Init(&argc, &argv);
    {
        int block_amt, block_num;

        MPI_Comm_rank(MPI_COMM_WORLD, &block_num);
        MPI_Comm_size(MPI_COMM_WORLD, &block_amt);

        // gettimeofday(&begin, NULL);

        Block *block = new Block(block_num, block_amt);

        for (int i = 0; i < FRAMES; ++i)
        {
            block->step_mpi();
        }

        // TODO delete block
    }
    MPI_Finalize();

    gettimeofday(&end, NULL);
    elapsed += (end.tv_sec - begin.tv_sec) + ((end.tv_usec - begin.tv_usec) / 1000000.0);
    printf("Runtime: %.5fs\n", elapsed);
}
