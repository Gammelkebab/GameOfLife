#include "writer.h"

#include <string.h>

Writer::Writer(World *world, int proc_num) : Actor(world, proc_num)
{
    world->set_blocks();

    /* File write buffers */

    file_write_buffers = new char *[world->total_rounds];
    /* File write buffers are created at runtime,
     * since the header size can not yet be determined.
     */
    file_write_requests = new MPI_Request[world->total_rounds];
}

void Writer::tick(int round)
{
    if (world->get_writer_num(round) == proc_num)
    {
        recv_and_write(round);
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
    // Copy the header lines into the file write buffer
    char *file_write_buffer = file_write_buffers[round];
    char *blocks_write_buffer = file_write_buffer + header_size; // Block buffer starts after header
    strcpy(file_write_buffer, header_buffer);
    // Receive all the block, store them in the file write buffer
    for (int row = 0; row < world->rows; row++)
    {
        for (int col = 0; col < world->cols; col++)
        {
            // Select the current block
            Block *block = world->blocks[row][col];
            int block_offset = block->starting_y * world->width_byte + block->starting_x;
            // Select the current part of the write buffer
            char *block_write_buffer = blocks_write_buffer + block_offset;
            int block_size = block->width * block->height;
            // Receive the blocks information
            MPI_Recv(block_write_buffer, block_size, MPI_UNSIGNED_CHAR, block->block_num, round, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
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
