#ifndef grid_h
#define grid_h

typedef unsigned char **Grid;

//allocate a 2D array
inline unsigned char **create_grid(int width, int height)
{
    unsigned char **grid = new unsigned char *[height];
    for (int i = 0; i < height; ++i)
    {
        grid[i] = new unsigned char[width];
    }
    return grid;
}

#endif