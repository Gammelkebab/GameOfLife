#include "writer.h"

#include <string.h>

#include "../debug/debug.h"

Writer::Writer(World *world, int proc_num) : Actor(world, proc_num)
{
    world->set_blocks();

    /* File write buffers */
    file_write_buffers = new char *[world->total_rounds];
    /* File write buffers are created at runtime,
     * since the header and block size can not yet be determined.
     */
    file_write_requests = new MPI_Request[world->total_rounds];

    file_handles = new MPI_File[world->total_rounds];
}

void Writer::tick(int round)
{
    if (world->get_writer_num(round) == proc_num)
    {
        recv_and_write(round);
    }
    else
    {
        file_write_requests[round] = MPI_REQUEST_NULL;
    }
}

void Writer::recv_and_write(int round)
{

    // Create the header lines for this round
    char header_buffer[100];
    sprintf(header_buffer, "P4\n%d %d\n", world->width, world->height);
    int header_size = strlen(header_buffer);
    // Create the file write buffer for this round
    int write_buffer_size = header_size + world->width_byte * world->height;
    file_write_buffers[round] = new char[write_buffer_size];
    char *file_write_buffer = file_write_buffers[round];
    // Copy the header lines into the file write buffer
    strcpy(file_write_buffer, header_buffer);

    // Block buffer starts after header
    char *block_write_buffer = &file_write_buffer[header_size];
    // Receive all the block, store them in the file write buffer
    for (int world_row = 0; world_row < world->rows; world_row++)
    {
        for (int world_col = 0; world_col < world->cols; world_col++)
        {
            // Select the current block
            Block *block = world->blocks[world_row][world_col];
            // Create the receive buffer
            int block_size = block->width_byte * block->height;
            unsigned char recv_buffer[block_size];
            // Receive the blocks information
            MPI_Recv(recv_buffer, block_size, MPI_UNSIGNED_CHAR, block->block_num, round, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            // Select the current part of the write buffer
            // Copy the received data into the write buffer
            for (int block_row = 0; block_row < block->height; block_row++)
            {
                int recv_row_offset = block_row * block->width_byte;
                unsigned char *recv_row = &recv_buffer[recv_row_offset];
                int write_row_offset = (block_row + block->starting_y) * world->width_byte + block->starting_x / 8;
                char *write_row = &block_write_buffer[write_row_offset];
                memcpy(write_row, recv_row, block->width_byte);
            }
        }
    }
    MPI_File *file = open_file(round);
    // Write the file write buffer to the file
    iwrite_block(file, header_size, round);
}

void Writer::finalize()
{
    // Finish all file write requests
    MPI_Waitall(world->total_rounds, file_write_requests, MPI_STATUSES_IGNORE);
}

/**
 * @brief Open the file for this round
 * 
 * @param round 
 * @return MPI_File* 
 */
MPI_File *Writer::open_file(int round)
{
    char filename[100];
    sprintf(filename, "./images/frame_%03d.pbm", round);
    MPI_File *file_handle = &file_handles[round];
    MPI_File_open(MPI_COMM_SELF, filename, MPI_MODE_WRONLY | MPI_MODE_CREATE, MPI_INFO_NULL, file_handle);
    return file_handle;
}

/* Block write to file */

/**
 * @brief Write the block for this round, unblocking
 * 
 * @param fh 
 * @param buffer_compressed 
 * @param write_requests 
 */
void Writer::iwrite_block(MPI_File *fh, int header_size, int round)
{
    int buffer_size = header_size + world->width_byte * world->height;
    MPI_File_iwrite(*fh, file_write_buffers[round], buffer_size, MPI_CHAR, &file_write_requests[round]);
}
