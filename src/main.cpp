#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <mpi.h>

#include "debug/debug.h"

#include "actors/actor.h"


#define MEDIUM
#define WORKER_SHARE 0.7

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

    int proc_amt, proc_num;

    MPI_Comm_rank(MPI_COMM_WORLD, &proc_num);
    MPI_Comm_size(MPI_COMM_WORLD, &proc_amt);

    if (proc_num == 0)
    {
        printf("proc_amt: %d\n", proc_amt);
    }

    char processor_name[100];
    int processor_name_length;
    MPI_Get_processor_name(processor_name, &processor_name_length);
    printf("%d \t=> %s\n", proc_num, processor_name);

    Actor *actor = Actor::create(GRIDSIZE_X, GRIDSIZE_Y, proc_amt, proc_num, WORKER_SHARE, FRAMES);

    for (int i = 0; i < FRAMES; ++i)
    {
        if (proc_num == 0)
        {
            printf("round: %d of %d\n", i, FRAMES);
        }
        actor->tick(i);
    }

    gettimeofday(&end, NULL);
    elapsed += (end.tv_sec - begin.tv_sec) + ((end.tv_usec - begin.tv_usec) / 1000000.0);
    if (proc_num == 0)
    {
        printf("Runtime: %.5fs\n", elapsed);
    }

    MPI_Finalize();
}
