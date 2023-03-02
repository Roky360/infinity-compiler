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

/// Initializes an empty Arithmetic Token.
/// \return
ArithmeticToken *init_empty_arithmetic_token();

/// Initializes an Arithmetic Token with values.
/// \param type Token type
/// \param value
/// \param original_tok
/// \return
ArithmeticToken *init_arithmetic_token_with(ArithmeticTokenType type, ArithmeticTokenValue value, Token *original_tok);

int is_operator(char *op);

int is_parentheses(TokenType type);

/// Returns the precedence of an operator in the language.
/// \param op Operator to check.
/// \return The precedence as in integer
int get_precedence(char *op);

/// Whether an operator is right-associative
/// \param op
/// \return Boolean
int is_right_associative(char *op);

/// Parses a list of Token(s) into a list of ArithmeticToken(s)
/// \param expression Input list - list of Token representing an expression
/// \param tokens Output list - list of ArithmeticToken
/// \param lexer For error reporting
/// \return If the expression can be parsed to a constant value (1), or it contains variables (0)
int parse_tokens(List *expression, List *tokens, Lexer *lexer);

/// Converts an expression from infix notation to postfix notation.
/// \param infix List of ArithmeticToken (input)
/// \param postfix Empty list that will contain the postfix notation of the expression (output)
/// \param lexer For error reporting
void infix_to_postfix(List *infix, List *postfix, Lexer *lexer);

/// Evaluates an expression in postfix notation. The expression needs to have a constant value (without variables)
/// \param postfix Expression in postfix
/// \param lexer For error reporting
/// \return The result of the expression, as double.
double evaluate_postfix(List *postfix, Lexer *lexer);

/// Main function to evaluate an expression.
/// \param expression Pointer to a list of Token(s)
/// \param res Pointer to a double, where the result will be
/// \param lexer For error reporting
/// \return If the expression can be evaluated, or it contains variables. \n
/// * If it can be evaluated - the result is stored in `res` and returns true
/// * If not - `expression` will contain the postfix expression and false is returned
int evaluate_expression(List **expression, double *res, Lexer *lexer);

#endif //INFINITY_COMPILER_EXPRESSION_EVALUATOR_H
