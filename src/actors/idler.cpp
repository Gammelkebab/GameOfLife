#include "idler.h"

Idler::Idler(World *world, int proc_num) : Actor(world, proc_num)
{
}

void Idler::tick(int round)
{
    (void)round;
}

void Idler::finalize()
{
}
