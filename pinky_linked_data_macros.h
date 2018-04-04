/*
Allen Webster
30.03.2018
*/

// TOP

#if !defined(PINKY_LINKED_DATA_MACROS_H)
#define PINKY_LINKED_DATA_MACROS_H

#define dll_init_sentinel(s) ((s)->next=(s),(s)->prev=(s))
#define dll_insert(p,n)        ((n)->next=(p)->next,(n)->prev=(p),(p)->next->prev=(n),(p)->next=(n)) 
#define dll_insert_behind(p,n) ((n)->prev=(p)->prev,(n)->next=(p),(p)->prev->next=(n),(p)->prev=(n))


#define dll_insert_list(p,f,l)        ((l)->next=(p)->next,(f)->prev=(p),(p)->next->prev=(l),(p)->next=(f))
#define dll_insert_list_behind(p,f,l) ((f)->prev=(p)->prev,(l)->next=(p),(p)->prev->next=(f),(p)->prev=(l))

#define zdll_push(f,l,n) ( ((f)==0)?((f)=(l)=(n)):((l)->next=(n),(l)=(n)) )

union Node{
    struct{
        Node *next;
        Node *prev;
    };
    struct{
        Node *first;
        Node *last;
    };
};

#endif

// BOTTOM