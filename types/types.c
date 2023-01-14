#include "types.h"
#include "../token/token.h"
#include <stdio.h>
#include <stdlib.h>

LiteralValue *init_literal_value(DataType type, Value value) {
    LiteralValue *literal = malloc(sizeof(LiteralValue));
    if (!literal) {
        printf("Cant allocate LiteralValue\n");
        exit(1);
    }
    literal->type = type;
    literal->value = value;
    return literal;
}

void literal_value_dispose(LiteralValue *value) {
    free(value);
}

Expression *init_expression_p() {
    Expression *expr = malloc(sizeof(Expression));
    expr->tokens = init_list(sizeof(Token *));
    return expr;
}

Expression init_expression() {
    return (Expression) {
            .tokens = init_list(sizeof(Token *)),
    };
}

void expression_dispose(Expression *expr) {
    list_dispose(expr->tokens);
    free(expr);
}

//DataType token_type_to_data_type(TokenType type) {
//    switch (type) {
//        case INT_KEYWORD:
//            return TYPE_INT;
//        case STRING_KEYWORD:
//            return TYPE_STRING;
////        case BOOL_KEYWORD:
////            return TYPE_BOOL;
//        case CHAR_KEYWORD:
//            return TYPE_CHAR;
//        default: // invalid type
//            return -1;
//    }
//}

char *data_type_to_str(DataType type) {
    switch (type) {
        case TYPE_VOID:
            return "void";
        case TYPE_INT:
            return "int";
        case TYPE_CHAR:
            return "char";
        case TYPE_STRING:
            return "string";
        case TYPE_DOUBLE:
            return "double";
        case TYPE_BOOL:
            return "bool";
    }
}
