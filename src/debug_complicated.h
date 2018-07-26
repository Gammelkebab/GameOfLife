#include <stdio.h>
#include <mpi.h>

#define DEBUG 3

inline void debug(const char *fmt, va_list args...)
{
#if defined(DEBUG) && DEBUG > 0
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    char full[200] = "DBG(%d): %s:%d:%s(): \t";
    strcat(full, fmt);
    fprintf(stderr, full, rank, __FILE__, __LINE__, __func__, &args);
#else /* Don't do anything in release builds */
#endif
}