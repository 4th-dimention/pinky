/*
Allen Webster
30.03.2018
*/

// TOP

internal Token_Array
scan(Arena *arena, String text){
    Token_Array array = {0};
    array.tokens = push_array(arena, Token, 0);
    
    char *s = text.str;
    char *e = s + text.size;
    for (;s < e;){
        if (char_is_whitepace(*s)){
            do{
                ++s;
            } while(s < e && char_is_whitepace(*s));
            continue;
        }
        
        if (*s == '/' && s + 1 < e){
            if (s[1] == '/'){
                s += 2;
                for (;s < e && *s != '\n'; ++s);
                continue;
            }
            else if (s[1] == '*'){
                s += 2;
                for (;s + 1 < e && !(s[0] == '*' && s[1] == '/'); ++s);
                if (s + 1 < e){
                    s += 2;
                }
                else{
                    s = e;
                }
                continue;
            }
        }
        
        Token *token = push_array(arena, Token, 1);
        token->text_pos = s;
        
        if (char_is_digit(*s)){
            token->type = TokenType_Number;
            b32 has_a_period = false;
            do{
                ++s;
                if (s < e && *s == '.' && !has_a_period){
                    has_a_period = true;
                    ++s;
                }
            } while(s < e && (char_is_digit(*s) || *s == '_'));
        }
        else if (char_is_alphabetic(*s) || *s == '_' || *s == '#'){
            token->type = TokenType_Name;
            do{
                ++s;
            } while(s < e && (char_is_alphabetic(*s) || *s == '_' ||char_is_digit(*s)));
        }
        else if (*s == '\'' || *s == '"'){
            char delim = *s++;
            token->type = TokenType_String;
            if (delim == '\''){
                token->type = TokenType_Character;
            }
            for (;s < e && *s != delim && *s != '\n';){
                if (*s == '\\'){
                    ++s;
                    if (s < e && *s == 'x'){
                        s += 2;
                    }
                }
                ++s;
            }
            ++s;
        }
        else{
            token->type = *s++;
            switch (token->type){
                case '<':
                {
                    if (s < e){
                        if (*s == '<'){
                            ++s;
                            token->type = TokenType_LShift;
                        }
                        else if (*s == '='){
                            ++s;
                            token->type = TokenType_LessEq;
                        }
                    }
                }break;
                
                case '>':
                {
                    if (s < e){
                        if (*s == '>'){
                            ++s;
                            token->type = TokenType_RShift;
                        }
                        else if (*s == '='){
                            ++s;
                            token->type = TokenType_GrtrEq;
                        }
                    }
                }break;
                
                case '=':
                {
                    if (s < e && *s == '='){
                        ++s;
                        token->type = TokenType_EqEq;
                    }
                }break;
                
                case '!':
                {
                    if (s < e && *s == '='){
                        ++s;
                        token->type = TokenType_NotEq;
                    }
                }break;
                
                case '|':
                {
                    if (s < e && *s == '|'){
                        ++s;
                        token->type = TokenType_Or;
                    }
                }break;
                
                case '&':
                {
                    if (s < e && *s == '&'){
                        ++s;
                        token->type = TokenType_And;
                    }
                }break;
            }
        }
        
        token->text_length = (i32)(s - token->text_pos);
    }
    
    array.count = (i32)(push_array(arena, Token, 0) - array.tokens);
    return(array);
}

////////////////////////////////

internal String_Intern_Table
intern_tokens(Arena *arena, Token_Array tokens){
    i32 max_strings = tokens.count + Keyword_COUNT + BuiltInType_COUNT;
    String_Intern_Table table = string_intern_table(arena, max_strings);
    
#define InternKeyword(N,S) keyword_##N = intern_string(&table, string(S));
    KeywordList(InternKeyword)
#undef InternKeyword
    
#define InternBuiltInType(N,s) built_in_type_name_##N = intern_string(&table, string(#N));
        BuiltInTypeList(InternBuiltInType,0)
#undef InternBuiltInType
    
        Token *token = tokens.tokens;
    for (i32 i = 0; i < tokens.count; ++i, ++token){
        if (token->type != TokenType_Name){
            continue;
        }
        
        token->interned = intern_string(&table, string(token->text_pos, token->text_length));
        
        if (intern_cmp(token->interned, keyword_proc) ||
            intern_cmp(token->interned, keyword_struct) ||
            intern_cmp(token->interned, keyword_const) ||
            intern_cmp(token->interned, keyword_enum) ||
            intern_cmp(token->interned, keyword_persist)){
            token->type = TokenType_TopKeyword;
        }
        else if (intern_cmp(token->interned, keyword_var)){
            token->type = TokenType_VarKeyword;
        }
        else if (intern_cmp(token->interned, keyword_goto) ||
                 intern_cmp(token->interned, keyword_label) ||
                 intern_cmp(token->interned, keyword_block) ||
                 intern_cmp(token->interned, keyword_if) ||
                 intern_cmp(token->interned, keyword_else) ||
                 intern_cmp(token->interned, keyword_for) ||
                 intern_cmp(token->interned, keyword_return)){
            token->type = TokenType_ControlKeyword;
        }
        else if (intern_cmp(token->interned, keyword_cast)){
            token->type = TokenType_CastKeyword;
        }
        else if (intern_cmp(token->interned, keyword_typesize) ||
                 intern_cmp(token->interned, keyword_exprsize) ||
                 intern_cmp(token->interned, keyword_exprtype)){
            token->type = TokenType_ExprKeyword;
        }
        else if (intern_cmp(token->interned, keyword__aligned) ||
                 intern_cmp(token->interned, keyword__free) ||
                 intern_cmp(token->interned, keyword__tight) ||
                 intern_cmp(token->interned, keyword__overlap)){
            token->type = TokenType_PackKeyword;
        }
        else if (intern_cmp(token->interned, keyword__offset) ||
                 intern_cmp(token->interned, keyword__size) ||
                 intern_cmp(token->interned, keyword__align)){
            token->type = TokenType_LayoutKeyword;
        }
    }
    
    return(table);
}

////////////////////////////////

internal void
set_sentinel(Node *s, Node *f, Node *l){
    dll_init_sentinel(s);
    if (f != 0){
        Assert(l != 0);
        dll_insert_list(s, f, l);
    }
}

internal void
set_signature(Procedure_Signature *signature, Node *first, Node *last, Type *ret){
    set_sentinel(&signature->params, first, last);
    signature->ret = ret;
}

internal void
set_struct_body(Struct_Body *body, Pack_Style pack, Node *first, Node *last){
    body->pack = pack;
    set_sentinel(&body->members, first, last);
}

internal Program*
push_program(Parse_Context *ctx, Node *first, Node *last){
    Program *program = push_array(ctx->arena, Program, 1);
    set_sentinel(&program->tops, first, last);
    return(program);
}

internal Top_Statement*
push_procedure(Parse_Context *ctx, char *name, Node *first, Node *last, Type *ret, Top_Statement *block){
    Top_Statement *node = push_array(ctx->arena, Top_Statement, 1);
    block_zero(node, sizeof(*node));
    node->kind = Top_Procedure;
    node->proc.name = name;
    set_signature(&node->proc.signature, first, last, ret);
    node->proc.block = block;
    return(node);
}

internal Top_Statement*
push_struct(Parse_Context *ctx, char *name, Pack_Style pack, Node *first, Node *last){
    Top_Statement *node = push_array(ctx->arena, Top_Statement, 1);
    block_zero(node, sizeof(*node));
    node->kind = Top_Struct;
    node->struct_node.name = name;
    set_struct_body(&node->struct_node.body, pack, first, last);
    return(node);
}

internal Top_Statement*
push_enum(Parse_Context *ctx, char *name, Type *type, Node *first, Node *last){
    Top_Statement *node = push_array(ctx->arena, Top_Statement, 1);
    block_zero(node, sizeof(*node));
    node->kind = Top_Enum;
    node->enum_node.name = name;
    node->enum_node.type = type;
    set_sentinel(&node->enum_node.members, first, last);
    return(node);
}

internal Top_Statement*
push_decl(Parse_Context *ctx, AST_Kind kind, char *name, Type *type, Expr *expr){
    Top_Statement *node = push_array(ctx->arena, Top_Statement, 1);
    block_zero(node, sizeof(*node));
    node->kind = kind;
    node->decl.name = name;
    node->decl.type = type;
    node->decl.expr = expr;
    return(node);
}

internal Procedure_Parameter*
push_procedure_parameter(Parse_Context *ctx, char *name, Type *type){
    Procedure_Parameter *param = push_array(ctx->arena, Procedure_Parameter, 1);
    block_zero(param, sizeof(*param));
    param->kind = ProcedureParameter;
    param->name = name;
    param->type = type;
    return(param);
}

internal Top_Statement*
push_for(Parse_Context *ctx, Top_Statement *init, Expr *check, Top_Statement *inc, Top_Statement *body){
    Top_Statement *node = push_array(ctx->arena, Top_Statement, 1);
    block_zero(node, sizeof(*node));
    node->kind = Statement_For;
    node->for_node.init = init;
    node->for_node.check = check;
    node->for_node.inc = inc;
    node->for_node.body = body;
    return(node);
}

internal Top_Statement*
push_if(Parse_Context *ctx, Expr *check, Top_Statement *body, Top_Statement *else_body){
    Top_Statement *node = push_array(ctx->arena, Top_Statement, 1);
    block_zero(node, sizeof(*node));
    node->kind = Statement_If;
    node->if_node.check = check;
    node->if_node.body = body;
    node->if_node.else_body = else_body;
    return(node);
}

internal Top_Statement*
push_block(Parse_Context *ctx, char *name, Node *first, Node *last){
    Top_Statement *node = push_array(ctx->arena, Top_Statement, 1);
    block_zero(node, sizeof(*node));
    node->kind = Statement_Block;
    node->block.name = name;
    set_sentinel(&node->block.stmnts, first, last);
    return(node);
}

internal Top_Statement*
push_goto(Parse_Context *ctx, Expr *expr){
    Top_Statement *node = push_array(ctx->arena, Top_Statement, 1);
    block_zero(node, sizeof(*node));
    node->kind = Statement_Goto;
    node->goto_expr = expr;
    return(node);
}

internal Top_Statement*
push_label(Parse_Context *ctx, char *name){
    Top_Statement *node = push_array(ctx->arena, Top_Statement, 1);
    block_zero(node, sizeof(*node));
    node->kind = Statement_Label;
    node->label = name;
    return(node);
}

internal Top_Statement*
push_return(Parse_Context *ctx, Expr *expr){
    Top_Statement *node = push_array(ctx->arena, Top_Statement, 1);
    block_zero(node, sizeof(*node));
    node->kind = Statement_Return;
    node->return_expr = expr;
    return(node);
}

internal Top_Statement*
push_assignment(Parse_Context *ctx, Expr *l, Expr *r){
    Top_Statement *node = push_array(ctx->arena, Top_Statement, 1);
    block_zero(node, sizeof(*node));
    node->kind = Statement_Assignment;
    node->assignment.l = l;
    node->assignment.r = r;
    return(node);
}

internal Top_Statement*
push_expression(Parse_Context *ctx, Expr *expr){
    Top_Statement *node = push_array(ctx->arena, Top_Statement, 1);
    block_zero(node, sizeof(*node));
    node->kind = Statement_Expression;
    node->expr = expr;
    return(node);
}

internal Struct_Member*
push_struct_variable(Parse_Context *ctx, char *name, Type *type){
    Struct_Member *node = push_array(ctx->arena, Struct_Member, 1);
    block_zero(node, sizeof(*node));
    node->kind = StructMember_Variable;
    node->variable.name = name;
    node->variable.type = type;
    return(node);
}

internal Struct_Member*
push_struct_constant(Parse_Context *ctx, Top_Statement *decl){
    Struct_Member *node = push_array(ctx->arena, Struct_Member, 1);
    block_zero(node, sizeof(*node));
    node->kind = StructMember_Constant;
    node->decl = decl;
    return(node);
}

internal Struct_Member*
push_struct_layout(Parse_Context *ctx, AST_Kind kind, Expr *expr){
    Struct_Member *node = push_array(ctx->arena, Struct_Member, 1);
    block_zero(node, sizeof(*node));
    node->kind = kind;
    node->layout = expr;
    return(node);
}

internal Struct_Member*
push_struct_sub_body(Parse_Context *ctx, Pack_Style pack, Node *first, Node *last){
    Struct_Member *node = push_array(ctx->arena, Struct_Member, 1);
    block_zero(node, sizeof(*node));
    node->kind = StructMember_SubBody;
    set_struct_body(&node->body, pack, first, last);
    return(node);
}

internal Enum_Member*
push_enum_member(Parse_Context *ctx, char *name, Expr *expr){
    Enum_Member *mem = push_array(ctx->arena, Enum_Member, 1);
    block_zero(mem, sizeof(*mem));
    mem->kind = EnumMember;
    mem->name = name;
    mem->expr = expr;
    return(mem);
}

internal Type*
push_primary_type(Parse_Context *ctx, char *name){
    Type *type = push_array(ctx->arena, Type, 1);
    block_zero(type, sizeof(*type));
    type->kind = Type_Primary;
    type->name = name;
    return(type);
}

internal Type*
push_pointer_type(Parse_Context *ctx, Type *sub){
    Type *type = push_array(ctx->arena, Type, 1);
    block_zero(type, sizeof(*type));
    type->kind = Type_Pointer;
    type->pointer.sub = sub;
    return(type);
}

internal Type*
push_array_type(Parse_Context *ctx, Expr *size, Type *sub){
    Type *type = push_array(ctx->arena, Type, 1);
    block_zero(type, sizeof(*type));
    type->kind = Type_Array;
    type->array.size = size;
    type->array.sub = sub;
    return(type);
}

internal Type*
push_procedure_type(Parse_Context *ctx, Node *first, Node *last, Type *ret){
    Type *type = push_array(ctx->arena, Type, 1);
    block_zero(type, sizeof(*type));
    type->kind = Type_Procedure;
    set_signature(&type->proc, first, last, ret);
    return(type);
}

internal Type*
push_exprtype_type(Parse_Context *ctx, Expr *expr){
    Type *type = push_array(ctx->arena, Type, 1);
    block_zero(type, sizeof(*type));
    type->kind = Type_ExprType;
    type->expr = expr;
    return(type);
}

internal Expr*
push_binary_expr(Parse_Context *ctx, Token_Type op, Expr *l, Expr *r){
    Expr *expr = push_array(ctx->arena, Expr, 1);
    block_zero(expr, sizeof(*expr));
    expr->kind = Expr_Binary;
    expr->binary.op = op;
    expr->binary.l = l;
    expr->binary.r = r;
    return(expr);
}

internal Expr*
push_unary_expr(Parse_Context *ctx, Token_Type op, Expr *u){
    Expr *expr = push_array(ctx->arena, Expr, 1);
    block_zero(expr, sizeof(*expr));
    expr->kind = Expr_Unary;
    expr->unary.op = op;
    expr->unary.u = u;
    return(expr);
}

internal Expr*
push_cast_expr(Parse_Context *ctx, Type *t, Expr *u){
    Expr *expr = push_array(ctx->arena, Expr, 1);
    block_zero(expr, sizeof(*expr));
    expr->kind = Expr_Cast;
    expr->cast.t = t;
    expr->cast.u = u;
    return(expr);
}

internal Expr*
push_call_expr(Parse_Context *ctx, Expr *p, Node *first, Node *last){
    Expr *expr = push_array(ctx->arena, Expr, 1);
    block_zero(expr, sizeof(*expr));
    expr->kind = Expr_Call;
    expr->call.p  = p;
    set_sentinel(&expr->call.args, first, last);
    return(expr);
}

internal Expr*
push_scope_expr(Parse_Context *ctx, Expr *l, char *r){
    Expr *expr = push_array(ctx->arena, Expr, 1);
    block_zero(expr, sizeof(*expr));
    expr->kind = Expr_Scope;
    expr->scope.l  = l;
    expr->scope.r  = r;
    return(expr);
}

internal Expr*
push_name(Parse_Context *ctx, char *name){
    Expr *expr = push_array(ctx->arena, Expr, 1);
    block_zero(expr, sizeof(*expr));
    expr->kind = Expr_Name;
    expr->name = name;
    return(expr);
}

internal Expr*
push_name(Parse_Context *ctx, u64 number){
    Expr *expr = push_array(ctx->arena, Expr, 1);
    block_zero(expr, sizeof(*expr));
    expr->kind = Expr_Number;
    expr->number = number;
    return(expr);
}

internal Expr*
push_string(Parse_Context *ctx, String string){
    Expr *expr = push_array(ctx->arena, Expr, 1);
    block_zero(expr, sizeof(*expr));
    expr->kind = Expr_String;
    expr->string = string;
    return(expr);
}

internal Expr*
push_character(Parse_Context *ctx, char character){
    Expr *expr = push_array(ctx->arena, Expr, 1);
    block_zero(expr, sizeof(*expr));
    expr->kind = Expr_Char;
    expr->character = character;
    return(expr);
}

internal Expr*
push_typesize(Parse_Context *ctx, Type *t){
    Expr *expr = push_array(ctx->arena, Expr, 1);
    block_zero(expr, sizeof(*expr));
    expr->kind = Expr_TypeSize;
    expr->typesize.t = t;
    return(expr);
}

internal Expr*
push_exprsize(Parse_Context *ctx, Expr *e){
    Expr *expr = push_array(ctx->arena, Expr, 1);
    block_zero(expr, sizeof(*expr));
    expr->kind = Expr_ExprSize;
    expr->exprsize.e = e;
    return(expr);
}

internal Expr_Node*
push_expr_node(Parse_Context *ctx, Expr *e){
    Expr_Node *node = push_array(ctx->arena, Expr_Node, 1);
    block_zero(node, sizeof(*node));
    node->expr = e;
    return(node);
}

////////////////////////////////

#define require(c) Stmnt( if ((c) == 0) { return(0); } )

internal void
token_next(Parse_Context *ctx){
    ctx->index += 1;
    if (ctx->index < ctx->tokens.count){
        ctx->current = ctx->tokens.tokens[ctx->index];
    }
    else{
        block_zero(&ctx->current, sizeof(ctx->current));
    }
}

internal Token_Type
token_get_type(Parse_Context *ctx){
    return(ctx->current.type);
}

internal b32
token_is(Parse_Context *ctx, Token_Type type){
    return(ctx->current.type == type);
}

internal b32
token_is(Parse_Context *ctx, char *interned){
    return(intern_cmp(ctx->current.interned, interned));
}

internal b32
token_match(Parse_Context *ctx, Token_Type type){
    b32 result = token_is(ctx, type);
    if (result){
        token_next(ctx);
    }
    return(result);
}

internal b32
token_match(Parse_Context *ctx, char *interned){
    b32 result = token_is(ctx, interned);
    if (result){
        token_next(ctx);
    }
    return(result);
}

internal b32
token_match_name(Parse_Context *ctx, char **out){
    b32 result = token_is(ctx, TokenType_Name);
    if (result){
        *out = ctx->current.interned;
        token_next(ctx);
    }
    return(result);
}

internal Top_Statement*
parse__top_statement(Parse_Context *ctx);

internal Top_Statement*
parse__restricted_statement(Parse_Context *ctx);

internal Type*
parse__type(Parse_Context *ctx);

internal Expr*
parse__expression(Parse_Context *ctx);

internal Expr*
parse__factor(Parse_Context *ctx){
    if (token_is(ctx, TokenType_Name)){
        char *name = 0;
        require(token_match_name(ctx, &name));
        Expr *e = push_name(ctx, name);
        return(e);
    }
    else if (token_is(ctx, TokenType_Number)){
        // TODO(allen): // TODO(allen): // TODO(allen): 
        u64 value = 0;
        token_next(ctx);
        Expr *e = push_name(ctx, value);
        return(e);
    }
    else if (token_is(ctx, TokenType_Character)){
        // TODO(allen): // TODO(allen): // TODO(allen): 
        char character = 0;
        token_next(ctx);
        Expr *e = push_character(ctx, character);
        return(e);
    }
    else if (token_is(ctx, TokenType_String)){
        // TODO(allen): // TODO(allen): // TODO(allen): 
        String string = {0};
        token_next(ctx);
        Expr *e = push_string(ctx, string);
        return(e);
    }
    else if (token_is(ctx, keyword_typesize)){
        token_next(ctx);
        require(token_match(ctx, '('));
        Type *type = parse__type(ctx);
        require(type);
        require(token_match(ctx, ')'));
        Expr *e = push_typesize(ctx, type);
        return(e);
    }
    else if (token_is(ctx, keyword_exprsize)){
        token_next(ctx);
        require(token_match(ctx, '('));
        Expr *expr = parse__expression(ctx);
        require(expr);
        require(token_match(ctx, ')'));
        Expr *e = push_exprsize(ctx, expr);
        return(e);
    }
    else if (token_is(ctx, '(')){
        token_next(ctx);
        Expr *expr = parse__expression(ctx);
        require(expr);
        require(token_match(ctx, ')'));
        return(expr);
    }
    return(0);
}

internal Expr*
parse__postfix(Parse_Context *ctx){
    Expr *f = parse__factor(ctx);
    require(f);
    
    if (token_match(ctx, '[')){
        Expr *index = parse__expression(ctx);
        require(index);
        require(token_match(ctx, ']'));
        Expr *p = push_binary_expr(ctx, '[', f, index);
        return(p);
    }
    else if (token_match(ctx, '(')){
        Node sent = {0};
        b32 first_arg = true;
        for (;!token_is(ctx, ')') && !token_is(ctx, TokenType_EOF);){
            if (first_arg){
                first_arg = false;
            }
            else{
                require(token_match(ctx, ','));
            }
            Expr *arg = parse__expression(ctx);
            require(arg);
            zdll_push(sent.first, sent.last, &push_expr_node(ctx, arg)->node);
        }
        require(token_match(ctx, ')'));
        
        Expr *p = push_call_expr(ctx, f, sent.first, sent.last);
        return(p);
    }
    else if (token_match(ctx, '.')){
        char *name = 0;
        require(token_match_name(ctx, &name));
        Expr *p = push_scope_expr(ctx, f, name);
        return(p);
    }
    
    return(f);
}

internal Expr*
parse__unary(Parse_Context *ctx){
    if (token_is(ctx, '-') ||
        token_is(ctx, '!') ||
        token_is(ctx, '~') ||
        token_is(ctx, '&') ||
        token_is(ctx, '*') ||
        false){
        Token_Type type = token_get_type(ctx);
        token_next(ctx);
        Expr *u = parse__unary(ctx);
        require(u);
        Expr *e = push_unary_expr(ctx, type, u);
        return(e);
    }
    else if (token_is(ctx, TokenType_CastKeyword)){
        token_next(ctx);
        require(token_match(ctx, '('));
        Type *type = parse__type(ctx);
        require(type);
        require(token_match(ctx, ')'));
        Expr *u = parse__unary(ctx);
        require(u);
        Expr *e = push_cast_expr(ctx, type, u);
        return(e);
    }
    else{
        return(parse__postfix(ctx));
    }
}

internal Expr*
parse__term(Parse_Context *ctx){
    Expr *l = parse__unary(ctx);
    require(l);
    Expr *r = 0;
    for (;  token_is(ctx, '*')
         || token_is(ctx, '/')
         || token_is(ctx, '%')
         || token_is(ctx, '&')
         || token_is(ctx, TokenType_LShift)
         || token_is(ctx, TokenType_RShift);){
        Token_Type type = token_get_type(ctx);
        token_next(ctx);
        r = parse__unary(ctx);
        require(r);
        l = push_binary_expr(ctx, type, l, r);
    }
    return(l);
}

internal Expr*
parse__formula(Parse_Context *ctx){
    Expr *l = parse__term(ctx);
    require(l);
    Expr *r = 0;
    for (;  token_is(ctx, '+')
         || token_is(ctx, '-')
         || token_is(ctx, '|')
         || token_is(ctx, '^');){
        Token_Type type = token_get_type(ctx);
        token_next(ctx);
        r = parse__term(ctx);
        require(r);
        l = push_binary_expr(ctx, type, l, r);
    }
    return(l);
}

internal Expr*
parse__predicate(Parse_Context *ctx){
    Expr *l = parse__formula(ctx);
    require(l);
    Expr *r = 0;
    for (;  token_is(ctx, TokenType_EqEq)
         || token_is(ctx, TokenType_NotEq)
         || token_is(ctx, '<')
         || token_is(ctx, '>')
         || token_is(ctx, TokenType_LessEq)
         || token_is(ctx, TokenType_GrtrEq);){
        Token_Type type = token_get_type(ctx);
        token_next(ctx);
        r = parse__formula(ctx);
        require(r);
        l = push_binary_expr(ctx, type, l, r);
    }
    return(l);
}

internal Expr*
parse__clause(Parse_Context *ctx){
    Expr *l = parse__predicate(ctx);
    require(l);
    Expr *r = 0;
    for (;token_is(ctx, TokenType_And);){
        Token_Type type = token_get_type(ctx);
        token_next(ctx);
        r = parse__predicate(ctx);
        require(r);
        l = push_binary_expr(ctx, type, l, r);
    }
    return(l);
}

internal Expr*
parse__expression(Parse_Context *ctx){
    Expr *l = parse__clause(ctx);
    require(l);
    Expr *r = 0;
    for (;token_is(ctx, TokenType_Or);){
        Token_Type type = token_get_type(ctx);
        token_next(ctx);
        r = parse__clause(ctx);
        require(r);
        l = push_binary_expr(ctx, type, l, r);
    }
    return(l);
}

internal Type*
parse__type(Parse_Context *ctx){
    char *name = 0;
    if (token_match_name(ctx, &name)){
        return(push_primary_type(ctx, name));
    }
    else if (token_match(ctx, '*')){
        Type *type = parse__type(ctx);
        require(type);
        return(push_pointer_type(ctx, type));
    }
    else if (token_match(ctx, '[')){
        Expr *expr = parse__expression(ctx);
        require(expr);
        require(token_match(ctx, ']'));
        Type *type = parse__type(ctx);
        require(type);
        return(push_array_type(ctx, expr, type));
    }
    else if (token_match(ctx, '(')){
        Node sent = {0};
        for (;!token_is(ctx, ')') && !token_is(ctx, TokenType_EOF);){
            char *param_name = 0;
            token_match_name(ctx, &param_name);
            require(token_match(ctx, ':'));
            Type *param_type = parse__type(ctx);
            require(param_type);
            Procedure_Parameter *param = push_procedure_parameter(ctx, param_name, param_type);
            zdll_push(sent.first, sent.last, &param->node);
        }
        require(token_match(ctx, ')'));
        require(token_match(ctx, ':'));
        Type *ret_type = parse__type(ctx);
        require(ret_type);
        return(push_procedure_type(ctx, sent.first, sent.last, ret_type));
    }
    else if (token_match(ctx, keyword_exprtype)){
        Expr *expr = parse__expression(ctx);
        require(expr);
        return(push_exprtype_type(ctx, expr));
    }
    return(0);
}

internal Top_Statement*
parse__block_inner(Parse_Context *ctx, char *name){
    require(token_match(ctx, '{'));
    Node sent = {0};
    for (;!token_is(ctx, '}') && !token_is(ctx, TokenType_EOF);){
        Top_Statement *top_stmnt = parse__top_statement(ctx);
        require(top_stmnt);
        zdll_push(sent.first, sent.last, &top_stmnt->node);
    }
    require(token_match(ctx, '}'));
    Top_Statement *block = push_block(ctx, name, sent.first, sent.last);
    return(block);
}

internal Top_Statement*
parse__unnamed_block(Parse_Context *ctx){
    return(parse__block_inner(ctx, 0));
}

internal Top_Statement*
parse__proc(Parse_Context *ctx){
    require(token_match(ctx, keyword_proc));
    
    char *name = 0;
    require(token_match_name(ctx, &name));
    require(token_match(ctx, '('));
    
    b32 first_parameter = true;
    Node sent = {0};
    for (;!token_is(ctx, ')') && !token_is(ctx, TokenType_EOF);){
        if (first_parameter){
            first_parameter = false;
        }
        else{
            require(token_match(ctx, ','));
        }
        
        char *param_name = 0;
        require(token_match_name(ctx, &param_name));
        
        require(token_match(ctx, ':'));
        
        Type *param_type = parse__type(ctx);
        require(param_type);
        
        Procedure_Parameter *param = push_procedure_parameter(ctx, param_name, param_type);
        zdll_push(sent.next, sent.prev, &param->node);
    }
    
    require(token_match(ctx, ')'));
    require(token_match(ctx, ':'));
    
    Type *ret_type = parse__type(ctx);
    require(ret_type);
    
    Top_Statement *block = parse__unnamed_block(ctx);
    require(block);
    
    Top_Statement *proc = push_procedure(ctx, name, sent.next, sent.prev, ret_type, block);
    return(proc);
}

internal void*
parse__struct_body(Parse_Context *ctx, char *name){
    Pack_Style pack = Pack_Aligned;
    if (token_is(ctx, TokenType_PackKeyword)){
        if (token_match(ctx, keyword__aligned)){
            pack = Pack_Aligned;
        }
        else if (token_match(ctx, keyword__free)){
            pack = Pack_Free;
        }
        else if (token_match(ctx, keyword__tight)){
            pack = Pack_Tight;
        }
        else if (token_match(ctx, keyword__overlap)){
            pack = Pack_Overlap;
        }
    }
    
    require(token_match(ctx, '{'));
    
    Node sent = {0};
    for (;!token_is(ctx, '}') && !token_is(ctx, TokenType_EOF);){
        Struct_Member *mem = 0;
        
        char *member_name = 0;
        if (token_match_name(ctx, &member_name)){
            require(token_match(ctx, ':'));
            Type *type = parse__type(ctx);
            require(type);
            require(token_match(ctx, ';'));
            mem = push_struct_variable(ctx, member_name, type);
            require(mem);
        }
        else if (token_is(ctx, TokenType_LayoutKeyword)){
            AST_Kind layout_kind = 0;
            if (token_match(ctx, keyword__offset)){
                layout_kind= StructMember_LayoutOffset;
            }
            else if (token_match(ctx, keyword__size)){
                layout_kind= StructMember_LayoutSize;
            }
            else if (token_match(ctx, keyword__align)){
                layout_kind= StructMember_LayoutAlign;
            }
            else{
                return(0);
            }
            
            Expr *expr = parse__expression(ctx);
            require(expr);
            require(token_match(ctx, ';'));
            mem = push_struct_layout(ctx, layout_kind, expr);
            require(mem);
        }
        else if (token_is(ctx, TokenType_PackKeyword) || token_is(ctx, '{')){
            mem = (Struct_Member*)parse__struct_body(ctx, 0);
            require(mem);
        }
        else{
            return(0);
        }
        
        zdll_push(sent.first, sent.last, &mem->node);
    }
    
    require(token_match(ctx, '}'));
    
    if (name == 0){
        Struct_Member *mem = push_struct_sub_body(ctx, pack, sent.first, sent.last);
        return(mem);
    }
    else{
        Top_Statement *top = push_struct(ctx, name, pack, sent.first, sent.last);
        return(top);
    }
}

internal Top_Statement*
parse__struct(Parse_Context *ctx){
    require(token_match(ctx, keyword_struct));
    char *name = 0;
    require(token_match_name(ctx, &name));
    return((Top_Statement*)parse__struct_body(ctx, name));
}

internal Top_Statement*
parse__constant(Parse_Context *ctx){
    require(token_match(ctx, keyword_const));
    char *name = 0;
    require(token_match_name(ctx, &name));
    require(token_match(ctx, ':'));
    Type *type = parse__type(ctx);
    require(type);
    require(token_match(ctx, '='));
    Expr *expr = parse__expression(ctx);
    require(token_match(ctx, ';'));
    Top_Statement *top = push_decl(ctx, Top_Constant, name, type, expr);
    return(top);
}

internal Enum_Member*
parse__enum_member(Parse_Context *ctx){
    char *name = 0;
    require(token_match_name(ctx, &name));
    require(token_match(ctx, '='));
    Expr *expr = parse__expression(ctx);
    require(expr);
    require(token_match(ctx, ';'));
    Enum_Member *mem = push_enum_member(ctx, name, expr);
    return(mem);
}

internal Top_Statement*
parse__enum(Parse_Context *ctx){
    require(token_match(ctx, keyword_enum));
    char *name = 0;
    require(token_match_name(ctx, &name));
    require(token_match(ctx, ':'));
    Type *type = parse__type(ctx);
    require(type);
    
    Node sent = {0};
    require(token_match(ctx, '{'));
    for (;!token_is(ctx, '}') && !token_is(ctx, TokenType_EOF);){
        Enum_Member *mem = parse__enum_member(ctx);
        require(mem);
        zdll_push(sent.first, sent.last, &mem->node);
    }
    require(token_match(ctx, '}'));
    
    Top_Statement *top = push_enum(ctx, name, type, sent.first, sent.last);
    return(top);
}

internal Top_Statement*
parse__persist(Parse_Context *ctx){
    require(token_match(ctx, keyword_persist));
    char *name = 0;
    require(token_match_name(ctx, &name));
    require(token_match(ctx, ':'));
    Type *type = parse__type(ctx);
    require(type);
    Expr *expr = 0;
    if (token_match(ctx, '=')){
        expr = parse__expression(ctx);
        require(token_match(ctx, ';'));
    }
    Top_Statement *top = push_decl(ctx, Top_Persist, name, type, expr);
    return(top);
}

internal Top_Statement*
parse__top(Parse_Context *ctx){
    if (token_is(ctx, keyword_proc)){
        return(parse__proc(ctx));
    }
    else if (token_is(ctx, keyword_struct)){
        return(parse__struct(ctx));
    }
    else if (token_is(ctx, keyword_const)){
        return(parse__constant(ctx));
    }
    else if (token_is(ctx, keyword_enum)){
        return(parse__enum(ctx));
    }
    else if (token_is(ctx, keyword_persist)){
        return(parse__persist(ctx));
    }
    return(0);
}

internal Top_Statement*
parse__opt_assignment(Parse_Context *ctx){
    Expr *l = parse__expression(ctx);
    require(l);
    
    if (token_is(ctx, '=')){
        token_next(ctx);
        
        Expr *r = parse__expression(ctx);
        require(r);
        
        Top_Statement *stmnt = push_assignment(ctx, l, r);
        return(stmnt);
    }
    else{
        Top_Statement *stmnt = push_expression(ctx, l);
        return(stmnt);
    }
}

internal Top_Statement*
parse__decl(Parse_Context *ctx){
    require(token_match(ctx, keyword_var));
    char *name = 0;
    require(token_match_name(ctx, &name));
    require(token_match(ctx, ':'));
    Type *type = parse__type(ctx);
    require(type);
    Expr *expr = 0;
    if (token_match(ctx, '=')){
        expr = parse__expression(ctx);
        require(expr);
    }
    return(push_decl(ctx, Statement_Decl, name, type, expr));
}

internal Top_Statement*
parse__decl_or_opt_assignment(Parse_Context *ctx){
    if (token_is(ctx, keyword_var)){
        return(parse__decl(ctx));
    }
    else{
        return(parse__opt_assignment(ctx));
    }
}

internal Top_Statement*
parse__for(Parse_Context *ctx){
    require(token_match(ctx, keyword_for));
    require(token_match(ctx, '('));
    Top_Statement *init = parse__decl_or_opt_assignment(ctx);
    require(token_match(ctx, ';'));
    Expr *check = parse__expression(ctx);
    require(token_match(ctx, ';'));
    Top_Statement *inc = parse__opt_assignment(ctx);
    require(token_match(ctx, ')'));
    
    Top_Statement *body = parse__restricted_statement(ctx);
    require(body);
    
    Top_Statement *stmnt = push_for(ctx, init, check, inc, body);
    return(stmnt);
}

internal Top_Statement*
parse__if(Parse_Context *ctx){
    require(token_match(ctx, keyword_if));
    require(token_match(ctx, '('));
    Expr *check = parse__expression(ctx);
    require(check);
    require(token_match(ctx, ')'));
    
    Top_Statement *body = parse__restricted_statement(ctx);
    require(body);
    
    Top_Statement *else_body = 0;
    if (token_match(ctx, keyword_else)){
        else_body = parse__restricted_statement(ctx);
        require(else_body);
    }
    
    Top_Statement *stmnt = push_if(ctx, check, body, else_body);
    return(stmnt);
}

internal Top_Statement*
parse__block(Parse_Context *ctx){
    char *name = 0;
    if (token_match(ctx, keyword_block)){
        require(token_match_name(ctx, &name));
    }
    return(parse__block_inner(ctx, name));
}

internal Top_Statement*
parse__goto(Parse_Context *ctx){
    require(token_match(ctx, keyword_goto));
    char *name = 0;
    require(token_match_name(ctx, &name));
    Expr *expr = push_name(ctx, name);
    for (;token_match(ctx, '.');){
        require(token_match_name(ctx, &name));
        expr = push_scope_expr(ctx, expr, name);
    }
    Top_Statement *top = push_goto(ctx, expr);
    return(top);
}

internal Top_Statement*
parse__return(Parse_Context *ctx){
    require(token_match(ctx, keyword_return));
    require(token_match(ctx, '('));
    Expr *expr = parse__expression(ctx);
    require(expr);
    require(token_match(ctx, ')'));
    Top_Statement *top = push_return(ctx, expr);
    return(top);
}

internal Top_Statement*
parse__restricted_statement(Parse_Context *ctx){
    if (token_is(ctx, keyword_for)){
        return(parse__for(ctx));
    }
    else if (token_is(ctx, keyword_if)){
        return(parse__if(ctx));
    }
    else if (token_is(ctx, keyword_block) || token_is(ctx, '{')){
        return(parse__block(ctx));
    }
    else if (token_is(ctx, keyword_goto)){
        Top_Statement *stmnt = parse__goto(ctx);
        require(stmnt);
        require(token_match(ctx, ';'));
        return(stmnt);
    }
    else if (token_is(ctx, keyword_return)){
        Top_Statement *stmnt = parse__return(ctx);
        require(stmnt);
        require(token_match(ctx, ';'));
        return(stmnt);
    }
    else{
        Top_Statement *stmnt = parse__opt_assignment(ctx);
        require(stmnt);
        require(token_match(ctx, ';'));
        return(stmnt);
    }
    return(0);
}

internal Top_Statement*
parse__statement(Parse_Context *ctx){
    if (token_is(ctx, keyword_var)){
        Top_Statement *top = parse__decl(ctx);
        require(token_match(ctx, ';'));
        return(top);
    }
    else if (token_is(ctx, keyword_label)){
        token_next(ctx);
        char *name = 0;
        require(token_match_name(ctx, &name));
        require(token_match(ctx, ';'));
        return(push_label(ctx, name));
    }
    else{
        return(parse__restricted_statement(ctx));
    }
}

internal Top_Statement*
parse__top_statement(Parse_Context *ctx){
    if (token_is(ctx, TokenType_TopKeyword)){
        return(parse__top(ctx));
    }
    else{
        return(parse__statement(ctx));
    }
}

internal Program*
parse__inner(Parse_Context *ctx){
    Node sent = {0};
    for (;!token_is(ctx, TokenType_EOF);){
        Top_Statement *top = parse__top(ctx);
        require(top);
        zdll_push(sent.first, sent.last, &top->node);
    }
    Program *program = push_program(ctx, sent.first, sent.last);
    Assert(program != 0);
    return(program);
}

internal Program*
parse(Arena *arena, Token_Array tokens){
    Temp_Memory temp = begin_temp(arena);
    Parse_Context ctx = {0};
    ctx.arena = arena;
    ctx.tokens = tokens;
    ctx.index = -1;
    token_next(&ctx);
    Program *program = parse__inner(&ctx);
    if (program == 0){
        end_temp(temp);
    }
    return(program);
}

////////////////////////////////

internal Name_Entry*
lookup_name_one_level(Name_Space *space, char *name){
    Name_Entry *result = 0;
    Node *sent = &space->sent;
    for (Node *n = sent->next;
         n != sent;
         n = n->next){
        Name_Entry *entry_ptr = (Name_Entry*)n;
        if (intern_cmp(name, entry_ptr->name)){
            result = entry_ptr;
            break;
        }
    }
    return(result);
}

internal Name_Entry*
lookup_name_recurse(Name_Space *space, char *name){
    Name_Entry *entry = lookup_name_one_level(space, name);
    if (entry == 0 && space->parent != 0){
        entry = lookup_name_recurse(space->parent, name);
    }
    return(entry);
}

internal Name_Entry*
lookup_name(Name_Space *space, char *name, b32 recurse){
    if (recurse){
        return(lookup_name_recurse(space, name));
    }
    else{
        return(lookup_name_one_level(space, name));
    }
}

internal Name_Space*
fill_names__new_space(Arena *arena, Name_Space *parent){
    Name_Space *space = push_array(arena, Name_Space, 1);
    block_zero(space, sizeof(*space));
    space->parent = parent;
    dll_init_sentinel(&space->sent);
    return(space);
}

internal b32
fill_names__add_entry(Arena *arena, Name_Space *containing_space, char *name, void *ptr, Name_Space *space, b32 recurse){
    b32 result = false;
    if (lookup_name(containing_space, name, recurse) == 0){
        result = true;
        Node *sent = &containing_space->sent;
        Name_Entry *entry = push_array(arena, Name_Entry, 1);
        dll_insert_behind(sent, &entry->node);
        entry->name = name;
        entry->ptr = ptr;
        entry->space = space;
    }
    return(result);
}

internal b32
fill_names__variable_decl(Arena *arena, Name_Space *containing_space, Top_Statement *decl, b32 recurse){
    require(fill_names__add_entry(arena, containing_space, decl->decl.name, decl, 0, recurse));
    return(true);
}

internal b32
fill_names__struct_body(Arena *arena, Name_Space *containing_space, Struct_Body *body){
    Node *sent = &body->members;
    for (Node *n = sent->next;
         n != sent;
         n = n->next){
        Struct_Member *member = CastFromMember(Struct_Member, node, n);
        switch (member->kind){
            case StructMember_Variable:
            {
                require(fill_names__add_entry(arena, containing_space, member->variable.name, member, 0, false));
            }break;
            
            case StructMember_Constant:
            {
                require(fill_names__variable_decl(arena, containing_space, member->decl, false));
            }break;
            
            case StructMember_LayoutOffset:
            case StructMember_LayoutSize:
            case StructMember_LayoutAlign:
            {}break;
            
            case StructMember_SubBody:
            {
                require(fill_names__struct_body(arena, containing_space, &member->body));
            }break;
            
            default:
            {
                return(false);
            }break;
        }
    }
    return(true);
}

internal b32
fill_names__enum(Arena *arena, Name_Space *containing_space, Top_Statement *enum_node){
    Node *sent = &enum_node->enum_node.members;
    for (Node *n = sent->first;
         n != sent;
         n = n->next){
        Enum_Member *member = CastFromMember(Enum_Member, node, n);
        switch (member->kind){
            case EnumMember:
            {
                require(fill_names__add_entry(arena, containing_space, member->name, member, 0, true));
            }break;
            
            default:
            {
                return(false);
            }break;
        }
    }
    return(true);
}

internal b32
fill_names__top_statement(Arena *arena, Name_Space *containing_space, Top_Statement *top){
    switch (top->kind){
        case Top_Procedure:
        {
            Name_Space *proc_space = fill_names__new_space(arena, containing_space);
            top->proc.space = proc_space;
            require(fill_names__add_entry(arena, containing_space, top->proc.name, top, proc_space, true));
            Node *sent = &top->proc.signature.params;
            for (Node *n = sent->next;
                 n != sent;
                 n = n->next){
                Procedure_Parameter *param = CastFromMember(Procedure_Parameter, node, n);
                require(fill_names__add_entry(arena, proc_space, param->name, param, 0, true));
            }
            require(fill_names__top_statement(arena, proc_space, top->proc.block));
        }break;
        
        case Top_Struct:
        {
            Name_Space *struct_space = fill_names__new_space(arena, containing_space);
            top->struct_node.space = struct_space;
            require(fill_names__add_entry(arena, containing_space, top->proc.name, top, struct_space, true));
            require(fill_names__struct_body(arena, struct_space, &top->struct_node.body));
        }break;
        
        case Top_Enum:
        {
            require(fill_names__add_entry(arena, containing_space, top->enum_node.name, top, 0, true));
            require(fill_names__enum(arena, containing_space, top));
        }break;
        
        case Top_Constant:
        case Top_Persist:
        {
            require(fill_names__variable_decl(arena, containing_space, top, true));
        }break;
        
        case Statement_For:
        {
            Name_Space *for_space = fill_names__new_space(arena, containing_space);
            top->for_node.space = for_space;
            require(fill_names__top_statement(arena, for_space, top->for_node.init));
            require(fill_names__top_statement(arena, for_space, top->for_node.inc));
            require(fill_names__top_statement(arena, for_space, top->for_node.body));
        }break;
        
        case Statement_If:
        {
            Name_Space *if_space = fill_names__new_space(arena, containing_space);
            top->if_node.space = if_space;
            require(fill_names__top_statement(arena, if_space, top->if_node.body));
            if (top->if_node.else_body != 0){
                require(fill_names__top_statement(arena, if_space, top->if_node.else_body));
            }
        }break;
        
        case Statement_Block:
        {
            Name_Space *block_space = fill_names__new_space(arena, containing_space);
            top->block.space = block_space;
            if (top->block.name != 0){
                require(fill_names__add_entry(arena, containing_space, top->block.name, top, block_space, true));
            }
            Node *sent = &top->block.stmnts;
            for (Node *n = sent->next;
                 n != sent;
                 n = n->next){
                Top_Statement *top_stmnt = CastFromMember(Top_Statement, node, n);
                require(fill_names__top_statement(arena, block_space, top_stmnt));
            }
        }break;
        
        case Statement_Label:
        {
            require(fill_names__add_entry(arena, containing_space, top->label, top, 0, true));
        }break;
        
        case Statement_Decl:
        {
            require(fill_names__variable_decl(arena, containing_space, top, true));
        }break;
        
        case Statement_Goto:
        case Statement_Return:
        case Statement_Assignment:
        case Statement_Expression:
        {}break;
        
        default:
        {
            return(false);
        }break;
    }
    return(true);
}

internal Name_Space*
fill_names__program(Arena *arena, Program *program){
    Name_Space *space = fill_names__new_space(arena, 0);
    program->space = space;
    Node *sent = &program->tops;
    for (Node *n = sent->next;
         n != sent;
         n = n->next){
        Top_Statement *top = CastFromMember(Top_Statement, node, n);
        require(fill_names__top_statement(arena, space, top));
    }
    return(space);
}

internal Name_Space*
fill_names(Arena *arena, Program *program){
    Temp_Memory temp = begin_temp(arena);
    Name_Space *space = fill_names__program(arena, program);
    if (space == 0){
        end_temp(temp);
    }
    return(space);
}

////////////////////////////////

internal Type*
type_check__decl_built_in(Arena *arena, Name_Space *space, char *name, i32 size){
    Type *type = push_array(arena, Type, 1);
    block_zero(type, sizeof(*type));
    type->kind = BuiltInType;
    type->size = size;
    type->align = size;
    fill_names__add_entry(arena, space, name, type, 0, false);
    return(type);
}

internal Type*
type_check__get_type_of_expr(Name_Space *space, Expr *expr){
    return(0);
}

internal Type*
type_check__resolve_type(Name_Space *space, Type *type){
    return(0);
}

internal b32
type_check__can_do_assignment(Type *l_type, Type *r_type){
    return(intern_cmp(l_type, r_type));
}

internal b32
type_check__is_integer(Type *type){
    if (type == type_i8 ||
        type == type_i16 ||
        type == type_i32 ||
        type == type_i64 ||
        type == type_imem ||
        type == type_u8 ||
        type == type_u16 ||
        type == type_u32 ||
        type == type_u64 ||
        type == type_umem){
        return(true);
    }
    return(false);
}

internal b32
type_check__is_boolean(Type *type){
    if (type == type_b8 ||
        type == type_b32){
        return(true);
    }
    return(false);
}

internal b32
type_check__is_goto_expr(Expr *expr){
    return(false);
}

internal b32
type_check__is_lvalue_expr(Expr *expr){
    return(false);
}

internal b32
type_check__struct_body(Name_Space *space, Struct_Body *body);

internal b32
type_check__struct_member(Name_Space *space, Struct_Member *member){
    switch (member->kind){
        case StructMember_Variable:
        {}break;
        
        case StructMember_Constant:
        {
            Top_Statement *decl = member->decl;
            Type *expr_type = type_check__get_type_of_expr(space, decl->decl.expr);
            Type *resolved_type = type_check__resolve_type(space, decl->decl.type);
            require(type_check__can_do_assignment(resolved_type, expr_type));
        }break;
        
        case StructMember_LayoutOffset:
        case StructMember_LayoutSize:
        case StructMember_LayoutAlign:
        {
            Type *expr_type = type_check__get_type_of_expr(space, member->layout);
            require(type_check__is_integer(expr_type));
        }break;
        
        case StructMember_SubBody:
        {
            require(type_check__struct_body(space, &member->body));
        }break;
        
        default:
        {
            InvalidPath;
        }break;
    }
    return(false);
}

internal b32
type_check__struct_body(Name_Space *space, Struct_Body *body){
    Node *sent = &body->members;
    for (Node *n = sent->next;
         n != sent;
         n = n->next){
        Struct_Member *member = CastFromMember(Struct_Member, node, n);
        require(type_check__struct_member(space, member));
    }
    return(true);
}

internal b32
type_check__top_statement(Top_Statement *current_proc, Name_Space *space, Top_Statement *top){
    switch (top->kind){
        case Top_Procedure:
        {
            Top_Statement *block = top->proc.block;
            require(type_check__top_statement(top, top->proc.space, block));
        }break;
        
        case Top_Struct:
        {
            Struct_Body *body = &top->struct_node.body;
            require(type_check__struct_body(top->struct_node.space, body));
        }break;
        
        case Top_Enum:
        {
            Type *resolved_type = type_check__resolve_type(space, top->enum_node.type);
            Node *sent = &top->enum_node.members;
            for (Node *n = sent->next;
                 n != sent;
                 n = n->next){
                Enum_Member *member = CastFromMember(Enum_Member, node, n);
                Type *expr_type = type_check__get_type_of_expr(space, member->expr);
                require(type_check__can_do_assignment(resolved_type, expr_type));
            }
        }break;
        
        case Top_Constant:
        case Top_Persist:
        case Statement_Decl:
        {
            Type *resolved_type = type_check__resolve_type(space, top->decl.type);
            Type *expr_type = type_check__get_type_of_expr(space, top->decl.expr);
            require(type_check__can_do_assignment(resolved_type, expr_type));
        }break;
        
        case Statement_For:
        {
            Name_Space *for_space = top->for_node.space;
            
            if (top->for_node.init != 0){
                require(type_check__top_statement(current_proc, for_space, top->for_node.init));
            }
            
            if (top->for_node.check != 0){
                Type *expr_type = type_check__get_type_of_expr(for_space, top->for_node.check);
                require(type_check__is_boolean(expr_type));
            }
            
            if (top->for_node.inc != 0){
                require(type_check__top_statement(current_proc, for_space, top->for_node.init));
            }
            
            require(type_check__top_statement(current_proc, for_space, top->for_node.body));
        }break;
        
        case Statement_If:
        {
            Name_Space *if_space = top->if_node.space;
            
            Type *expr_type = type_check__get_type_of_expr(if_space, top->if_node.check);
            require(type_check__is_boolean(expr_type));
            
            require(type_check__top_statement(current_proc, if_space, top->if_node.body));
            
            if (top->if_node.else_body != 0){
                require(type_check__top_statement(current_proc, if_space, top->if_node.else_body));
            }
        }break;
        
        case Statement_Block:
        {
            Name_Space *block_space = top->block.space;
            
            Node *sent = &top->block.stmnts;
            for (Node *n = sent->next;
                 n != sent;
                 n = n->next){
                Top_Statement *stmnt = CastFromMember(Top_Statement, node, n);
                require(type_check__top_statement(current_proc, block_space, stmnt));
            }
        }break;
        
        case Statement_Goto:
        {
            require(type_check__is_goto_expr(top->goto_expr));
        }break;
        
        case Statement_Label:
        {}break;
        
        case Statement_Return:
        {
            Type *expr_type = type_check__get_type_of_expr(space, top->return_expr);
            require(type_check__can_do_assignment(current_proc->proc.signature.ret, expr_type));
        }break;
        
        case Statement_Assignment:
        {
            require(type_check__is_lvalue_expr(top->assignment.l));
            Type *l_type = type_check__get_type_of_expr(space, top->assignment.l);
            Type *r_type = type_check__get_type_of_expr(space, top->assignment.r);
            require(type_check__can_do_assignment(l_type, r_type));
        }break;
        
        case Statement_Expression:
        {
            Type *type = type_check__get_type_of_expr(space, top->expr);
            require(type != 0);
        }break;
        
        default:
        {
            InvalidPath;
        }break;
    }
    return(true);
}

internal b32
type_check(Arena *arena, Program *program){
    Name_Space *built_in_space = fill_names__new_space(arena, 0);
    
#define DeclBuiltInType(N,s) type_##N = type_check__decl_built_in(arena, built_in_space, #N, s);
    BuiltInTypeList(DeclBuiltInType, 8)
#undef DeclBuiltInType
        program->space->parent = built_in_space;
    
    Node *sent = &program->tops;
    for (Node *n = sent->next;
         n != sent;
         n = n->next){
        Top_Statement *top = CastFromMember(Top_Statement, node, n);
        require(type_check__top_statement(0, program->space, top));
    }
    
    return(true);
}

// BOTTOM


