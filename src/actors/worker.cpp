#include "worker.h"

#include "../debug/debug.h"

#include "../helpers/timing.h"

Worker::Worker(World *world, int proc_num) : Actor(world, proc_num)
{
    block = new Block(world);
    this->write_send_buffers = (unsigned char **)malloc(world->total_iterations * sizeof(unsigned char *));
    for (int i = 0; i < world->total_iterations; i++)
    {
        this->write_send_buffers[i] = (unsigned char *)malloc(block->compressed_size);
    }

    set_worker_comm();

    world->print();
    fill(0);
    glider(750, 400);
}

void Worker::set_worker_comm()
{
    int active_comm_list[world->worker_amt];
    for (int i = 0; i < world->worker_amt; i++)
    {
        active_comm_list[i] = i;
    }

    MPI_Group world_group, active_group;
    MPI_Comm_group(MPI_COMM_WORLD, &world_group);
    MPI_Group_incl(world_group, world->worker_amt, active_comm_list, &active_group);
    MPI_Comm_create(MPI_COMM_WORLD, active_group, &worker_comm);
}

void Worker::tick(int iteration, int total_iterations)
{
    write(iteration, total_iterations); // Write
    communicate();    // Communicate
    step();           // Step
}

void Worker::fill(unsigned char value)
{
    block->fill(value);
}

void Worker::glider(int x, int y)
{
    using namespace figures;
    figures::glider(grid, x, y);
}

void Worker::write(int iteration)
{
    unsigned char *write_send_buffer = write_send_buffers[iteration];
    int size = grid->compress(write_send_buffer);
    int writer_index = iteration % world->writer_amt;
    MPI_Isend(write_send_buffer, size, MPI_UNSIGNED_CHAR, writer_index, iteration, MPI_COMM_WORLD, write_req)
}

// Communicate borders from and to the active block
void Worker::communicate()
{
}

void Worker::step()
{
}
