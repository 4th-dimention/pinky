/*
Allen Webster
30.03.2018
*/

// TOP

#if !defined(PINKY_ARENA_H)
#define PINKY_ARENA_H

struct Arena{
    u8 *base;
    umem pos;
    umem max;
};

struct Temp_Memory{
    Arena *arena;
    umem pos;
};

#endif

// BOTTOM