#include "worker.h"

#include "../debug/debug.h"

#include "../helpers/timing.h"
#include "../world/border_direction.h"

Worker::Worker(World *world, int proc_num) : Actor(world, proc_num)
{
    block = new Block(world);

    write_buffers = new unsigned char *[world->total_rounds];
    border_send_buffers = new unsigned char *[Border_direction_count];
    for (int i = Border_direction_first; i < Border_direction_last; i++)
    {
        Border_direction dir = (Border_direction)i;
        int buffer_size = block->get_border_size_byte(dir);
        border_send_buffers[dir] = new unsigned char[buffer_size];
    }
    border_send_requests = new MPI_Request[Border_direction_count];

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

void Worker::tick(int round, int total_rounds)
{
    store(round);  // Write
    communicate(); // Communicate
    step();        // Step
}

void Worker::store(int round)
{
    unsigned char *write_buffer = write_buffers[round];
    int size = block->compress(&write_buffer);
    MPI_File fh = ...;
    MPI_Request *request = ...;
    MPI_File_iwrite(fh, write_buffer, size, MPI_UNSIGNED_CHAR, request);
}

// Communicate borders from and to this actors block
void Worker::communicate()
{
    for (int i = Border_direction_first; i <= Border_direction_last; i++)
    {
        Border_direction dir = (Border_direction)i;

        unsigned char *border_send_buffer = border_send_buffers[dir];
        block->wrap_border(dir, border_send_buffer);
        border_send(dir, border_send_buffer);
    }

    for (int i = Border_direction_first; i <= Border_direction_last; i++)
    {
        Border_direction dir = (Border_direction)i;

        int border_recv_buffer_size = block->get_border_size_byte(dir);
        unsigned char border_recv_buffer[border_recv_buffer_size];

        border_recv(dir, border_recv_buffer);
        block->unwrap_border(dir, border_recv_buffer);
    }
}

void Worker::step()
{

    int neighbours;
    for (int y = 1; y <= height; y++)
    {
        for (int x = 1; x <= width; x++)
        {
            neighbours = getNeighbours(grid, x, y);
            next_grid[y][x] = isAlive(neighbours, grid[y][x]);
        }
    }

    //swap pointers
    unsigned char **swap = grid;
    grid = next_grid;
    next_grid = swap;
}

/* Border communication */

void Worker::border_send(Border_direction target_dir, unsigned char *buffer)
{
    int element_count = block->get_border_size_byte(target_dir);
    int target_block = block->get_neighbor_block_num(target_dir);
    int tag = target_dir;
    MPI_Request *request = &border_send_requests[target_dir];
    MPI_Isend(buffer, element_count, MPI_UNSIGNED_CHAR, target_block, tag, worker_comm, request);
}

void Worker::border_recv(Border_direction source_dir, unsigned char *buffer)
{
    int element_count = block->get_border_size_byte(source_dir);
    int source_block = block->get_neighbor_block_num(source_dir);
    int tag = opposite_direction(source_dir);
    MPI_Recv(buffer, element_count, MPI_UNSIGNED_CHAR, source_block, tag, worker_comm, MPI_STATUS_IGNORE);
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
