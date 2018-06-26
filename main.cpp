#include <time.h>
#include <sys/time.h>

// TODO funktioniert das? und wieso nimmt er das falsche?
//#define SMALL //uses small resolution, less iterations for testing

#include "./world.h"
#include "./block.h"
// #include "./figures.h"

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

    // gettimeofday(&begin, NULL);

    Block *block = new Block(block_num, block_amt);

    for (int i = 0; i < FRAMES; ++i)
    {
        block->write(i);
        //block->step_mpi(i);
    }

    gettimeofday(&end, NULL);
    elapsed += (end.tv_sec - begin.tv_sec) + ((end.tv_usec - begin.tv_usec) / 1000000.0);
    if (block_num == 0)
    {
        printf("Runtime: %.5fs\n", elapsed);
    }

    MPI_Finalize();
}
