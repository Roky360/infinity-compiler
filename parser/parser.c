#include "parser.h"
#include "../config/globals.h"
#include "../logging/logging.h"
#include "../io/io.h"
#include "../expression_evaluator/expression_evaluator.h"
#include "../config/table_initializers.h"
#include <string.h>

Parser *init_parser(Lexer *lexer) {
    Parser *parser = malloc(sizeof(Parser));
    if (!parser)
        throw_memory_allocation_error(PARSER);

    parser->lexer = lexer;
    parser->token = lexer_next_token(lexer);
    return parser;
}

void parser_dispose(Parser *parser) {
    lexer_dispose(parser->lexer);
    token_dispose(parser->token);
    free(parser);
}

void parser_handle_unexpected_token(Parser *parser, char *expectations) {
//    char *errMsg;
//    alsprintf(&errMsg, "Unexpected token: '%s'. Expecting %s", parser->token->value, expectations);
//    log_exception_with_trace(PARSER, parser->lexer, errMsg);
//    log_exception_with_trace(PARSER, parser->lexer, parser->lexer->row, col, tok_len,
//                             "Unexpected token: \"%s\". Expecting %s", parser->token->value, expectations);
    log_exception_with_trace(PARSER, parser->lexer, parser->token->line, parser->token->column,
                             parser->token->length,
                             "Unexpected token \"%s\". Expecting %s", parser->token->value,
                             expectations);
}

/*
Proceeds to the next token, while checking
that the current token is of the same type as the `type` parameter.
Returns the current token. parser->token gets the next token.
*/
Token *parser_forward(Parser *parser, TokenType type) {
    Token *prev_tok;

    if (parser->token->type != type) {
        parser_handle_unexpected_token(parser, token_type_to_str(type));
    }
    prev_tok = parser->token;
    parser->token = lexer_next_token(parser->lexer);

    return prev_tok;
}

/**
 * Moving forward with a list of expected tokens.
 * The `expectations` parameter will be displayed as error message
 * in case the current token doesn't satisfy any of the tokens in the `types` list.
 */
Token *parser_forward_with_list(Parser *parser, TokenType *types, size_t types_len, char *expectations) {
    int i;
    for (i = 0; i < types_len; i++) {
        if (parser->token->type == types[i])
            return parser_forward(parser, types[i]);
    }
    log_exception_with_trace(PARSER, parser->lexer, parser->token->line, parser->token->column,
                             parser->token->length,
                             "Unexpected token \"%s\". Expecting %s", parser->token->value,
                             expectations);
    return NULL;
}

AstNode *parser_parse(Parser *parser) {
    return parser_parse_compound(parser);
}

AstNode *parser_parse_expression(Parser *parser, Expression *expression) {
    double res;
    AstNode *expr_node = init_ast(AST_EXPRESSION);

    if (list_is_empty(expression->tokens)) {
        // void expression
        expr_node->data.expression.value = init_literal_value(
                TYPE_VOID,
                (Value) {.void_value = NULL}
        );
    } else if (((Token *) expression->tokens->items[0])->type == STRING) {
        // string
        expr_node->data.expression.value = init_literal_value(
                TYPE_STRING,
                (Value) {.string_value = ((Token *) expression->tokens->items[0])->value}
        );
        if (expression->tokens->size > 1) {
            Token *err_token = (Token *) expression->tokens->items[1];
            log_exception_with_trace(PARSER, parser->lexer, err_token->line, err_token->column,
                                     err_token->length,
                                     "Expected end of expression after string value");
        }
    } else {
        // parse expression
        if (evaluate_expression(&expression->tokens, &res, parser->lexer)) {
            expr_node->data.expression.value = init_literal_value(TYPE_DOUBLE, (Value) {.double_value = res});
        } else {
            expr_node->data.expression.contains_variables = 1;
            expr_node->data.expression.value = init_literal_value(TYPE_DOUBLE, (Value) {});
        }
    }
    expr_node->data.expression.tokens = expression->tokens;

    return expr_node;
}

LiteralValue *get_default_literal_value(TokenType type) {
    switch (type) {
        case INT_KEYWORD:
            return init_literal_value(TYPE_INT, (Value) {.integer_value = 0});
        case STRING_KEYWORD:
            return init_literal_value(TYPE_STRING, (Value) {.string_value = ""});
        case CHAR_KEYWORD:
            return init_literal_value(TYPE_INT, (Value) {.char_value = '\0'});
//        case BOOL_KEYWORD:
//            return init_literal_value(TYPE_BOOL, (Value) {.bool_value = 0});
        default:
            log_error(PARSER, "Unsupported type '%s'", token_type_to_str(type));
            return NULL;
    }
}

void parser_get_tokens_until(Parser *parser, List *tokens, TokenType terminator) {
    while (parser->token->type != terminator) {
        list_push(tokens, parser_forward(parser, parser->token->type));
    }
    parser_forward(parser, terminator);
}

Token *parser_get_tokens_until_list(Parser *parser, List *tokens, TokenType *terminators, int term_len) {
    int done = 0, paren_count = 0, i;
    Token *start_tok = parser->token, *curr_tok;
    char expected_tokens_buf[50] = {0}, *tok_buf;
    while (!done) {
        for (i = 0; i < term_len; i++) {
            if (parser->token->type == terminators[i] && paren_count == 0) {
                done = 1;
                break;
            }
        }
        // end of file has reached before end of statement
        if (parser->token->type == EOF_TOKEN) {
            for (i = 0; i < term_len; i++) { // prepare list of expected tokens
                alsprintf(&tok_buf, "%s%s", token_type_to_str(terminators[i]), i == term_len - 1 ? "" : ", ");
                strcat(expected_tokens_buf, tok_buf);
                free(tok_buf);
            }
            log_exception_with_trace(PARSER, parser->lexer, start_tok->line, start_tok->column, start_tok->length,
                                     "End of file has reached unexpectedly. Expected one of those tokens: %s.",
                                     expected_tokens_buf);
        }
        if (!done) {
            curr_tok = parser_forward(parser, parser->token->type);
            if (curr_tok->type == L_PARENTHESES)
                paren_count++;
            else if (curr_tok->type == R_PARENTHESES)
                paren_count--;
            list_push(tokens, curr_tok);
        }
    }
    return parser_forward(parser, parser->token->type);
}

/* like parser_get_tokens_until, for expressions inside parentheses.
 * makes sure that parentheses are closing correctly
 * */
void parser_get_parenthesized_expression(Parser *parser, List *tokens) {
    int paren_count = 1;
    parser_forward(parser, L_PARENTHESES);
    while (paren_count > 0) {
        if (parser->token->type == L_PARENTHESES)
            paren_count++;
        else if (parser->token->type == R_PARENTHESES)
            paren_count--;
        if (paren_count > 0)
            list_push(tokens, parser_forward(parser, parser->token->type));
    }
    parser_forward(parser, R_PARENTHESES);
}

AstNode *parser_parse_compound(Parser *parser) {
    AstNode *root = init_ast(AST_COMPOUND);

    while (parser->token->type != EOF_TOKEN) {
        list_push(root->data.compound.children, parser_parse_statement(parser));
    }
    return root;
}

void parser_parse_block(Parser *parser, List *block) {
    unsigned int row = parser->token->line,
            col = parser->token->column, tok_len = parser->token->length;
    parser_forward(parser, L_CURLY_BRACE);
    while (parser->token->type != R_CURLY_BRACE) {
        if (parser->token->type == EOF_TOKEN) {
            log_exception_with_trace(PARSER, parser->lexer, row, col, (int) tok_len,
                                     "Expected '}' to close this block.");
        }
        list_push(block, parser_parse_statement(parser));
    }
    parser_forward(parser, R_CURLY_BRACE);
}

AstNode *parser_parse_statement(Parser *parser) {
    char *id_ptr;
    AstNode *(*parser_func)(Parser *);

    parser_func = hash_table_lookup(statement_to_parser_map, alsprintf(&id_ptr, "%d", parser->token->type));
    free(id_ptr);
    if (parser_func) {
        // call the right parsing function
        return parser_func(parser);
    } else if (parser->token->type == SEMICOLON) {
        log_warning_with_trace(PARSER, parser->lexer, parser->token->line, parser->token->column,
                               parser->token->length,
                               "Consider removing empty statements");
        parser_forward(parser, SEMICOLON);
        return parser_parse_statement(parser);
    } else {
        // print error message
        log_exception_with_trace(PARSER, parser->lexer, parser->token->line, parser->token->column,
                                 parser->token->length,
                                 "Expected an expression, got %s", token_type_to_str(parser->token->type));
        return NULL;
    }
}

AstNode *parser_parse_id(Parser *parser) {
    Token *id_token = parser_forward(parser, ID);
    switch (parser->token->type) {
        case ASSIGNMENT:
            return parser_parse_assignment(parser, id_token);
        case L_PARENTHESES:
            return parser_parse_function_call(parser, id_token);
        case SEMICOLON:
            log_warning_with_trace(PARSER, parser->lexer, parser->token->line, parser->token->column,
                                   parser->token->length,
                                   "Expression has no affect. Consider removing this expression.");
            parser_forward(parser, SEMICOLON);
            return init_ast(AST_NOOP);
        default:
            parser_handle_unexpected_token(parser, "an expression");
            return NULL;
    }
}

AstNode *parser_parse_start_expression(Parser *parser) {
    AstNode *node = init_ast(AST_START_EXPRESSION);
    parser_forward(parser, START_KEYWORD);
    node->data.start_expr.starting_point = parser_forward(parser, ID)->value;
    parser_forward(parser, SEMICOLON);
    return node;
}

AstNode *parser_parse_var_declaration(Parser *parser) {
    AstNode *node, *value_expr;
    Token *var_type;
    Expression *expr;

    node = init_ast(AST_VARIABLE_DECLARATION);
    var_type = parser_forward_with_list(parser, data_types, data_types_len, "type definition");
    if (var_type->type == DOUBLE) { // doubles not supported
        log_exception_with_trace(PARSER, parser->lexer, parser->token->line, parser->token->column,
                                 parser->token->length, "Variables of type double are not supported yet");
    }
    node->data.variable_declaration.var = init_variable(
            parser_forward(parser, ID)->value,
            init_literal_value((DataType) var_type->type, (Value) {})
    );

    // if value_expr is immediately assigned to variable
    if (parser->token->type == ASSIGNMENT) {
        expr = init_expression_p();
        parser_forward(parser, ASSIGNMENT);

        parser_get_tokens_until(parser, expr->tokens, SEMICOLON);
        node->data.variable_declaration.value = parser_parse_expression(parser, expr);
        if (node->data.variable_declaration.value->data.expression.value->type == TYPE_VOID) {
            log_exception_with_trace(PARSER, parser->lexer, parser->token->line, parser->token->column,
                                     parser->token->length, "Expected an expression");
        }
    } else {
        // variable is initialized with default value_expr
        value_expr = init_ast(AST_EXPRESSION);
        parser_forward(parser, SEMICOLON);

        value_expr->data.expression.value = get_default_literal_value(var_type->type);
        value_expr->data.expression.contains_variables = 0;

        node->data.variable_declaration.value = value_expr;
    }

    return node;
}

AstNode *parser_parse_function_definition(Parser *parser) {
    Variable *arg;
    DataType arg_type;
    Expression *return_expr; // for one-line functions
    AstNode *return_node, *node = init_ast(AST_FUNCTION_DEFINITION);

    parser_forward(parser, FUNC_KEYWORD);

    // define function name
    node->data.function_definition.func_name = parser_forward(parser, ID)->value;

    // get arguments
    parser_forward(parser, L_PARENTHESES);
    while (parser->token->type != R_PARENTHESES) {
        // get arg type
        arg_type = (DataType) parser->token->type;
        if (!is_valid_data_type(parser->token->type)) // invalid type
        {
            parser_handle_unexpected_token(parser, "argument type");
        }
        parser_forward(parser, parser->token->type);
        // get arg name
        arg = init_variable(
                parser_forward(parser, ID)->value,
                init_literal_value(arg_type, (Value) {})
        );
        list_push(node->data.function_definition.args, arg);

        if (parser->token->type != R_PARENTHESES)
            parser_forward(parser, COMMA);
    }
    parser_forward(parser, R_PARENTHESES);

    // get return type
    if (parser->token->type == ARROW) {
        parser_forward(parser, ARROW);
        node->data.function_definition.returnType =
                (DataType) parser_forward_with_list(parser, data_types, data_types_len, "return type")->type;
        if (node->data.function_definition.returnType == TYPE_DOUBLE) {
            log_exception_with_trace(PARSER, parser->lexer, parser->token->line, parser->token->column,
                                     parser->token->length, "Doubles are not supported yet");
        }
    } else {
        // return type not provided - default is void
        node->data.function_definition.returnType = TYPE_VOID;
    }

    // one-line function
    if (parser->token->type == THICK_ARROW) {
        parser_forward(parser, THICK_ARROW);
        return_node = init_ast(AST_RETURN_STATEMENT);
        return_expr = init_expression_p();

        parser_get_tokens_until(parser, return_expr->tokens, SEMICOLON);
        return_node->data.return_statement.value_expr = parser_parse_expression(parser, return_expr);
        list_push(node->data.function_definition.body, return_node);
    } else {
        // parse function body
        parser_parse_block(parser, node->data.function_definition.body);
    }

    return node;
}

AstNode *parser_parse_assignment(Parser *parser, Token *id_token) {
    AstNode *node;
    Expression *expr;
    node = init_ast(AST_ASSIGNMENT);
    expr = init_expression_p();

    parser_forward(parser, ASSIGNMENT);
    node->data.assignment.dst_variable = id_token;
    parser_get_tokens_until(parser, expr->tokens, SEMICOLON);
    node->data.assignment.expression = parser_parse_expression(parser, expr);

    return node;
}

AstNode *parser_parse_if_statement(Parser *parser) {
    AstNode *node, *condition_node;
    List *condition;
    node = init_ast(AST_IF_STATEMENT);
    condition = init_list(sizeof(Token *));

    parser_forward(parser, IF_KEYWORD);
    // parse boolean expression
    parser_get_parenthesized_expression(parser, condition);
    if (condition->size == 0) {
        log_exception_with_trace(PARSER, parser->lexer, parser->token->line, parser->token->column, 0,
                                 "Expected an expression in if condition");
    }
    condition_node = node->data.if_statement.condition = init_ast(AST_EXPRESSION);
    node->data.if_statement.condition->data.expression.tokens = condition;
    // TODO: change the type to int   ->                                                   \/
    node->data.if_statement.condition->data.expression.value = init_literal_value(TYPE_DOUBLE, (Value) {});
    if (evaluate_expression(&condition_node->data.expression.tokens,
                            &condition_node->data.expression.value->value.double_value, parser->lexer)) {
        // TODO: move this to the analyzer
        Token *tok = ((ArithmeticToken *) condition_node->data.expression.tokens->items[0])->original_tok
                     ? ((ArithmeticToken *) condition_node->data.expression.tokens->items[0])->original_tok
                     : ((ArithmeticToken *) condition_node->data.expression.tokens->items[1])->original_tok;
        log_warning_with_trace(PARSER, parser->lexer, tok->line, tok->column, tok->length,
                               "This if statement is always %s",
                               node->data.if_statement.condition->data.expression.value->value.double_value ? "true"
                                                                                                            : "false");
    } else {
        condition_node->data.expression.contains_variables = 1;
        condition_node->data.expression.value = init_literal_value(TYPE_DOUBLE, (Value) {});
    }

    // parse the body
    parser_parse_block(parser, node->data.if_statement.body_node);
    // check for else statement or `else if ...` statement
    if (parser->token->type == ELSE_KEYWORD) {
        parser_forward(parser, ELSE_KEYWORD);
        if (parser->token->type == IF_KEYWORD)
            list_push(node->data.if_statement.else_node, parser_parse_if_statement(parser));
        else
            parser_parse_block(parser, node->data.if_statement.else_node);
    }

    return node;
}

AstNode *parser_parse_loop(Parser *parser) {
    Expression *start;
    Token *prev, *counter_tok;
    AstNode *node = init_ast(AST_LOOP);
    node->data.loop.start->tokens = init_list(sizeof(Token *));
    start = init_expression_p();
    start->value = init_literal_value(TYPE_DOUBLE, (Value) {});

    parser_forward(parser, LOOP_KEYWORD);
    prev = parser_get_tokens_until_list(parser, start->tokens, (TokenType[]) {COLON, TIMES_KEYWORD}, 2);
    if (list_is_empty(start->tokens))
        log_exception_with_trace(PARSER, parser->lexer, parser->token->line, parser->token->column, 0,
                                 "Expected an expression.");
    if (prev->type == COLON) { // with counter
        // pick loop counter name
        counter_tok = (Token *) start->tokens->items[0];
        if (counter_tok->type != ID || start->tokens->size != 1)
            log_exception_with_trace(PARSER, parser->lexer, counter_tok->line, counter_tok->column, counter_tok->length,
                                     "Expected loop counter name.");
        node->data.loop.loop_counter_name = counter_tok->value;
        node->data.loop.loop_counter_col = counter_tok->column;

        list_clear(start->tokens, 1);
        prev = parser_get_tokens_until_list(parser, start->tokens, (TokenType[]) {TO_KEYWORD, TIMES_KEYWORD}, 2);
        if (list_is_empty(start->tokens))
            log_exception_with_trace(PARSER, parser->lexer, parser->token->line, parser->token->column, 0,
                                     "Expected an expression.");
        if (!evaluate_expression(&start->tokens, &start->value->value.double_value, parser->lexer)) {
            start->contains_variables = 1;
        }
        if (prev->type == TIMES_KEYWORD) { // i: ▨ times
            node->data.loop.end = start;
        } else { // i: _ to ▨ times
            node->data.loop.start = start;
            parser_get_tokens_until(parser, node->data.loop.end->tokens, TIMES_KEYWORD);
            if (list_is_empty(node->data.loop.end->tokens))
                log_exception_with_trace(PARSER, parser->lexer, parser->token->line, parser->token->column, 0,
                                         "Expected an expression");
            if (!evaluate_expression(&node->data.loop.end->tokens, &node->data.loop.end->value->value.double_value,
                                     parser->lexer)) {
                node->data.loop.end->contains_variables = 1;
            }
        }
    } else { // without counter: loop ▨ times
        node->data.loop.end = start;
        if (!evaluate_expression(&node->data.loop.end->tokens, &node->data.loop.end->value->value.double_value,
                                 parser->lexer)) {
            node->data.loop.end->contains_variables = 1;
        }
    }

    // parse loop body
    parser_parse_block(parser, node->data.loop.body);

    return node;
}

AstNode *parser_parse_return_statement(Parser *parser) {
    AstNode *node = init_ast(AST_RETURN_STATEMENT);
    Expression *expr = init_expression_p();

    parser_forward(parser, RETURN_KEYWORD);

    parser_get_tokens_until(parser, expr->tokens, SEMICOLON);
    node->data.return_statement.value_expr = parser_parse_expression(parser, expr);

    return node;
}

AstNode *parser_parse_function_call(Parser *parser, Token *id_token) {
    Token *prev_tok;
    Expression *arg_expr;
    AstNode *node = init_ast(AST_FUNCTION_CALL);
    node->data.function_call.func_name = id_token->value;

    parser_forward(parser, L_PARENTHESES);
    prev_tok = parser->token;
    if (prev_tok->type == R_PARENTHESES)
        parser_forward(parser, R_PARENTHESES);
    while (prev_tok->type != R_PARENTHESES) {
        arg_expr = init_expression_p();
        prev_tok = parser_get_tokens_until_list(parser, arg_expr->tokens,
                                                (TokenType[]) {COMMA, R_PARENTHESES}, 2);
        list_push(node->data.function_call.args, parser_parse_expression(parser, arg_expr));
    }
    parser_forward(parser, SEMICOLON);

    return node;
}

AstNode *parser_parse_while_loop(Parser *parser) {
    AstNode *node, *condition_node;
    List *condition;
    node = init_ast(AST_WHILE_LOOP);
    condition = init_list(sizeof(Token *));

    parser_forward(parser, WHILE_KEYWORD);
    // parse boolean expression
    parser_get_parenthesized_expression(parser, condition);
    if (condition->size == 0) {
        log_exception_with_trace(PARSER, parser->lexer, parser->token->line, parser->token->column, 0,
                                 "Expected an expression in while condition");
    }
    condition_node = node->data.while_loop.condition = init_ast(AST_EXPRESSION);
    node->data.while_loop.condition->data.expression.tokens = condition;
    // TODO: change the type to int   ->                                                   \/
    node->data.while_loop.condition->data.expression.value = init_literal_value(TYPE_DOUBLE, (Value) {});
    if (evaluate_expression(&condition_node->data.expression.tokens,
                            &condition_node->data.expression.value->value.double_value, parser->lexer)) {
        // TODO: move this to the analyzer
        Token *tok = ((ArithmeticToken *) condition_node->data.expression.tokens->items[0])->original_tok
                     ? ((ArithmeticToken *) condition_node->data.expression.tokens->items[0])->original_tok
                     : ((ArithmeticToken *) condition_node->data.expression.tokens->items[1])->original_tok;
        if ((int) node->data.while_loop.condition->data.expression.value->value.double_value) {
            log_warning_with_trace(PARSER, parser->lexer, tok->line, tok->column, tok->length,
                                   "Infinite loop - condition in while loop is always true");
        } else {
            log_warning_with_trace(PARSER, parser->lexer, tok->line, tok->column, tok->length,
                                   "While loop condition is always false");
        }
    } else {
        condition_node->data.expression.contains_variables = 1;
        condition_node->data.expression.value = init_literal_value(TYPE_DOUBLE, (Value) {});
    }

    parser_parse_block(parser, node->data.while_loop.body);

    return node;
}

AstNode *parser_parse_swap_statement(Parser *parser) {
    AstNode *node = init_ast(AST_SWAP_STATEMENT);
    parser_forward(parser, SWAP_KEYWORD);
    node->data.swap_statement.var_a = parser_forward(parser, ID);
    parser_forward(parser, COMMA);
    node->data.swap_statement.var_b = parser_forward(parser, ID);
    parser_forward(parser, SEMICOLON);
    return node;
}
