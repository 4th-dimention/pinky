/*
Allen Webster
30.03.2018
*/

// TOP

#if !defined(PINKY_STRING_H)
#define PINKY_STRING_H

struct String{
    char *str;
    i32 size;
    i32 cap;
};

struct String_Intern_Table{
    Arena *arena;
    String *strings;
    i32 count;
    i32 max;
};

#endif

// BOTTOM

