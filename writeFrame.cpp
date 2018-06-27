//creates a .pbm file displaying the current grid
//pbm in mode P4 reads bitwise, 1 black, 0 white
void writeFrame(unsigned char **grid, int i)
{
    char *filename = new char[100];
    sprintf(filename, "./images/frame_%03d.pbm", i);
    FILE *outfile = fopen(filename, "w");
    //Create first lines of .pbm layout:
    //mode: P4 (bitwise black white)
    //number of pixels in a row
    //number of rows
    fprintf(outfile, "P4\n%d %d\n", GRIDSIZE_X, GRIDSIZE_Y);

    //determine number of bytes in a row
    int gridsize_x_byte = GRIDSIZE_X / 8;
    //if not all bits of a full byte are used (e.g. 10x16 grid)
    //the last bits are ignored
    if (GRIDSIZE_X % 8 != 0)
    {
        gridsize_x_byte++;
    }
    //array holding all bit-information packed in bytes
    //char out[gridsize_x_byte*GRIDSIZE_Y] = {0};
    char out[gridsize_x_byte * GRIDSIZE_Y];
    memset(out, 0, gridsize_x_byte * GRIDSIZE_Y * sizeof(char));

    for (int y = 0; y < GRIDSIZE_Y; ++y)
    {
        for (int x = 0; x < gridsize_x_byte; ++x)
        {
            //information about 8 cells are packed in one byte
            for (int k = 0; k < 8; ++k)
            {
                if (x * 8 + k >= GRIDSIZE_X)
                    break;
                if (grid[y][x * 8 + k] == 1)
                {
                    out[y * gridsize_x_byte + x] += pow(2, 7 - k);
                }
            }
        }
    }
    fwrite(&out[0], sizeof(char), gridsize_x_byte * GRIDSIZE_Y, outfile);
    fclose(outfile);
}