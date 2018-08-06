#include "actor.h"

#include "../debug/debug.h"

#include "worker.h"
#include "idler.h"

Actor::Actor(World *world, int proc_num) : world(world), proc_num(proc_num)
{
    set_worker_comm();
    debug3("Actor %d done with comm setup.\n", proc_num);
}

Actor *Actor::create(int gridsize_x, int gridsize_y, int proc_amt, int proc_num, int total_rounds)
{
    World *world = new World(gridsize_x, gridsize_y, proc_amt, total_rounds);
    #if defined(DEBUG) && DEBUG >= 2
    world->print();
    #endif
    debug2("World %d created.\n", proc_num);
    if (world->is_worker(proc_num))
    {
        debug2("Actor %d is a worker.\n", proc_num);
        return new Worker(world, proc_num);
    }
    else
    {
        debug2("Actor %d is an idler.\n", proc_num);
        return new Idler(world, proc_num);
    }
}

void Actor::set_worker_comm()
{
    int active_comm_list[world->worker_amt];
    for (int i = 0; i < world->worker_amt; i++)
    {
        active_comm_list[i] = i;
    }

    MPI_Group world_group, active_group;
    MPI_Comm_group(MPI_COMM_WORLD, &world_group);
    MPI_Group_incl(world_group, world->worker_amt, active_comm_list, &active_group);
    MPI_Comm_create(MPI_COMM_WORLD, active_group, &worker_comm);
}
