#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"

#include "world.h"
#include "dummy_world.h"
#include "array2d.h"
#include "active_block.h"
#include "timing.h"

World *World::create(int width, int height, int proc_amt, int proc_num)
{
    // Distributes rows and columns proportionally to the overall pixel aspect ratio
    float rows_tmp = sqrt(proc_amt * ((float)height / width));
    int rows = floor(rows_tmp);
    int cols = floor(proc_amt / rows_tmp);

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

    if (proc_num < rows * cols)
    {

        return new World(width, height, rows, cols, proc_num);
    }
    else
    {

        return new Dummy_world();
    }
}

World::World(int width, int height, int rows, int cols, int proc_num) : width(width), height(height), rows(rows), cols(cols), proc_num(proc_num)
{
    if (proc_num == 0)
    {
        master = true;
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

    send_requests = new MPI_Request[block_amt];
    recv_requests = new MPI_Request[block_amt];

    send_requests[proc_num] = MPI_REQUEST_NULL;
    recv_requests[proc_num] = MPI_REQUEST_NULL;

    set_active_comm();

    first_write = false;
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
void World::tick(int iteration, int total_iterations)
{
    struct timeval begin;
    gettimeofday(&begin, NULL);
    write(iteration, total_iterations); // Write
    debug("[%d] - write done\n", proc_num);
    if (master)
    {
        printf("Write %03d \t- ", iteration + 1);
        print_time_since(begin);
    }
    gettimeofday(&begin, NULL);
    communicate(); // Communicate
    if (master)
    {
        printf("Comm. %03d \t- ", iteration + 1);
        print_time_since(begin);
    }
    gettimeofday(&begin, NULL);
    step(); // Step
    if (master)
    {
        printf("Step  %03d \t- ", iteration + 1);
        print_time_since(begin);
    }
}

void World::write(int iteration, int total_iterations)
{
    int writing_block_num = iteration % block_amt;
    if (writing_block_num == proc_num) // This thread has to write
    {
        debug("I write\n");
        if (iteration == 0)
        {
            load_for_write(); // Initial load
        }
        debug("Now waiting\n");
        MPI_Waitall(block_amt, recv_requests, MPI_STATUSES_IGNORE); // Wait for all data to be loaded
        debug("Data is now there\n");
        write_to_file(iteration); // File write
        if (iteration < total_iterations - 1)
        {
            load_for_write(); // Normal load
        }
        debug("I Write done");
    }
    else // Someone else has to write
    {
        debug("I send\n");
        // Send the data of the active block to the current writer
        MPI_Request send_request = send_requests[writing_block_num];
        if (send_request)
        {
            MPI_Wait(&send_request, MPI_STATUS_IGNORE);
        }
        active_block->send_for_write(writing_block_num, &send_requests[writing_block_num]);
        debug("Data sent\n");
    }
}

// Communicate borders from and to the active block
void World::communicate()
{
    active_block->communicate();
}

void World::step()
{
    active_block->step();
}

void World::write_to_file(int step)
{
    debug("Trying to write to file\n");
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
    debug("out gets size %d x %d\n", gridsize_x_byte, height);
    char out[gridsize_x_byte * height];
    memset(out, 0, gridsize_x_byte * height);

    debug("Write-setup done\n");

    // Delegate the actual writing process to the blocks
    for (int y = 0; y < rows; y++)
    {
        for (int x = 0; x < cols; x++)
        {
            blocks[y][x]->write(out, gridsize_x_byte);
            debug("[%d, %d] write done\n", y, x);
        }
    }

    debug("Blocks done writing\n");

    // TODO
    //MPI_File_open()
    //MPI_File_iwrite()
    fwrite(&out[0], sizeof(char), gridsize_x_byte * height, outfile);
    fclose(outfile);
}

void World::load_for_write()
{
    for (int y = 0; y < rows; y++)
    {
        for (int x = 0; x < cols; x++)
        {
            int block_num = y * cols + x;
            if (block_num != proc_num)
            {
                blocks[y][x]->load_for_write(&recv_requests[block_num]);
            }
        }
    }
}

void World::print()
{
    printf("World:\n{\n");
    printf("\tProcessor: %d\n", proc_num);
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

void World::fill(unsigned char value)
{
    active_block->fill(value);
}

void World::glider(int x, int y)
{
    active_block->glider(x, y);
}
