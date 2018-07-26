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

inline int btw(int val, int val_min, int val_max)
{
    return max(min(val, val_max), val_min);
}

#endif