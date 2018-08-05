#include "world.h"

#include <math.h>
#include <stdio.h>

#include "../debug/debug.h"
#include "../helpers/grid.h"
#include "../helpers/timing.h"

World::World(int width, int height, int proc_amt, int total_rounds) : width(width), height(height), total_rounds(total_rounds)
{
    int worker_amt_tmp = proc_amt;

    // Distributes rows and columns proportionally to the overall pixel aspect ratio
    float rows_tmp = sqrt(worker_amt_tmp * ((float)height / width));
    rows = floor(rows_tmp);
    cols = floor(worker_amt_tmp / rows_tmp);

    // Try to create one more row or column (Might be possible due to flooring)
    int r1 = (rows + 1) * cols;
    int c1 = rows * (cols + 1);
    if (r1 <= proc_amt && c1 <= proc_amt)
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
    else if (r1 <= proc_amt)
    {
        rows++;
    }
    else if (c1 <= proc_amt)
    {
        cols++;
    }

    block_amt = rows * cols;
    worker_amt = block_amt; // There is one worker for every block
}

void World::print()
{
    printf("World:\n{\n");
    printf("\tPixels: %d x %d\n", width, height);
    printf("\tBlocks: %d x %d\n", cols, rows);
    printf("\n}\n");
}

bool World::is_worker(int proc_num)
{
    return proc_num < worker_amt;
}
