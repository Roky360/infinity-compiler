#ifndef INFINITY_COMPILER_PARSER_H
#define INFINITY_COMPILER_PARSER_H

#include "../lexer/lexer.h"
#include "../ast/ast.h"

typedef struct Parser {
    Lexer *lexer;
    Token *token;
} Parser;

Parser *init_parser(Lexer *lexer);

void parser_dispose(Parser *parser);

void parser_handle_unexpected_token(Parser *parser, char *expectations);

/// Proceeds to the next token, while checking
/// that the current token is of the same type as the `type` parameter.
/// \param parser
/// \param type
/// \return The current token. `parser->token` gets the next token.
Token *parser_forward(Parser *parser, TokenType type);

/// Moving forward with a list of expected tokens.
/// The `expectations` parameter will be displayed as error message
/// in case the current token doesn't satisfy any of the tokens in the `types` list.
/// \param parser
/// \param types Expected token types
/// \param types_len The length of the `types` list
/// \param expectations What the parser was expecting. Will be added to an error message
/// that will be printed in case the next token doesn't meet the expectations.
/// \return The previous token (before moving forward).
Token *parser_forward_with_list(Parser *parser, TokenType *types, size_t types_len, char *expectations);

/// Main function to parse a source code string.
/// \param parser
/// \return The root node of the generated AST tree.
AstNode *parser_parse(Parser *parser);

AstNode *parser_parse_expression(Parser *parser, Expression *expression);

LiteralValue *get_default_literal_value(TokenType type);

void parser_get_tokens_until(Parser *parser, List *tokens, TokenType terminator);

Token *parser_get_tokens_until_list(Parser *parser, List *tokens, TokenType *terminators, int term_len);

void parser_get_parenthesized_expression(Parser *parser, List *tokens);

AstNode *parser_parse_compound(Parser *parser);

/// Parses a block (between curly braces) and populates the `block` list with AstNode(s)
/// \param parser
/// \param block An empty list to hold the nodes in the block
void parser_parse_block(Parser *parser, List *block);

AstNode *parser_parse_statement(Parser *parser);

AstNode *parser_parse_id(Parser *parser);

AstNode *parser_parse_start_expression(Parser *parser);

AstNode *parser_parse_var_declaration(Parser *parser);

AstNode *parser_parse_function_definition(Parser *parser);

AstNode *parser_parse_assignment(Parser *parser, Token *id_token);

AstNode *parser_parse_if_statement(Parser *parser);

AstNode *parser_parse_loop(Parser *parser);

AstNode *parser_parse_return_statement(Parser *parser);

AstNode *parser_parse_function_call(Parser *parser, Token *id_token);

AstNode *parser_parse_while_loop(Parser *parser);

AstNode *parser_parse_swap_statement(Parser *parser);

#endif //INFINITY_COMPILER_PARSER_H
