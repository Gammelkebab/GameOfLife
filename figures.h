#ifndef figures_h
#define figures_h

/**
the figures are ignoring the size of the grid, offset has to be checked manually
**/

//figure with a 2-step period, stationary
void blinker(unsigned char **grid, int offset_X = 0, int offset_Y = 0)
{
    grid[3 + offset_Y][4 + offset_X] = 1;
    grid[4 + offset_Y][4 + offset_X] = 1;
    grid[5 + offset_Y][4 + offset_X] = 1;
}

//figure with a 8-step period, moves
void glider(unsigned char **grid, int offset_X = 0, int offset_Y = 0)
{
    grid[4 + offset_Y][4 + offset_X] = 1;
    grid[4 + offset_Y][5 + offset_X] = 1;
    grid[4 + offset_Y][6 + offset_X] = 1;
    grid[3 + offset_Y][6 + offset_X] = 1;
    grid[2 + offset_Y][5 + offset_X] = 1;
}

//figure
void pentadecathlon(unsigned char **grid, int offset_X = 0, int offset_Y = 0)
{
    for (int x = 0; x < 8; ++x)
    {
        for (int y = 0; y < 3; ++y)
        {
            grid[y + offset_Y][x + offset_X] = 1;
        }
    }
    grid[1 + offset_Y][1 + offset_X] = 0;
    grid[1 + offset_Y][6 + offset_X] = 0;
}

#endif