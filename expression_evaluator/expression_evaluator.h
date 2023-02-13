#ifndef INFINITY_COMPILER_EXPRESSION_EVALUATOR_H
#define INFINITY_COMPILER_EXPRESSION_EVALUATOR_H

#include "../token/token.h"
#include "../list/list.h"
#include "../lexer/lexer.h"

/* Operators definitions */
#define OP_ADD "+"
#define OP_SUB "-"
#define OP_MUL "*"
#define OP_DIV "/"
#define OP_POW "^"
#define OP_MOD "%"
#define OP_FACT "!"
#define OP_LOGICAL_AND "and"
#define OP_LOGICAL_OR "or"
#define OP_NOT "not"
#define OP_EQUALITY "=="
#define OP_NOT_EQUAL "!="
#define OP_GRATER_THAN ">"
#define OP_GRATER_EQUAL ">="
#define OP_LOWER_THAN "<"
#define OP_LOWER_EQUAL "<="

#define NOT_OP_PLACEHOLDER "$n"
#define FACT_OP_PLACEHOLDER "$f"

typedef enum {
    VAR,
    NUMBER,
    OPERATOR,
    PAREN,
    PLACEHOLDER,
} ArithmeticTokenType;

typedef union {
    char *var;
    double number;
    char *op;
    char paren;
} ArithmeticTokenValue;

typedef struct {
    ArithmeticTokenType type;
    ArithmeticTokenValue value;
    Token *original_tok;
} ArithmeticToken;

char *print_ar_token(const void *item);

ArithmeticToken *init_empty_arithmetic_token();

ArithmeticToken *init_arithmetic_token_with(ArithmeticTokenType type, ArithmeticTokenValue value, Token *original_tok);

int is_operator(char *op);

int is_parentheses(TokenType type);

int get_precedence(char *op);

int is_right_associative(char *op);

int parse_tokens(List *expression, List *tokens, Lexer *lexer);

void infix_to_postfix(List *infix, List *postfix, Lexer *lexer);

double evaluate_postfix(List *postfix, Lexer *lexer);

int evaluate_expression(List **expression, double *res, Lexer *lexer);

#endif //INFINITY_COMPILER_EXPRESSION_EVALUATOR_H
