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
    expr->contains_variables = 0;
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

int is_valid_data_type(int type) {
    return type == TYPE_VOID
           || type == TYPE_INT
           || type == TYPE_CHAR
           || type == TYPE_STRING
           || type == TYPE_DOUBLE;
}

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
