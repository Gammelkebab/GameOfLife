#include "actor.h"

#include "worker.h"
#include "writer.h"

Actor::Actor(World *world, int proc_num) : world(world), proc_num(proc_num)
{
}

Actor *Actor::create(int gridsize_x, int gridsize_y, int proc_amt, int proc_num, double worker_share, int total_iterations)
{
    World *world = new World(gridsize_x, gridsize_y, proc_amt, proc_num, worker_share, total_iterations);
    if (world->is_worker(proc_num))
    {
        return new Worker(world, proc_num);
    }
    else
    {
        return new Writer(world, proc_num);
    }
}
