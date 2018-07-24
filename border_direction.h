#ifndef border_direction_h
#define border_direction_h

typedef enum Border_direction
{
    NORTH,
    SOUTH,
    EAST,
    WEST,
    NORTH_WEST,
    NORTH_EAST,
    SOUTH_WEST,
    SOUTH_EAST
} Border_direction;

void print_border_direction(Border_direction dir)
{
    switch (dir)
    {
    case NORTH:
        printf("NORTH");
        break;
    case SOUTH:
        printf("SOUTH");
        break;
    case WEST:
        printf("WEST");
        break;
    case EAST:
        printf("EAST");
        break;
    case NORTH_WEST:
        printf("NORTH_WEST");
        break;
    case NORTH_EAST:
        printf("NORTH_EAST");
        break;
    case SOUTH_WEST:
        printf("SOUTH_WEST");
        break;
    case SOUTH_EAST:
        printf("SOUTH_EAST");
        break;
    default:
        printf("NONE");
    }
}

Border_direction opposite_direction(Border_direction dir)
{
    switch (dir)
    {
    case NORTH:
        return SOUTH;
    case SOUTH:
        return NORTH;
    case WEST:
        return EAST;
    case EAST:
        return WEST;
    case NORTH_WEST:
        return SOUTH_EAST;
    case NORTH_EAST:
        return SOUTH_WEST;
    case SOUTH_WEST:
        return NORTH_EAST;
    case SOUTH_EAST:
        return NORTH_WEST;
    default:
        return NORTH;
    }
}

#endif