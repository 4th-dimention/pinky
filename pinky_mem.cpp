/*
Allen Webster
30.03.2018
*/

// TOP

internal void
block_zero(void *ptr, umem size){
    u8 *p = (u8*)ptr;
    u8 *e = p + size;
    for (;p < e; ++p){
        *p = 0;
    }
}

internal void
block_copy(void *dst, void *src, umem size){
    u8 *d = (u8*)dst;
    u8 *s = (u8*)src;
    u8 *e = s + size;
    for (;s < e; ++s, ++d){
        *d = *s;
    }
}

// BOTTOM