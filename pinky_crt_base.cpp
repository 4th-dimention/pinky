/*
Allen Webster
30.03.2018
*/

// TOP

#include <stdio.h>
#include <stdlib.h>

#include "pinky_defines.h"
#include "pinky_linked_data_macros.h"
#include "pinky_arena.h"
#include "pinky_string.h"
#include "pinky.h"

#include "pinky_mem.cpp"
#include "pinky_arena.cpp"
#include "pinky_string.cpp"
#include "pinky.cpp"

#include "pinky_printer.cpp"

internal String
load_file(Arena *arena, FILE *file){
    String result = {0};
    fseek(file, 0, SEEK_END);
    result.size = ftell(file);
    result.cap = result.size + 1;
    fseek(file, 0, SEEK_SET);
    result.str = push_array(arena, char, result.cap);
    if (result.str != 0){
        fread(result.str, 1, result.size, file);
        result.str[result.size] = 0;
    }
    else{
        block_zero(&result, sizeof(result));
    }
    return(result);
}

internal String
load_file(Arena *arena, char *file_name){
    String result = {0};
    FILE *file = fopen(file_name, "rb");
    if (file != 0){
        result = load_file(arena, file);
        fclose(file);
    }
    return(result);
}

int main(int arg, char **argv){
    umem arena_size = MB(128);
    Arena arena = make_arena(malloc(arena_size), arena_size);
    String text = load_file(&arena, "test.pnk");
    Token_Array tokens = scan(&arena, text);
    String_Intern_Table string_interns = intern_tokens(&arena, tokens);
    //print_token_array(stdout, tokens);
    Program *program = parse(&arena, tokens);
    if (program == 0){
        fprintf(stdout, "reject\n");
        exit(1);
    }
    
    Name_Space *names = fill_names(&arena, program);
    if (names == 0){
        fprintf(stdout, "reject\n");
        exit(1);
    }
    
    fprintf(stdout, "accept\n");
    return(0);
}

// BOTTOM

