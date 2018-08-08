#include "worker.h"

#include <string.h>

#include "../debug/debug.h"
#include "../helpers/timing.h"

#include "../world/border_direction.h"
#include "../world/active_block.h"

Worker::Worker(World *world, int proc_num) : Actor(world, proc_num)
{
    block = new Active_block(world, proc_num);

    /* Borders */
    border_send_buffers = new unsigned char **[world->total_rounds];
    for (int r = 0; r < world->total_rounds; r++)
    {
        border_send_buffers[r] = new unsigned char *[Border_direction_count];
        for (int i = Border_direction_first; i <= Border_direction_last; i++)
        {
            Border_direction dir = (Border_direction)i;
            int buffer_size = block->get_border_size(dir);
            border_send_buffers[r][dir] = new unsigned char[buffer_size];
        }
    }
    border_send_requests = new MPI_Request *[world->total_rounds];
    for (int r = 0; r < world->total_rounds; r++)
    {
        border_send_requests[r] = new MPI_Request[Border_direction_count];
    }

    /* Block sends */
    block_send_buffers = new unsigned char *[world->total_rounds];
    for (int r = 0; r < world->total_rounds; r++)
    {
        block_send_buffers[r] = new unsigned char[block->width_byte * block->height];
    }
    block_send_requests = new MPI_Request[world->total_rounds];
}

void Worker::tick(int round)
{
    //timeval t;
    //start_timer(&t);

    store(round);
    //print_time_since("Store", &t);
    //start_timer(&t);

    communicate(round);
    //print_time_since("Comm", &t);
    //start_timer(&t);

    step();
    //print_time_since("Step", &t);
}

void Worker::finalize()
{
    // Finish all border send requests
    for (int round = 0; round < world->total_rounds; round++)
    {
        MPI_Waitall(Border_direction_count, border_send_requests[round], MPI_STATUSES_IGNORE);
    }
}

void Worker::store(int round)
{
    unsigned char *buffer_compressed = block_send_buffers[round];
    block->compress(buffer_compressed);
    int buffer_size = block->width_byte * block->height;
    int writer_num = world->get_writer_num(round);
    MPI_Isend(buffer_compressed, buffer_size, MPI_UNSIGNED_CHAR, writer_num, round, MPI_COMM_WORLD, &block_send_requests[round]);
}

// Communicate borders from and to this actors block
void Worker::communicate(int round)
{
    for (int i = Border_direction_first; i <= Border_direction_last; i++)
    {
        Border_direction dir = (Border_direction)i;

        unsigned char *border_send_buffer = border_send_buffers[round][dir];
        MPI_Request *send_request = &border_send_requests[round][dir];
        block->wrap_border(dir, border_send_buffer);
        border_send(dir, border_send_buffer, send_request);
    }

    for (int i = Border_direction_first; i <= Border_direction_last; i++)
    {
        Border_direction dir = (Border_direction)i;

        int border_recv_buffer_size = block->get_border_size(dir);
        unsigned char border_recv_buffer[border_recv_buffer_size];

        border_recv(dir, border_recv_buffer);
        block->unwrap_border(dir, border_recv_buffer);
    }
}

void Worker::step()
{
    block->step();
}

/* Border communication */

/**
 * Send a border, unblocking
 */
void Worker::border_send(Border_direction target_dir, unsigned char *buffer, MPI_Request *send_request)
{
    int element_count = block->get_border_size(target_dir);
    int target_block = block->get_neighbor_block_num(target_dir);
    int tag = target_dir;
    MPI_Isend(buffer, element_count, MPI_UNSIGNED_CHAR, target_block, tag, worker_comm, send_request);
}

/**
 * Receive a border, blocking
 */
void Worker::border_recv(Border_direction source_dir, unsigned char *buffer)
{
    int element_count = block->get_border_size(source_dir);
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
    block->glider(x, y);
}
