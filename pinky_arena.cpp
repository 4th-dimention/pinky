/*
Allen Webster
30.03.2018
*/

// TOP

internal Arena
make_arena(void *base, umem size){
    Arena arena = {0};
    arena.base = (u8*)base;
    arena.pos = 0;
    arena.max = size;
    return(arena);
}

internal void*
arena_push(Arena *arena, umem size){
    void *result = 0;
    if (arena->pos + size <= arena->max){
        result = arena->base + arena->pos;
        arena->pos += size;
    }
    return(result);
}

internal void*
arena_peak(Arena *arena, umem size){
    void *result = 0;
    if (arena->pos + size <= arena->max){
        result = arena->base + arena->pos;
    }
    return(result);
}

internal void
arena_align(Arena *arena, umem b){
    umem p = arena->pos + b - 1;
    arena->pos = p - p%b;
}

#define push_array(a,T,c) (T*)(arena_push((a), sizeof(T)*(c)))
#define peek_array(a,T,c) (T*)(arena_peek((a), sizeof(T)*(c)))
#define push_align(a,b) arena_align((a), (b))

internal Temp_Memory
begin_temp(Arena *arena){
    Temp_Memory temp;
    temp.arena = arena;
    temp.pos = arena->pos;
    return(temp);
}

internal void
end_temp(Temp_Memory temp){
    temp.arena->pos = temp.pos;
}

// BOTTOM

