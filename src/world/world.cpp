#include "world.h"

#include <math.h>
#include <stdio.h>

#include "../debug/debug.h"
#include "../helpers/array2d.h"
#include "../helpers/timing.h"

World::World(int width, int height, int proc_amt, int proc_num, double worker_share, int total_iterations) : width(width), height(height), total_iterations(total_iterations)
{
    // Initial worker amount is set to the approximate worker share percentage of all processes
    // We do not wann all or no threads to be workers
    // After calculating the number of rows and columns,
    // the resulting number of blocks is never larger than the temporarily set worker amount
    int worker_amt_tmp = btw(proc_amt * worker_share, 1, proc_amt - 1);

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
    worker_amt = block_amt;             // There is one worker for every block
    writer_amt = proc_num - worker_amt; // All remaining threads are writers

    blocks = new Block **[rows];
    for (int y = 0; y < rows; y++)
    {
        blocks[y] = new Block *[cols];
        for (int x = 0; x < cols; x++)
        {
            int block_num = y * cols + x;
            blocks[y][x] = new Block(this, block_num);
        }
    }
}

void World::print()
{
    printf("World:\n{\n");
    printf("\tPixels: %d x %d\n", width, height);
    printf("\tBlocks: %d x %d\n", cols, rows);
    printf("\n}\n");
    for (int y = 0; y < rows; y++)
    {
        for (int x = 0; x < cols; x++)
        {
            blocks[y][x]->print();
        }
    }
}

bool World::is_worker(int proc_num)
{
    return proc_num < worker_amt;
}
