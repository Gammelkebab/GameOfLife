#include "world.h"

#include <math.h>
#include <stdio.h>

World::World(int width, int height, int block_amt) : width(width), height(height)
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

void World::print()
{
    printf("World:\n{\n");
    printf("\tPixels: %d x %d\n", width, height);
    printf("\tBlocks: %d x %d\n", cols, rows);
    printf("\n}\n");
}