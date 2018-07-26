#include <time.h>
#include <sys/time.h>
#include <stdio.h>

// TODO funktioniert das? und wieso nimmt er das falsche?
//#define SMALL //uses small resolution, less iterations for testing

#include "./block.h"
#include "./figures.h"

#define SMALL //uses small resolution, less iterations for testing

#ifdef SMALL
#define GRIDSIZE_X 202
#define GRIDSIZE_Y 200
#define FRAMES 100
#else
#ifdef MEDIUM
#define GRIDSIZE_X 1600
#define GRIDSIZE_Y 900
#define FRAMES 100
#else
#define GRIDSIZE_X 1920
#define GRIDSIZE_Y 1080
#define FRAMES 900
#endif
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

    char processor_name[100];
    int processor_name_length;
    MPI_Get_processor_name(processor_name, &processor_name_length);
    printf("%d \t=> %s\n", block_num, processor_name);

    Block *block = new Block(block_num, block_amt, GRIDSIZE_X, GRIDSIZE_Y);

    int active_comm_amt = block->world->rows * block->world->cols;
    int active_comm_list[active_comm_amt];
    for (int i = 0; i < active_comm_amt; i++)
    {
        active_comm_list[i] = i;
    }

    MPI_Group world_group, active_group;
    MPI_Comm_group(MPI_COMM_WORLD, &world_group);
    MPI_Group_incl(world_group, active_comm_amt, active_comm_list, &active_group);
    MPI_Comm active_comm;
    MPI_Comm_create(MPI_COMM_WORLD, active_group, &active_comm);

    block->set_active_comm(active_comm);

    block->fill(0);
    glider(block->grid, 75, 75);

    for (int i = 0; i < FRAMES; ++i)
    {
        /*
        if (block_num == 0)
        {
            printf("round: %d\n", i + 1);
        }
        */
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
