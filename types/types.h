#ifndef INFINITY_COMPILER_TYPES_H
#define INFINITY_COMPILER_TYPES_H

#include "../list/list.h"
#include "../token/token.h"

typedef enum DataType {
    TYPE_VOID = VOID_KEYWORD,
    TYPE_INT = INT_KEYWORD,
    TYPE_CHAR = CHAR_KEYWORD,
    TYPE_STRING = STRING_KEYWORD,
    TYPE_DOUBLE,
    TYPE_BOOL = BOOL_KEYWORD,
} DataType;

typedef union Value {
    int integer_value;
    char char_value;
    char *string_value;
//    int bool_value;
    double double_value;
    void *void_value; // if void - initialize this to NULL
} Value;

typedef struct LiteralValue {
    DataType type;
    Value value;
} LiteralValue;

/*
An `Expression` represents an expression with or without variables.
Like: 5+7 or x*2-3
*/
typedef struct Expression {
    List *tokens;
    LiteralValue *value;
    int contains_variables; // contains variables, like: 2 * x + 3
    // without variables: 5 - 8 / 4
} Expression;

LiteralValue *init_literal_value(DataType type, Value value);

void literal_value_dispose(LiteralValue *value);

Expression *init_expression_p();

Expression init_expression();

void expression_dispose(Expression *expr);

//DataType token_type_to_data_type(TokenType type);

char *data_type_to_str(DataType type);

#endif //INFINITY_COMPILER_TYPES_H
