#ifndef INFINITY_COMPILER_PARSER_H
#define INFINITY_COMPILER_PARSER_H

#include "../lexer/lexer.h"
#include "../ast/ast.h"

typedef struct ParserStruct {
    Lexer *lexer;
    Token *token;
} Parser;

Parser *init_parser(Lexer *lexer);

void parser_dispose(Parser *parser);

void parser_handle_unexpected_token(Parser *parser, char *expectations);

Token *parser_forward(Parser *parser, TokenType type);

Token *parser_forward_with_list(Parser *parser, TokenType *types, size_t types_len, char *expectations);

AstNode *parser_parse(Parser *parser);

AstNode *parser_parse_expression(Parser *parser, Expression *expression);

LiteralValue *get_default_literal_value(TokenType type);

void parser_get_tokens_until(Parser *parser, List *tokens, TokenType terminator);

void parser_get_parenthesized_expression(Parser *parser, List *tokens);

AstNode *parser_parse_compound(Parser *parser);

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

#endif //INFINITY_COMPILER_PARSER_H
