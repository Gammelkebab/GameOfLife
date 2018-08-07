#include "actor.h"

#include "../debug/debug.h"

#include "worker.h"
#include "idler.h"

Actor::Actor(World *world, int proc_num) : world(world), proc_num(proc_num)
{
    set_worker_comm();
}

Actor *Actor::create(int gridsize_x, int gridsize_y, int proc_amt, int proc_num, int total_rounds, double worker_share)
{
    World *world = new World(gridsize_x, gridsize_y, proc_amt, total_rounds, worker_share);

    if (world->is_worker(proc_num))
    {
        return new Worker(world, proc_num);
    }
    else
    {
        return new Idler(world, proc_num);
    }
}

void Actor::set_worker_comm()
{
    MPI_Group world_group, active_group;
    MPI_Comm_group(MPI_COMM_WORLD, &world_group);
    MPI_Group_excl(world_group, world->writer_amt, world->writer_nums, &active_group);
    MPI_Comm_create(MPI_COMM_WORLD, active_group, &worker_comm);
}
