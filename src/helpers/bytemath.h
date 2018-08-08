#ifndef bytemath_h
#define bytemath_h

/**
 * Convert a bit amount n into the smallest fitting amount of bytes
 * e.g. 0 => 0, 1 => 1, 5 => 1; 15 => 2
 */
inline int to_bytes(int n)
{
    return n / 8  + (n % 8 == 0 ? 0 : 1);  
}

#endif