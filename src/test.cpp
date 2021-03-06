#include "./block.h"
#include "./figures.h"

void create_dummy_blocks(Block **blocks, int block_amt, int gridsize_x, int gridsize_y)
{
    for (int block_num = 0; block_num < block_amt; block_num++)
    {
        blocks[block_num] = new Block(block_num, block_amt, gridsize_x, gridsize_y);
    }
}

void wrap_unwrap_test(Block *block)
{
    unsigned char **grid = block->grid;
    for (int i = 1; i <= block->width; i++)
    {
        grid[0][i] = i % 2;
    }
    unsigned char buffer[block->width];

    block->wrap(buffer, NORTH);
    printf("Wrapped:\n[");
    for (int i = 0; i < block->width; i++)
    {
        if (i > 0)
        {
            printf(", ");
        }
        printf("%d", buffer[i]);
    }
    printf("]\n");

    for (int i = 0; i < block->width; i++)
    {
        buffer[i] = i % 2;
    }

    block->unwrap(buffer, NORTH);
    printf("Unwrapped:\n[");
    for (int i = 1; i <= block->width; i++)
    {
        if (i > 1)
        {
            printf(", ");
        }
        printf("%d", grid[0][i]);
    }
    printf("]\n");
}

void neighbour_number_test(Block *block)
{
    printf("NORTH from Block 0: Block %d\n", block->neighbour_number(NORTH));
    printf("SOUTH from Block 0: Block %d\n", block->neighbour_number(SOUTH));
    printf("WEST  from Block 0: Block %d\n", block->neighbour_number(WEST));
    printf("EAST  from Block 0: Block %d\n", block->neighbour_number(EAST));
    printf("NORTH_WEST from Block 0: Block %d\n", block->neighbour_number(NORTH_WEST));
    printf("NORTH_EAST from Block 0: Block %d\n", block->neighbour_number(NORTH_EAST));
    printf("SOUTH_WEST from Block 0: Block %d\n", block->neighbour_number(SOUTH_WEST));
    printf("SOUTH_EAST from Block 0: Block %d\n", block->neighbour_number(SOUTH_EAST));
}

void grid_fill_test(Block *block)
{
    block->printGrid();
    block->fill(0);
    block->printGrid();
    block->fill(1);
    block->printGrid();
}

void set_bit_test(Block *block)
{
    printf("Toggling 2nd position\n");

    unsigned char a = 0;
    printf("%d => ", a);
    block->set_bit(&a, 2, 1);
    printf("%d\n", a);

    unsigned char b = 255;
    printf("%d => ", b);
    block->set_bit(&b, 2, 0);
    printf("%d\n", b);
}

void step_test(Block *block)
{
    block->fill(0);               // clear grid
    blinker(block->grid, 10, 10); // create blinker at (10, 10)

    // Fill the upper and lower border
    for (int x = 0; x <= block->width + 1; x++)
    {
        block->grid[0][x] = 1;
        block->grid[block->width + 1][x] = 1;
    }

    // Fill the left and right border
    for (int y = 0; y <= block->height + 1; y++)
    {
        block->grid[y][0] = 1;
        block->grid[y][block->width + 1] = 1;
    }

    block->printGrid();
    block->step();
    block->printGrid();
    block->step();
    block->printGrid();
}

int main()
{
    int block_amt = 14;
    int gridsize_x = 407;
    int gridsize_y = 298;

    /*
     * Dummy block layout:
     * 
     *   |     0     |     1     |     2     |     3     |     
     * --+-----------+-----------+-----------+-----------+-----
     * 0 | 0: (0, 0) | 1: (1, 0) | 2: (2, 0) | 3: (3, 0) | 104 
     * --+-----------+-----------+-----------+-----------+-----
     * 1 | 4: (0, 1) | 5: (1, 1) | 6: (2, 1) | 7: (3, 1) | 96  
     * --+-----------+-----------+-----------+-----------+-----
     * 2 | 8: (0, 2) | 9: (1, 2) | 10:(2, 2) | 11:(3, 2) | 98  
     * --+-----------+-----------+-----------+-----------+-----
     *   |    104    |    104    |     96    |    103    |     
     */

    Block *blocks[block_amt];
    create_dummy_blocks(blocks, block_amt, gridsize_x, gridsize_y);

    /*
    for (int block_num = 0; block_num < block_amt; block_num++)
    {
        blocks[block_num]->printBlock(block_num == 0);
    }
    */

    //Block *block = blocks[0];

    //wrap_unwrap_test(block);

    //neighbour_number_test(block);

    //block->printBlock(true);
    //grid_fill_test(block);

    //block = blocks[11];

    //block->printBlock(true);
    //grid_fill_test(block);

    //set_bit_test(block);

    //step_test(block);
}