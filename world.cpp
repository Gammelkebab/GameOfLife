#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "world.h"
#include "dummy_world.h"
#include "array2d.h"
#include "active_block.h"

World *World::create(int width, int height, int proc_amt, int proc_num)
{
    // Distributes rows and columns proportionally to the overall pixel aspect ratio
    float rows_tmp = sqrt(proc_amt * ((float)height / width));
    int rows = floor(rows_tmp);
    int cols = floor(proc_amt / rows_tmp);

    if (proc_num < rows * cols)
    {
        return new World(width, height, rows, cols, proc_amt, proc_num);
    }
    else
    {
        return new Dummy_world();
    }
}

World::World(int width, int height, int rows, int cols, int proc_amt, int proc_num) : width(width), height(height)
{
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

    blocks = new Block **[rows];
    for (int i = 0; i < cols; i++)
    {
        blocks[i] = new Block *[rows];
        for (int j = 0; j < cols; j++)
        {
            int block_num = i * rows + j;
            if (block_num == proc_num)
            {
                active_block = new Active_block(this, block_num);
                blocks[i][j] = active_block;
            }
            else
            {
                blocks[i][j] = new Block(this, block_num);
            }
        }
    }

    set_active_comm();
}

void World::set_active_comm()
{
    int active_comm_amt = rows * cols;
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

    this->active_comm = active_comm;
}

/* 
 * MPI Additions to the normal step:
 * There are 3 parts to every step:
 * 1. write the current grid to memory (first step because it has to be done for the initial random grid)
 * 2. communicate all border pixels
 * 3. do the actual step algorithm
 */
void World::step(int iteration)
{
    struct timeval begin;
    gettimeofday(&begin, NULL);
    write(round);
    MPI_Barrier(active_comm);
    if (x == 0 && y == 0)
    {
        printf("Write %03d \t- ", round + 1);
        print_time_since(begin);
    }
    gettimeofday(&begin, NULL);
    communicate(round);
    MPI_Barrier(active_comm);
    if (x == 0 && y == 0)
    {
        printf("Comm. %03d \t- ", round + 1);
        print_time_since(begin);
    }
    gettimeofday(&begin, NULL);
    step();
    MPI_Barrier(active_comm);
    if (x == 0 && y == 0)
    {
        printf("Step  %03d \t- ", round + 1);
        print_time_since(begin);
    }
}

void World::write(int step)
{
    if (inactive)
        return;

    //pbm in mode P4 reads bitwise, 1 black, 0 white
    char *filename = new char[100];
    sprintf(filename, "./images/frame_%03d.pbm", step);
    FILE *outfile = fopen(filename, "w");
    //Create first lines of .pbm layout:
    //mode: P4 (bitwise black white)
    //number of pixels in a row
    //number of rows
    fprintf(outfile, "P4\n%d %d\n", width, height);

    //determine number of bytes in a row
    int gridsize_x_byte = width / 8;
    //if not all bits of a full byte are used (e.g. 10x16 grid)
    //the last bits are ignored
    if (width % 8 != 0)
    {
        gridsize_x_byte++;
    }
    //array holding all bit-information packed in bytes
    char out[gridsize_x_byte * height];
    memset(out, 0, gridsize_x_byte * height);

    // Delegate the actual writing process to the blocks
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            blocks[y][x]->write(out, gridsize_x_byte);
        }
    }

    fwrite(&out[0], sizeof(char), gridsize_x_byte * height, outfile);
    fclose(outfile);
}

void World::print()
{
    printf("World:\n{\n");
    printf("\tPixels: %d x %d\n", width, height);
    printf("\tBlocks: %d x %d\n", cols, rows);
    printf("\n}\n");
}

void World::fill(unsigned char value)
{
    active_block->fill(value);
}
