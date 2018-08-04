#include "actor.h"

#include "worker.h"
#include "idler.h"

Actor *Actor::create(int gridsize_x, int gridsize_y, int proc_amt, int proc_num, int total_rounds)
{
    World *world = new World(gridsize_x, gridsize_y, proc_amt, proc_num, total_rounds);
    if (world->is_worker(proc_num))
    {
        return new Worker(world, proc_num);
    }
    else
    {
        return new Idler(world, proc_num);
    }
}
