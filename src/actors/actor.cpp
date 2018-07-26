#include "actor.h"

Actor::Actor()
{
    World *world = World::create(GRIDSIZE_X, GRIDSIZE_Y, proc_amt, proc_num, WORKER_SHARE);

    //world->print();

    world->fill(0);
    world->glider(750, 400);
}

Actor *Actor::create(int gridsize_x, int gridsize_y, int proc_amt, int proc_num, double worker_share)
{
    World *world = new World(...);
    if (world->is_worker())
    {
        return new Worker();
    }
    else
    {
        return new Writer();
    }
}