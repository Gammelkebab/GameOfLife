#ifndef min_max_h
#define min_max_h

inline int min(int a, int b)
{
    return a <= b ? a : b;
}

inline int max(int a, int b)
{
    return a >= b ? a : b;
}

#endif