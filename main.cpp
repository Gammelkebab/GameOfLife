#include <time.h>
#include <sys/time.h>
#include <stdio.h>

// TODO funktioniert das? und wieso nimmt er das falsche?
//#define SMALL //uses small resolution, less iterations for testing

#include "./block.h"
#include "./figures.h"

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

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    //time measurement
    double elapsed = 0;
    struct timeval begin, end;

    gettimeofday(&begin, NULL);

    int block_amt, block_num;

    MPI_Comm_rank(MPI_COMM_WORLD, &block_num);
    MPI_Comm_size(MPI_COMM_WORLD, &block_amt);

    if (block_num == 0)
    {
        printf("block_amt: %d\n", block_amt);
    }

    // gettimeofday(&begin, NULL);

    Block *block = new Block(block_num, block_amt, GRIDSIZE_X, GRIDSIZE_Y);

    /*
    for (int i = 0; i < 3; ++i)
    {
        //block->printGrid();
        block->write(i);
        if (i == 0)
        {
            block->fill(0);
        }
        if (i == 1)
        {
            block->fill(1);
        }
    }
    */

    block->fill(0);
    glider(block->grid, 25, 25);

    for (int i = 0; i < FRAMES; ++i)
    {
        if (block_num == 0)
        {
            printf("round: %d\n", i + 1);
        }
        block->step_mpi(i);
    }

    gettimeofday(&end, NULL);
    elapsed += (end.tv_sec - begin.tv_sec) + ((end.tv_usec - begin.tv_usec) / 1000000.0);
    if (block_num == 0)
    {
        printf("Runtime: %.5fs\n", elapsed);
    }

    MPI_Finalize();
}
