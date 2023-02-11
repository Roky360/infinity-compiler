#ifndef INFINITY_COMPILER_TOKEN_H
#define INFINITY_COMPILER_TOKEN_H

typedef enum TokenType {
    /** Values */
    ID,
    INT,    // integer literal
    CHAR,   // character literal
    STRING, // string literal
    DOUBLE, // floating point number literal
//    BOOL,
    /** Keywords */
    START_KEYWORD,
    VOID_KEYWORD,
    INT_KEYWORD,
    BOOL_KEYWORD,
    CHAR_KEYWORD,
    STRING_KEYWORD,
    RETURN_KEYWORD,
    FUNC_KEYWORD,
    IF_KEYWORD,
    ELSE_KEYWORD,
    TRUE_KEYWORD,
    FALSE_KEYWORD,
    SWAP_KEYWORD,
    // loop-related
    LOOP_KEYWORD,
    TO_KEYWORD,
    TIMES_KEYWORD,
    WHILE_KEYWORD,

    AND_OPERATOR_KEYWORD,
    OR_OPERATOR_KEYWORD,
    NOT_OPERATOR_KEYWORD,
    /** Parentheses */
    L_PARENTHESES,     // (
    R_PARENTHESES,     // )
    L_CURLY_BRACE,    // {
    R_CURLY_BRACE,    // }
    L_SQUARE_BRACKET, // [
    R_SQUARE_BRACKET, // [
    /** Punctuation */
    SEMICOLON,
    COMMA,
    COLON,
    ASSIGNMENT, // =
    EQUALS,     // ==
    NOT_EQUAL,
    GRATER_THAN,
    LOWER_THAN,
    GRATER_EQUAL,
    LOWER_EQUAL,
    /** Operators */
    ADD_OP,
    SUB_OP,
    MUL_OP,
    DIVIDE_OP,
    MODULUS_OP,
    POWER_OP,
    FACTORIAL_OP,
    ARROW, // ->
    THICK_ARROW, // =>
    DEC,
    INC,

    EOF_TOKEN,
} TokenType;

typedef struct Token {
    TokenType type;
    char *value;

    unsigned int line;
    unsigned int column;
    int length;
} Token;

Token *init_token(char *value, TokenType type, unsigned int line, unsigned int column, int length);

void token_dispose(Token *token);

char *token_type_to_str(TokenType type);

#endif //INFINITY_COMPILER_TOKEN_H
