/*
Allen Webster
30.03.2018
*/

// TOP

internal void
print_token_array(FILE *out, Token_Array array){
    fprintf(stdout, "token count: %d\n", array.count);
    for (i32 i = 0; i < array.count; ++i){
        Token token = array.tokens[i];
        switch (token.type){
            case TokenType_TopKeyword:
            case TokenType_VarKeyword:
            case TokenType_ControlKeyword:
            case TokenType_CastKeyword:
            case TokenType_ExprKeyword:
            case TokenType_PackKeyword:
            case TokenType_LayoutKeyword:
            {
                fprintf(out, "KEY: %.*s\n", token.text_length, token.text_pos);
            }break;
            
            case TokenType_Name:
            {
                fprintf(out, "NAME: %.*s\n", token.text_length, token.text_pos);
            }break;
            
            case TokenType_Number:
            {
                fprintf(out, "NUMBER: %.*s\n", token.text_length, token.text_pos);
            }break;
            
            case TokenType_Character:
            {
                fprintf(out, "CHAR: %c\n", token.character);
            }break;
            
            case TokenType_LShift:
            {
                fprintf(out, "<<\n");
            }break;
            
            case TokenType_RShift:
            {
                fprintf(out, ">>\n");
            }break;
            
            case TokenType_EqEq:
            {
                fprintf(out, "==\n");
            }break;
            
            case TokenType_NotEq:
            {
                fprintf(out, "!=\n");
            }break;
            
            case TokenType_LessEq:
            {
                fprintf(out, "<=\n");
            }break;
            
            case TokenType_GrtrEq:
            {
                fprintf(out, ">=\n");
            }break;
            
            default:
            {
                fprintf(out, "%c\n", token.type);
            }break;
        }
    }
}

// BOTTOM

