#include "block.h"

#include "string.h"

int Block::compress(unsigned char **buffer_place)
{
    memset(target, 0, width_byte * height_byte);
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            //information about 8 cells are packed in one byte
            for (int k = 0; k < 8; ++k)
            {
                if ((x + starting_x) * 8 + k >= width)
                {
                    break;
                }
                if (grid[y + starting_y][(x + starting_x) * 8 + k] == 1)
                {
                    target[((y + starting_y) / 8) * max_width_byte + (x + starting_x) / 8] += pow(2, 7 - k);
                }
            }
        }
    }
}