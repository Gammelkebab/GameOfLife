#ifndef world_h
#define world_h

/**
 * Information on the Overall world
 */
class World
{
  public:
    int width, height;
    int rows, cols;

  public:
    World(int width, int height, int block_amt);
    void print();
};

#endif