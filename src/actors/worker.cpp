#include "worker.h"

#include <string.h>

#include "../debug/debug.h"

#include "../helpers/timing.h"
#include "../world/border_direction.h"

Worker::Worker(World *world, int proc_num) : Actor(world, proc_num)
{
    block = new Block(world, proc_num);
    debug2("Block %d created.\n", proc_num);
#if defined(DEBUG) && DEBUG >= 2
    block->print();
#endif

    /* Headers */
    header_write_buffers = new char *[world->total_rounds];
    for (int r = 0; r < world->total_rounds; r++)
    {
        header_write_buffers[r] = new char[100];
    }
    header_write_requests = new MPI_Request[world->total_rounds];

    /* Blocks */
    block_write_buffers = new unsigned char **[world->total_rounds];
    for (int r = 0; r < world->total_rounds; r++)
    {
        block_write_buffers[r] = new unsigned char *[block->height];
        for (int row = 0; row < block->height; row++)
        {
            int buffer_size = block->width_byte;
            block_write_buffers[r][row] = new unsigned char[buffer_size];
        }
    }
    block_write_requests = new MPI_Request *[world->total_rounds];
    for (int r = 0; r < world->total_rounds; r++)
    {
        block_write_requests[r] = new MPI_Request[block->height];
    }

    /* Borders */
    border_send_buffers = new unsigned char **[world->total_rounds];
    for (int r = 0; r < world->total_rounds; r++)
    {
        border_send_buffers[r] = new unsigned char *[Border_direction_count];
        for (int i = Border_direction_first; i <= Border_direction_last; i++)
        {
            Border_direction dir = (Border_direction)i;
            int buffer_size = block->get_border_size_byte(dir);
            border_send_buffers[r][dir] = new unsigned char[buffer_size];
        }
    }
    border_send_requests = new MPI_Request *[world->total_rounds];
    for (int r = 0; r < world->total_rounds; r++)
    {
        border_send_requests[r] = new MPI_Request[Border_direction_count];
    }
    file_handles = new MPI_File[world->total_rounds];

    debug3("Worker %d done with buffer creation.\n", proc_num);

    set_worker_comm();

    debug3("Worker %d done with comm setup.\n", proc_num);
    debug3("Filling Worker %d.\n", proc_num);
    //fill(0);
    fill(1);
    debug3("Creating Glider at Worker %d.\n", proc_num);
    //glider(0, 0);
    debug3("Worker %d done.\n", proc_num);
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

void Worker::tick(int round)
{
    debug2("Worker %d at store.\n", proc_num);
    store(round);
    debug2("Worker %d at comm.\n", proc_num);
    communicate(round);
    debug2("Worker %d at step.\n", proc_num);
    step();
    debug2("Worker %d done.\n", proc_num);
}

void Worker::finalize()
{
    // Finish all header write requests
    for (int round = 0; round < world->total_rounds; round++)
    {
        MPI_Wait(&header_write_requests[round], MPI_STATUS_IGNORE);
    }
    // Finish all block write requests
    for (int round = 0; round < world->total_rounds; round++)
    {
        for (int row = 0; row < block->height; row++)
        {
            MPI_Wait(&block_write_requests[round][row], MPI_STATUS_IGNORE);
        }
    }
    // Finish all border send requests
    for (int round = 0; round < world->total_rounds; round++)
    {
        for (int i = Border_direction_first; i <= Border_direction_last; i++)
        {
            MPI_Wait(&border_send_requests[round][i], MPI_STATUS_IGNORE);
        }
    }
}

void Worker::store(int round)
{
    unsigned char **buffer_compressed = block_write_buffers[round];
    block->compress(buffer_compressed);
    debug3("Worker %d round %d done compressing.\n", proc_num, round);

    MPI_File *fh = open_file(round);
    debug3("Worker %d round %d file opened.\n", proc_num, round);
    MPI_Request *header_write_request = &header_write_requests[round];
    int header_size = iwrite_header(fh, round, header_write_request);
    debug3("Worker %d round %d done writing header.\n", proc_num, round);

    MPI_Request *block_write_requests = this->block_write_requests[round];
    iwrite_block(fh, buffer_compressed, block_write_requests, header_size);
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

        int border_recv_buffer_size = block->get_border_size_byte(dir);
        unsigned char border_recv_buffer[border_recv_buffer_size];

        border_recv(dir, border_recv_buffer);
        block->unwrap_border(dir, border_recv_buffer);
    }
}

void Worker::step()
{
    block->step();
}

/**
 * @brief Open the file for this round
 * 
 * @param round 
 * @return MPI_File* 
 */
MPI_File *Worker::open_file(int round)
{
    char filename[100];
    sprintf(filename, "./images/frame_%03d.pbm", round);
    MPI_File *file_handle = &file_handles[round];
    MPI_File_open(worker_comm, filename, MPI_MODE_WRONLY | MPI_MODE_CREATE, MPI_INFO_NULL, file_handle);
    return file_handle;
}

/* Block write to file */

/**
 * @brief Write the header for this round, unblocking
 * Only writes the header if this is the first block
 * Returns the header size, which is later needed for offset
 * 
 * @param fh 
 * @param round 
 * @param write_request 
 * @return int 
 */
int Worker::iwrite_header(MPI_File *fh, int round, MPI_Request *write_request)
{
    debug4("Worker %d round %d writing header.\n", proc_num, round);

    // Create the header lines
    char *header = header_write_buffers[round];
    sprintf(header, "P4\n%d %d\n", world->width, world->height);
    int header_size = strlen(header);

    // Let thread 0 write the header to file
    if (block->x == 0 && block->y == 0)
    {
        debug3("Header for round %d: %s\n", round, header);
        MPI_File_iwrite_at(*fh, 0, header, header_size, MPI_CHAR, write_request);
    }
    else
    {
        *write_request = MPI_REQUEST_NULL;
    }

    return header_size;
}

/**
 * @brief Write the block for this round, unblocking
 * 
 * @param fh 
 * @param buffer_compressed 
 * @param write_requests 
 */
void Worker::iwrite_block(MPI_File *fh, unsigned char **buffer_compressed, MPI_Request *write_requests, int header_size)
{
    int row_width_byte = world->width_byte;
    int buffer_rows = block->height;
    for (int row = 0; row < buffer_rows; row++)
    {
        /* The offset gives the starting position inside the file, containing:
         * The size of the header
         * The offset added by the current row
         * The offset inside the current row
         */
        int offset = header_size + (block->starting_y + row) * row_width_byte + block->starting_x / 8;
        debug4("Worker %d row %d has offset %d.\n", proc_num, row, offset);
        unsigned char *row_buffer = buffer_compressed[row];
        MPI_File_iwrite_at(*fh, offset, row_buffer, row_width_byte, MPI_UNSIGNED_CHAR, &write_requests[row]);
    }
}

/* Border communication */

/**
 * Send a border, unblocking
 */
void Worker::border_send(Border_direction target_dir, unsigned char *buffer, MPI_Request *send_request)
{
    int element_count = block->get_border_size_byte(target_dir);
    int target_block = block->get_neighbor_block_num(target_dir);
    int tag = target_dir;
    MPI_Isend(buffer, element_count, MPI_UNSIGNED_CHAR, target_block, tag, worker_comm, send_request);
}

/**
 * Receive a border, blocking
 */
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
    block->glider(x, y);
}
