/*
Allen Webster
30.03.2018
*/

// TOP

#if !defined(PINKY_H)
#define PINKY_H

#define KeywordList(M) \
M(proc, "proc") \
M(struct, "struct") \
M(const, "const") \
M(enum, "enum") \
M(persist, "persist") \
M(var, "var") \
M(goto, "goto") \
M(label, "label") \
M(block, "block") \
M(if, "if") \
M(else, "else") \
M(for, "for") \
M(return, "return") \
M(cast, "cast") \
M(typesize, "typesize") \
M(exprsize, "exprsize") \
M(exprtype, "exprtype") \
M(_aligned, "#aligned") \
M(_free, "#free") \
M(_tight, "#tight") \
M(_overlap, "#overlap") \
M(_offset, "#offset") \
M(_size, "#size") \
M(_align, "#align")

#define BuiltInTypeList(M,ms) \
M(void,0) \
M(i8,  1) \
M(i16, 2) \
M(i32, 4) \
M(i64, 8) \
M(imem,ms) \
M(u8,  1) \
M(u16, 2) \
M(u32, 4) \
M(u64, 8) \
M(umem,ms) \
M(b8,  1) \
M(b32, 4) \
M(f32, 4) \
M(f64, 8)

enum{
#define EnumDef(N,S) Keyword_##N,
    KeywordList(EnumDef)
#undef EnumDef
        Keyword_COUNT
};

enum{
#define EnumDef(N,s) BuiltInType_##N,
    BuiltInTypeList(EnumDef,0)
#undef EnumDef
        BuiltInType_COUNT
};

////////////////////////////////

typedef i32 Token_Type;
enum{
    TokenType_EOF = 0,
    TokenType_RESERVATION = 127,
    TokenType_Name,
    TokenType_Number,
    TokenType_Character,
    TokenType_String,
    
    TokenType_Or,
    TokenType_And,
    
    TokenType_LShift,
    TokenType_RShift,
    
    TokenType_EqEq,
    TokenType_NotEq,
    TokenType_LessEq,
    TokenType_GrtrEq,
    
    TokenType_TopKeyword,
    TokenType_VarKeyword,
    TokenType_ControlKeyword,
    TokenType_CastKeyword,
    TokenType_ExprKeyword,
    TokenType_PackKeyword,
    TokenType_LayoutKeyword,
    
    TokenType_NEXT_AVAILABLE,
};

struct Token{
    Token_Type type;
    char *text_pos;
    i32 text_length;
    union{
        char *interned;
        u64 value;
        String string;
        char character;
    };
};

struct Token_Array{
    Token *tokens;
    i32 count;
};

#define StringDef(N,S) global char * keyword_##N = 0;
KeywordList(StringDef)
#undef StringDef

#define StringDef(N,s) global char * built_in_type_name_##N = 0;
BuiltInTypeList(StringDef,0)
#undef StringDef

////////////////////////////////

typedef i32 AST_Kind;
enum{
    Top_Procedure,
    Top_Struct,
    Top_Enum,
    Top_Constant,
    Top_Persist,
    
    ProcedureParameter,
    
    Statement_For,
    Statement_If,
    Statement_Block,
    Statement_Goto,
    Statement_Label,
    Statement_Return,
    Statement_Decl,
    Statement_Assignment,
    Statement_Expression,
    
    StructMember_Variable,
    StructMember_Constant,
    StructMember_LayoutOffset,
    StructMember_LayoutSize,
    StructMember_LayoutAlign,
    StructMember_SubBody,
    
    EnumMember,
    
    Type_Primary,
    Type_Pointer,
    Type_Array,
    Type_Procedure,
    Type_ExprType,
    
    Expr_Binary,
    Expr_Unary,
    Expr_Cast,
    Expr_Call,
    Expr_Scope,
    Expr_Name,
    Expr_Number,
    Expr_String,
    Expr_Char,
    Expr_TypeSize,
    Expr_ExprSize,
    
    BuiltInType,
};

typedef i32 Pack_Style;
enum{
    Pack_Aligned,
    Pack_Free,
    Pack_Tight,
    Pack_Overlap,
};

struct Expr{
    AST_Kind kind;
    char *text_pos;
    union{
        struct{
            Token_Type op;
            Expr *l;
            Expr *r;
        } binary;
        struct{
            Token_Type op;
            Expr *u;
        } unary;
        struct{
            struct Type *t;
            Expr *u;
        } cast;
        struct{
            Expr *p;
            Node args;
        } call;
        struct{
            Expr *l;
            char *r;
        } scope;
        char *name;
        u64 number;
        String string;
        char character;
        struct{
            struct Type *t;
        } typesize;
        struct{
            Expr *e;
        } exprsize;
    };
};

struct Expr_Node{
    Node node;
    Expr *expr;
};

struct Procedure_Parameter{
    AST_Kind kind;
    Node node;
    char *text_pos;
    char *name;
    struct Type *type;
};
struct Procedure_Signature{
    Node params;
    struct Type *ret;
};

struct Type{
    AST_Kind kind;
    char *text_pos;
    union{
        char *name;
        struct{
            Type *sub;
        } pointer;
        struct{
            Expr *size;
            Type *sub;
        } array;
        Procedure_Signature proc;
        Expr *expr;
    };
};

struct Struct_Body{
    Pack_Style pack;
    Node members;
};

struct Struct_Member{
    AST_Kind kind;
    Node node;
    char *text_pos;
    union{
        struct{
            char *name;
            struct Type *type;
        } variable;
        struct Top_Statement *decl;
        Expr *layout;
        Struct_Body body;
    };
};

struct Enum_Member{
    AST_Kind kind;
    Node node;
    char *text_pos;
    char *name;
    Expr *expr;
};

struct Top_Statement{
    AST_Kind kind;
    Node node;
    char *text_pos;
    union{
        struct{
            char *name;
            Procedure_Signature signature;
            Top_Statement *block;
            struct Name_Space *space;
        } proc;
        struct{
            char *name;
            Struct_Body body;
            struct Name_Space *space;
        } struct_node;
        struct{
            char *name;
            Type *type;
            Node members;
        } enum_node;
        struct{
            char *name;
            struct Type *type;
            struct Expr *expr;
        } decl;
        struct{
            Top_Statement *init;
            Expr *check;
            Top_Statement *inc;
            Top_Statement *body;
            struct Name_Space *space;
        } for_node;
        struct{
            Expr *check;
            Top_Statement *body;
            Top_Statement *else_body;
            struct Name_Space *space;
        } if_node;
        struct{
            char *name;
            Node stmnts;
            struct Name_Space *space;
        } block;
        Expr *goto_expr;
        char *label;
        Expr *return_expr;
        struct{
            Expr *l;
            Expr *r;
        } assignment;
        Expr *expr;
    };
};

struct Program{
    Node tops;
    struct Name_Space *space;
};

////////////////////////////////

struct Name_Entry{
    Node node;
    char *name;
    void *ptr;
    struct Name_Space *space;
};

struct Name_Space{
    Name_Space *parent;
    Node sent;
};

////////////////////////////////

struct Built_In_Type{
    AST_Kind kind;
    i32 size;
};

////////////////////////////////

struct Parse_Context{
    Arena *arena;
    Token_Array tokens;
    i32 index;
    Token current;
};


#endif

// BOTTOM

