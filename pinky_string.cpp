/*
Allen Webster
30.03.2018
*/

// TOP

internal b32
char_is_whitepace(char c){
    return((c == ' ' ||
            c == '\r' ||
            c == '\n' ||
            c == '\t' ||
            c == '\f' ||
            c == '\v'));
}

internal b32
char_is_digit(char c){
    return('0' <= c && c <= '9');
}

internal b32
char_is_oct_digit(char c){
    return(('0' <= c && c <= '7'));
}

internal b32
char_is_hex_digit(char c){
    return(('0' <= c && c <= '9') || ('a' <= c && c <= 'f') || ('A' <= c && c <= 'F'));
}

internal i32
digit_to_int(char c){
    return(c - '0');
}

internal i32
oct_digit_to_int(char c){
    return(c - '0');
}

internal i32
hex_digit_to_int(char c){
    if ('0' <= c && c <= '9'){
        return(c - '0');
    }
    else if ('A' <= c && c <= 'F'){
        return(c - 'A' + 0xA);
    }
    else{
        return(c - 'a' + 0xA);
    }
}

internal b32
char_is_alphabetic(char c){
    return(('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z'));
}

internal i32
cstring_length(char *c){
    char *s = c;
    for (;*c;++c);
    i32 l = (i32)(c - s);
    return(l);
}

internal String
string(char *s, i32 size, i32 cap){
    String str;
    str.str = s;
    str.size = size;
    str.cap = cap;
    return(str);
}

internal String
string(char *s, i32 size){
    return(string(s, size, size));
}

internal String
string(char *c){
    return(string(c, cstring_length(c)));
}

internal b32
match(String a, String b){
    b32 result = false;
    if (a.size == b.size){
        result = true;
        for (i32 i = 0; i < a.size; ++i){
            if (a.str[i] != b.str[i]){
                result = false;
                break;
            }
        }
    }
    return(result);
}

////////////////////////////////

internal String_Intern_Table
string_intern_table(Arena *arena, i32 max){
    String_Intern_Table table = {0};
    table.arena = arena;
    table.strings = push_array(arena, String, max);
    table.max = max;
    return(table);
}

internal char*
intern_string(String_Intern_Table *table, String str){
    i32 count = table->count;
    String *s_ptr = table->strings;
    for (i32 i = 0; i < count; ++i, ++s_ptr){
        if (match(*s_ptr, str)){
            return(s_ptr->str);
        }
    }
    
    Assert(table->count < table->max);
    String *new_str = &table->strings[table->count++];
    new_str->str = push_array(table->arena, char, str.size + 1);
    block_copy(new_str->str, str.str, str.size);
    new_str->str[str.size] = 0;
    new_str->size = str.size;
    new_str->cap = str.size + 1;
    
    return(new_str->str);
}

internal b32
intern_cmp(void *a, void *b){
    return(a == b && a != 0);
}

// BOTTOM

