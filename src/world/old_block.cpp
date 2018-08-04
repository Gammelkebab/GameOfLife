#include "block.h"

// TODO group nearby blocks on same cluster node

Block::Block(World *world)
{
    x = block_num % world->cols;
    y = block_num / world->cols;

    first_row = y == 0;
    first_col = x == 0;
    last_row = y == world->rows - 1;
    last_col = x == world->cols - 1;

    // calculate the pixels assigned to this block
    // takes care of 8 bit allignment
    // takes care of additional pixels from remainder of splitting
    width = world->width / world->cols;
    width -= width % 8;
    max_width = width;
    int remainder_x = world->width % (world->cols * 8); // the remainder after assigning full 8 bit blocks
                                                        // this value is always < world->cols * 8
                                                        // distribute all full 8 bit blocks of the remainder to the first blocks,
                                                        // then the remaining bits to the last block
    if (x < remainder_x / 8)
    {
        width += 8;
    }
    if (last_col)
    {
        width += remainder_x % 8;
    }

    // Calculate the maximum width of a block in the world
    if (remainder_x > 8)
    {
        max_width += 8;
    }
    else
    {
        max_width += remainder_x;
    }

    // the same for the height
    height = world->height / world->rows;
    height -= height % 8;
    max_height = height;
    int remainder_y = world->height % (world->rows * 8);
    if (y < remainder_y / 8)
    {
        height += 8;
    }
    if (last_row)
    {
        height += remainder_y % 8;
    }

    // Calculate the maximum height of a block in the world
    if (remainder_y > 8)
    {
        max_height += 8;
    }
    else
    {
        max_height += remainder_y;
    }

    // calculate the position of the first (upper left) pixel assigned to this block
    // takes care of additional pixels from remainder of splitting
    starting_x = world->width / world->cols;
    starting_x -= starting_x % 8;
    starting_x *= x;
    starting_x += min(remainder_x / 8, x) * 8; // add all horizontal remainder pixels before this block
    starting_y = world->height / world->rows;
    starting_y -= starting_y % 8;
    starting_y *= y;
    starting_y += min(remainder_y / 8, y) * 8; // add all vertical remainder pixels before this block

    // Create the write buffer grid, containing compressed information about the block
    // 8 Pixels are compressed into one byte
    width_byte = width / 8 + (remainder_x == 0 ? 0 : 1);
    height_byte = height / 8 + (remainder_y == 0 ? 0 : 1);

    max_width_byte = max_width / 8 + (max_width % 8 == 0 ? 0 : 1);
    max_height_byte = max_height / 8 + (max_height % 8 == 0 ? 0 : 1);
    write_grid = (char *)malloc(max_width_byte * max_height_byte);
}

void Block::write(char *grid_out, int bytes_per_row)
{
    for (int y = 0; y < height_byte; y++)
    {
        for (int x = 0; x < width_byte; x++)
        {
            debug("[%d] - %p is now written with (%d, %d) value at %d\n", block_num, grid_out, y, x, (y + starting_y / 8) * bytes_per_row + (x + starting_x / 8));
            grid_out[(y + starting_y / 8) * bytes_per_row + (x + starting_x / 8)] = write_grid[y * max_width_byte + x];
            // write the compressed and packed data of the write_grid into the compressed but scattered grid_out
            // Pixels are still compressed into bytes, but the data of one block has holes in between,
            // where parts of the other blocks go
            debug("done.\n");
        }
    }
}

void Block::load_for_write(MPI_Request *request)
{
    debug("[%d, %d] - Receiving %d x %d bytes\n", world->proc_num, block_num, max_width_byte, max_height_byte);
    MPI_Irecv(write_grid, max_width_byte * max_height_byte, MPI_UNSIGNED_CHAR, block_num, world->proc_num, world->active_comm, request);
}

void Block::print()
{
    printf("Block:\n{\n");
    printf("\tPosition: \t(%d, %d)\n", x, y);
    printf("\tPixel size: \t%d x %d\n", width, height);
    printf("\tMaximum size: \t(%d x %d)\n", max_width, max_height);
    printf("\tPixel start: \t(%d x %d)\n", starting_x, starting_y);
    printf("\tSpecial Info:\n");
    printf("\t{\n");
    printf("\t\tFirst Row: \t%c\n", first_row ? '+' : '0');
    printf("\t\tLast Row: \t%c\n", last_row ? '+' : '0');
    printf("\t\tFirst Col: \t%c\n", first_col ? '+' : '0');
    printf("\t\tLast Col: \t%c\n", last_col ? '+' : '0');
    printf("\t}\n");
    printf("}\n");
}

void Worker::fill(unsigned char value)
{

    for (int x = 1; x <= width; x++)
    {
        for (int y = 1; y <= height; y++)
        {
            grid[y][x] = value;
        }
    }
}

Active_block::Active_block(World *world, int block_num) : Block(world, block_num)
{

    grid = array2D(width + 2, height + 2);
    next_grid = array2D(width + 2, height + 2);
    send_block_buffers = (char **)malloc((world->block_amt) * sizeof(char *));

    for (int i = 0; i < world->block_amt; i++)
    {
        send_block_buffers[i] = (char *)malloc(max_width_byte * max_height_byte);
    }

    randomize();
}

Active_block::~Active_block()
{
    // TODO
    delete[] grid;
    delete[] next_grid;
}

//rules for Game of Life
//you can change this function, but the rules have to remain the same
int Active_block::isAlive(int neighbours, unsigned char cell)
{
    switch (neighbours)
    {
    case (2):
        return cell;
    case (3):
        return 1;
    default:
        return 0;
    }
}

//count neighbours that are alive in a 2D grid
//there are 9 neighbours for each cell
//the grid has no real border, neighbours above top row is the bottom row
//alive = 1, dead = 0
int Active_block::getNeighbours(unsigned char **grid, int x, int y)
{
    if (x == 0 || x == width + 1 || y == 0 || y == height + 1)
    {
        printf("Trying to get neighbours of border pixel (%d, %d). Returning 2.", x, y);
        return 2;
    }
    int sum = 0;
    // pixels in row above and below
    for (int i = 0; i < 3; ++i)
    {
        sum += grid[y - 1][x - 1 + i];
        sum += grid[y + 1][x - 1 + i];
    }
    // pixels left and right
    sum += grid[y][x - 1];
    sum += grid[y][x + 1];
    return sum;
}

//console output, small grid is preferable
void Active_block::printGrid()
{
    // Print the current blocks adress
    printf("Block [%d, %d]:\n", x, y);

    // Print all the assigned pixels without the borders
    for (int y = 0; y < height + 2; y++)
    {
        if (y == width + 1)
        {
            printf("\n");
        }
        for (int x = 0; x < width + 2; x++)
        {
            if (x == width + 1)
            {
                printf("\t");
            }
            if (grid[y][x])
            {
                printf("X");
            }
            else
            {
                printf("-");
            }
            if (x == 0)
            {
                printf("\t");
            }
        }
        printf("\n");
        if (y == 0)
        {
            printf("\n");
        }
    }
    printf("\n\n");
}

//fill the grid randomly with ~35% alive
void Active_block::randomize()
{

    srand(time(NULL));
    for (int x = 1; x <= width; ++x)
    {
        for (int y = 1; y <= height; ++y)
        {
            grid[y][x] = (int)rand() % 100 < 35 ? 1 : 0;
        }
    }
}

void print_unsigned_char_array(unsigned char *arr, int size)
{
    printf("[");
    for (int i = 0; i < size; i++)
    {
        if (i > 0)
        {
            printf(", ");
        }
        printf("%d", arr[i]);
    }
    printf("]\n");
}

/* Functions calculating the surrounding block numbers */

int Active_block::position_to_block_number(int x, int y)
{
    return (y * world->cols + x);
}

int Active_block::north()
{
    if (first_row)
    {
        // above first row is last row
        return position_to_block_number(x, world->rows - 1);
    }
    else
    {
        return position_to_block_number(x, y - 1);
    }
}

int Active_block::south()
{
    if (last_row)
    {
        // below last row is first row
        return position_to_block_number(x, 0);
    }
    else
    {
        return position_to_block_number(x, y + 1);
    }
}

int Active_block::west()
{
    if (first_col)
    {
        // left from first col is last col
        return position_to_block_number(world->cols - 1, y);
    }
    else
    {
        return position_to_block_number(x - 1, y);
    }
}

int Active_block::east()
{
    if (last_col)
    {
        // right from last col is first col
        return position_to_block_number(0, y);
    }
    else
    {
        return position_to_block_number(x + 1, y);
    }
}

int Active_block::north_west()
{
    int other_x, other_y;
    if (first_col)
    {
        other_x = world->cols - 1;
    }
    else
    {
        other_x = x - 1;
    }
    if (first_row)
    {
        other_y = world->rows - 1;
    }
    else
    {
        other_y = y - 1;
    }
    return position_to_block_number(other_x, other_y);
}

int Active_block::north_east()
{
    int other_x, other_y;
    if (last_col)
    {
        other_x = 0;
    }
    else
    {
        other_x = x + 1;
    }
    if (first_row)
    {
        other_y = world->rows - 1;
    }
    else
    {
        other_y = y - 1;
    }
    return position_to_block_number(other_x, other_y);
}

int Active_block::south_west()
{
    int other_x, other_y;
    if (first_col)
    {
        other_x = world->cols - 1;
    }
    else
    {
        other_x = x - 1;
    }
    if (last_row)
    {
        other_y = 0;
    }
    else
    {
        other_y = y + 1;
    }
    return position_to_block_number(other_x, other_y);
}

int Active_block::south_east()
{
    int other_x, other_y;
    if (last_col)
    {
        other_x = 0;
    }
    else
    {
        other_x = x + 1;
    }
    if (last_row)
    {
        other_y = 0;
    }
    else
    {
        other_y = y + 1;
    }
    return position_to_block_number(other_x, other_y);
}

int Active_block::neighbour_number(Border_direction dir)
{
    switch (dir)
    {
    case NORTH:
        return north();
    case SOUTH:
        return south();
    case WEST:
        return west();
    case EAST:
        return east();
    case NORTH_WEST:
        return north_west();
    case NORTH_EAST:
        return north_east();
    case SOUTH_WEST:
        return south_west();
    case SOUTH_EAST:
        return south_east();
    default:
        return -1;
    }
}

/* Wrap functions */

void Active_block::wrap_row(unsigned char *buffer, int row)
{
    for (int x = 1; x <= width; x++)
    {
        buffer[x - 1] = grid[row][x];
    }
}

void Active_block::wrap_col(unsigned char *buffer, int col)
{
    for (int y = 1; y <= height; y++)
    {
        buffer[y - 1] = grid[y][col];
    }
}

void Active_block::wrap_corner(unsigned char *buffer, int x, int y)
{
    buffer[0] = grid[y][x];
}

void Active_block::wrap(unsigned char *buffer, Border_direction dir)
{
    switch (dir)
    {
    case NORTH:
        wrap_row(buffer, 1);
        break;
    case SOUTH:
        wrap_row(buffer, height);
        break;
    case WEST:
        wrap_col(buffer, 1);
        break;
    case EAST:
        wrap_col(buffer, width);
        break;
    case NORTH_WEST:
        wrap_corner(buffer, 1, 1);
        break;
    case NORTH_EAST:
        wrap_corner(buffer, width, 1);
        break;
    case SOUTH_WEST:
        wrap_corner(buffer, 1, height);
        break;
    case SOUTH_EAST:
        wrap_corner(buffer, width, height);
        break;
    }
}

/* Unwrap functions */

void Active_block::unwrap_row(unsigned char *buffer, int row)
{
    for (int x = 1; x <= width; x++)
    {
        grid[row][x] = buffer[x - 1];
    }
}

void Active_block::unwrap_col(unsigned char *buffer, int col)
{
    for (int y = 1; y <= height; y++)
    {
        grid[y][col] = buffer[y - 1];
    }
}

void Active_block::unwrap_corner(unsigned char *buffer, int x, int y)
{
    grid[y][x] = buffer[0];
}

void Active_block::unwrap(unsigned char *buffer, Border_direction dir)
{
    switch (dir)
    {
    case NORTH:
        unwrap_row(buffer, 0);
        break;
    case SOUTH:
        unwrap_row(buffer, height + 1);
        break;
    case WEST:
        unwrap_col(buffer, 0);
        break;
    case EAST:
        unwrap_col(buffer, width + 1);
        break;
    case NORTH_WEST:
        unwrap_corner(buffer, 0, 0);
        break;
    case NORTH_EAST:
        unwrap_corner(buffer, width + 1, 0);
        break;
    case SOUTH_WEST:
        unwrap_corner(buffer, 0, height + 1);
        break;
    case SOUTH_EAST:
        unwrap_corner(buffer, width + 1, height + 1);
        break;
    }
}
/* element count */

int Active_block::count_by_direction(Border_direction dir)
{
    if (dir == NORTH || dir == SOUTH)
    {
        return width;
    }
    else if (dir == EAST || dir == WEST)
    {
        return height;
    }
    else
    {
        return 1;
    }
}

void Active_block::load_for_write()
{
    debug("Active_block::load_for_write\n");
    store_grid_compressed(write_grid);
}

void Active_block::send_for_write(int target_num, MPI_Request *request)
{
    debug("send_for_write\n");
    store_grid_compressed(send_block_buffers[target_num]);
    debug("Sending %d x %d bytes\n", max_width_byte, max_height_byte);
    MPI_Isend(send_block_buffers[target_num], max_width_byte * max_height_byte, MPI_UNSIGNED_CHAR, target_num, target_num, world->active_comm, request);
}
