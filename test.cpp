#include "./block.h"

int main()
{
    int block_amt = 14;
    int gridsize_x = 407;
    int gridsize_y = 298;

    Block *blocks[block_amt];
    for (int block_num = 0; block_num < block_amt; block_num++)
    {
        blocks[block_num] = new Block(block_num, block_amt, gridsize_x, gridsize_y);
        blocks[block_num]->printBlock(block_num == 0);
    }
}