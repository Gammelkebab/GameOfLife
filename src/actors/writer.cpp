#include "writer.h"

#include <string.h>

Writer::Writer(World *world, int proc_num) : Actor(world, proc_num)
{
    set_writer_comm();
}

void Writer::set_writer_comm()
{
    int active_comm_list[world->writer_amt];
    for (int i = 0; i < world->writer_amt; i++)
    {
        active_comm_list[i] = world->worker_amt + i;
    }

    MPI_Group world_group, active_group;
    MPI_Comm_group(MPI_COMM_WORLD, &world_group);
    MPI_Group_incl(world_group, world->worker_amt, active_comm_list, &active_group);
    MPI_Comm_create(MPI_COMM_WORLD, active_group, &writer_comm);
}

void Writer::tick(int round)
{
    int writer_num = round % world->writer_amt;
    if (writer_num == proc_num)
    {
        char write_buffer[world->rows * world->cols];
        load_for_write(write_buffer);
        write_to_file(write_buffer, round);
    }
}

void Writer::load_for_write(char *write_buffer)
{
    for (int y = 0; y < world->rows; y++)
    {
        for (int x = 0; x < world->cols; x++)
        {
            // Receive all data for block(x, y)
            Block *block = world->blocks[y][x];
            for (int block_row = 0; block_row < block->rows; block_row++)
            {
                // Receive one row of block(x, y)
                int first_byte_address = block->starting_y_byte * world->bytes_per_row + block->starting_x;
                char *row_buffer = &write_buffer[first_byte_address];
                MPI_Recv(row_buffer, block->width_byte, MPI_CHAR, block->block_num, block_row, MPI_COMM_WORLD, MPI_STATUS_IGNORE); // TODO TODO TODO !! Protocol !!
            }
        }
    }
}

void Writer::write_to_file(char *write_buffer, int round)
{
    debug("Trying to write to file\n");
    //pbm in mode P4 reads bitwise, 1 black, 0 white
    char *output_filename = new char[100];
    sprintf(output_filename, "./images/frame_%03d.pbm", round);

    MPI_File output_file;
    MPI_File_open(MPI_COMM_SELF, output_filename, MPI_MODE_WRONLY | MPI_MODE_CREATE, MPI_INFO_NULL, &output_file);

    //Create first lines of .pbm layout:
    //mode: P4 (bitwise black white)
    //number of pixels in a row
    //number of rows
    char header[100];
    sprintf(header, "P4\n%d %d\n\0", world->width, world->height);
    int header_length = strlen(header);
    MPI_File_write_at(output_file, 0, header, header_length, MPI_CHAR, MPI_STATUS_IGNORE);

    //determine number of bytes in a row
    int gridsize_x_byte = world->width / 8;
    //if not all bits of a full byte are used (e.g. 10x16 grid)
    //the last bits are ignored
    if (width % 8 != 0)
    {
        gridsize_x_byte++;
    }
    //array holding all bit-information packed in bytes
    debug("out gets size %d x %d\n", gridsize_x_byte, world->height);
    char out[gridsize_x_byte * world->height];
    memset(out, 0, gridsize_x_byte * world->height);

    // TODO
    //MPI_File_open()
    //MPI_File_iwrite()
    fwrite(&out[0], sizeof(char), gridsize_x_byte * world->height, outfile);
    fclose(outfile);
}
