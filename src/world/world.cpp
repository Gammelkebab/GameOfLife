#include "world.h"

#include <math.h>
#include <stdio.h>

#include "../debug/debug.h"
#include "../helpers/grid.h"
#include "../helpers/timing.h"
#include "../helpers/min_max.h"

World::World(int width, int height, int proc_amt, int total_rounds, double worker_share) : width(width), height(height), total_rounds(total_rounds)
{
    width_byte = width / 8 + (width % 8 == 0 ? 0 : 1);

    // Never have noone or everyone at work
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
    worker_amt = block_amt; // There is one worker for every block
    writer_amt = proc_amt - worker_amt;

    // TODO
    // This enables scattering the writers amongst cores,
    // as long as the processor-core-distribution is known
    writer_nums = new int[writer_amt];
    for (int w = 0; w < writer_amt; w++)
    {
        writer_nums[w] = w + worker_amt; // Put writers behind workers
    }
}

void World::set_blocks()
{
    blocks = new Block **[rows];
    for (int row = 0; row < rows; row++)
    {
        blocks[row] = new Block *[cols];
        for (int col = 0; col < cols; col++)
        {
            int block_num = row * cols + col;
            blocks[row][col] = new Block(this, block_num);
        }
    }
}

bool World::is_worker(int proc_num)
{
    // Processor proc_num is a worker,
    // as long as it does not appear in the writer array
    for (int w = 0; w < writer_amt; w++)
    {
        if (writer_nums[w] == proc_num)
        {
            return false;
        }
    }
    return true;
}

int World::get_writer_num(int round)
{
    return writer_nums[round % writer_amt];
}

void World::print()
{
    printf("World:\n{\n");
    printf("\tPixels: \t%d x %d\n", width, height);
    printf("\tBlocks: \t%d x %d\n", cols, rows);
    printf("\tWorkers: \t%d\n", worker_amt);
    printf("\tTotal rounds: \t%d\n", total_rounds);
    printf("\n}\n");
}
