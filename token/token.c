#include "token.h"
#include <stdio.h>
#include <stdlib.h>

Token *init_token(char *value, TokenType type, unsigned int line, unsigned int column, int length) {
    Token *token = malloc(sizeof(Token));
    if (!token) {
        printf("Cant allocate memory for token\n");
        exit(1);
    }
    token->value = value;
    token->type = type;
    token->line = line;
    token->column = column;
    token->length = length;

    return token;
}

void token_dispose(Token *token) {
    free(token->value);
    free(token);
}

char *token_type_to_str(TokenType type) {
    switch (type) {
        case START_KEYWORD:
            return "<START_KEYWORD>";
        case ID:
            return "<ID>";
        case INT:
            return "<INT>";
        case STRING:
            return "<STRING>";
        case DOUBLE:
            return "<DOUBLE>";
        case CHAR:
            return "<CHAR>";
//        case BOOL:
//            return "<BOOL>";
        case VOID_KEYWORD:
            return "<VOID_KEYWORD>";
        case INT_KEYWORD:
            return "<INT_KEYWORD>";
        case STRING_KEYWORD:
            return "<STRING_KEYWORD>";
        case BOOL_KEYWORD:
            return "<BOOL_KEYWORD>";
        case CHAR_KEYWORD:
            return "<CHAR_KEYWORD>";
        case IF_KEYWORD:
            return "<IF_KEYWORD>";
        case ELSE_KEYWORD:
            return "<ELSE_KEYWORD>";
        case LOOP_KEYWORD:
            return "<LOOP_KEYWORD>";
        case TO_KEYWORD:
            return "<TO_KEYWORD>";
        case TIMES_KEYWORD:
            return "<TIMES_KEYWORD>";
        case L_PARENTHESES:
            return "<LEFT_PARENTHESES>";
        case R_PARENTHESES:
            return "<RIGHT_PARENTHESES>";
        case L_CURLY_BRACE:
            return "<LEFT_CURLY_BRACE>";
        case R_CURLY_BRACE:
            return "<RIGHT_CURLY_BRACE>";
        case RETURN_KEYWORD:
            return "<RETURN_KEYWORD>";
        case FUNC_KEYWORD:
            return "<FUNC_KEYWORD>";
        case SEMICOLON:
            return "<SEMICOLON>";
        case COMMA:
            return "<COMMA>";
        case COLON:
            return "<COLON>";
        case ASSIGNMENT:
            return "<ASSIGNMENT>";
        case EQUALS:
            return "<EQUALS>";
        case GRATER_THAN:
            return "<GRATER_THAN>";
        case GRATER_EQUAL:
            return "<GRATER_EQUAL>";
        case LOWER_THAN:
            return "<LOWER_THAN>";
        case LOWER_EQUAL:
            return "<LOWER_EQUAL>";
        case ADD_OP:
            return "<ADD_OP>";
        case SUB_OP:
            return "<SUBTRACT_OP>";
        case MUL_OP:
            return "<MUL_OP>";
        case DIVIDE_OP:
            return "<DIVIDE_OP>";
        case POWER_OP:
            return "<POWER_OP>";
        case ARROW:
            return "<ARROW>";
        case DEC:
            return "<DEC>";
        case INC:
            return "<INC>";
        case EOF_TOKEN:
            return "<EOF>";
        default:
            return "<Unknown-token>";
    }
}
