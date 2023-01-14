#include "parser.h"
#include "../config/globals.h"
#include "../logging/logging.h"
#include "../io/io.h"
#include "../test_evals/expression_evaluator.h"
#include <stdio.h>

Parser *init_parser(Lexer *lexer) {
    Parser *parser = malloc(sizeof(Parser));
    if (!parser)
        log_error(PARSER, "Cant allocate memory for parser.");

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
    char *errMsg;
    alsprintf(&errMsg, "Unexpected token: '%s'. Expecting %s", parser->token->value, expectations);
    throw_exception_with_trace(PARSER, parser->lexer, errMsg);
}

/*
Proceeds to the next token, while checking
that the current token is of the same type as the `type` parameter.
Returns the current token. parser->token gets the next token.
*/
Token *parser_forward(Parser *parser, TokenType type) {
    Token *currTok;

    if (parser->token->type != type) {
        parser_handle_unexpected_token(parser, token_type_to_str(type));
    }
    currTok = parser->token;
    parser->token = lexer_next_token(parser->lexer);

    return currTok;
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
    parser_handle_unexpected_token(parser, expectations);
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
            throw_exception_with_trace(PARSER, parser->lexer, "Expected end of expression");
        }
    } else {
        // parse expression
        if (evaluate(expression->tokens, &res)) {
            expr_node->data.expression.value = init_literal_value(TYPE_DOUBLE, (Value) {.double_value = res});
        } else {
            // TODO: how to parse expressions with variables?
            fprintf(stderr, "Expressions with variables are not supported right now\n");
            exit(1);
        }
    }

    return expr_node;
}

LiteralValue *get_default_literal_value(TokenType type) {
    char *errMsg;

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
            alsprintf(&errMsg, "Unsupported type '%s'", token_type_to_str(type));
            log_error(PARSER, errMsg);
            return NULL;
    }
}

void parser_get_tokens_until(Parser *parser, List *tokens, TokenType terminator) {
    while (parser->token->type != terminator) {
        list_push(tokens, parser_forward(parser, parser->token->type));
    }
    parser_forward(parser, terminator);
}

AstNode *parser_parse_compound(Parser *parser) {
    AstNode *root = init_ast(AST_COMPOUND);

    while (parser->token->type != EOF_TOKEN) {
        list_push(root->data.compound.children, parser_parse_statement(parser));
    }
    return root;
}

void parser_parse_block(Parser *parser, List *block) {
    parser_forward(parser, L_CURLY_BRACE);
    while (parser->token->type != R_CURLY_BRACE) {
        list_push(block, parser_parse_statement(parser));
    }
    parser_forward(parser, R_CURLY_BRACE);
}

AstNode *parser_parse_statement(Parser *parser) {
    char *errMsg;
    switch (parser->token->type) {
        case START_KEYWORD:
            return parser_parse_start_expression(parser);
        case ID:
            return parser_parse_id(parser);
        case FUNC_KEYWORD:
            return parser_parse_function_definition(parser);
        case INT_KEYWORD:
        case CHAR_KEYWORD:
        case BOOL_KEYWORD:
        case STRING_KEYWORD:
            return parser_parse_var_declaration(parser);
        case IF_KEYWORD:
            return parser_parse_if_statement(parser);
        case LOOP_KEYWORD:
            return parser_parse_loop(parser);
        case RETURN_KEYWORD:
            return parser_parse_return_statement(parser);
        default:
            alsprintf(&errMsg, "Expected an expression, got %s", token_type_to_str(parser->token->type));
            throw_exception_with_trace(PARSER, parser->lexer, errMsg);
            return NULL;
    }
}

AstNode *parser_parse_id(Parser *parser) {
    Token *id_token = parser_forward(parser, ID);
    switch (parser->token->type) {
        case ASSIGNMENT:
            return parser_parse_assignment(parser, id_token);
        case L_PARENTHESES:
//            return parser_parse_function_call();
            return NULL;
        case SEMICOLON:
            log_warning(parser->lexer, "Expression has no affect. Consider removing this expression.");
            parser_forward(parser, SEMICOLON);
            return init_ast(AST_NOOP);
        default:
            throw_exception_with_trace(PARSER, parser->lexer, "Expected");
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
        throw_exception_with_trace(PARSER, parser->lexer, "Variables of type double are not supported yet");
    }
    node->data.variable_declaration.var = init_variable(
            parser_forward(parser, ID)->value,
            init_literal_value((DataType) var_type->type/*TODO:here*/, (Value) {})
    );

    // if value_expr is immediately assigned to variable
    if (parser->token->type == ASSIGNMENT) {
        expr = init_expression_p();
        parser_forward(parser, ASSIGNMENT);

        parser_get_tokens_until(parser, expr->tokens, SEMICOLON);
        node->data.variable_declaration.value = parser_parse_expression(parser, expr);
        if (node->data.variable_declaration.value->data.expression.value->type == TYPE_VOID) {
            throw_exception_with_trace(PARSER, parser->lexer, "Expected an expression");
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
    char *errMsg;
    Variable *arg;
    DataType argType;
    AstNode *node = init_ast(AST_FUNCTION_DEFINITION);

    parser_forward(parser, FUNC_KEYWORD);

    // define function name
    node->data.function_definition.func_name = parser_forward(parser, ID)->value;

    // get arguments
    parser_forward(parser, L_PARENTHESES);
    while (parser->token->type != R_PARENTHESES) {
        // get arg type
        /*TODO:here*/
//        argType = token_type_to_data_type(parser->token->type);
        argType = (DataType) parser->token->type;
        if (argType == -1) // invalid type
        {
            alsprintf(&errMsg, "Expected argument type, got %s token.", token_type_to_str(parser->token->type));
            throw_exception_with_trace(PARSER, parser->lexer, errMsg);
        }
        parser_forward(parser, parser->token->type);
        // get arg name
        arg = init_variable(
                parser_forward(parser, ID)->value,
                init_literal_value(argType, (Value) {})
        );
        list_push(node->data.function_definition.args, arg);

        if (parser->token->type != R_PARENTHESES)
            parser_forward(parser, COMMA);
    }
    parser_forward(parser, R_PARENTHESES);

    // get return type
    if (parser->token->type == ARROW) {
        parser_forward(parser, ARROW);
        /*TODO:here*/
        node->data.function_definition.returnType =
                (DataType) parser_forward_with_list(parser, data_types, data_types_len, "return type")->type;
        if (node->data.function_definition.returnType == TYPE_DOUBLE) {
            throw_exception_with_trace(PARSER, parser->lexer, "Doubles are not supported yet");
        }
    } else {
        // return type not provided - default is void
        node->data.function_definition.returnType = TYPE_VOID;
    }


    parser_parse_block(parser, node->data.function_definition.body);

//    printf("name: %s\n", node->data.function_definition.func_name);
//    printf("return type: %d\n", node->data.function_definition.returnType);
//    for (int i = 0; i < node->data.function_definition.args->size; i++) {
//        printf("var %s\n", ((Variable *) (node->data.function_definition.args->items[i]))->name);
//    }
//    AstNode *v1 = ((AstNode *) (node->data.function_definition.body->items[0]));
//    AstNode *v2 = ((AstNode *) (node->data.function_definition.body->items[1]));
//    AstNode *v3 = ((AstNode *) (node->data.function_definition.body->items[2]));

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
    char *warning;
    AstNode *node;
    List *condition;
    node = init_ast(AST_IF_STATEMENT);
    condition = init_list(sizeof(Token *));

    parser_forward(parser, IF_KEYWORD);
    // parse boolean expression
    parser_forward(parser, L_PARENTHESES);
    parser_get_tokens_until(parser, condition, R_PARENTHESES);
    node->data.if_statement.condition = init_expression_p();
    node->data.if_statement.condition->tokens = condition;
    // TODO: change the type to int   ->                                   \/
    node->data.if_statement.condition->value = init_literal_value(TYPE_DOUBLE, (Value) {});
    if (evaluate(node->data.if_statement.condition->tokens,
                 &node->data.if_statement.condition->value->value.double_value)) {
        // TODO: move this to the analyzer
        alsprintf(&warning, "This if statement is always %s",
                  node->data.if_statement.condition->value->value.double_value ? "true" : "false");
        log_warning(parser->lexer, warning);
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
    int start;
    Token *prev;
    AstNode *node = init_ast(AST_LOOP);

    parser_forward(parser, LOOP_KEYWORD);
    switch (parser->token->type) {
        // simple loop
        case INT:
            node->data.loop.end = atoi(parser_forward(parser, INT)->value);
            parser_forward(parser, TIMES_KEYWORD);
            break;
            // assign a loop counter
        case ID:
            // pick loop counter name
            node->data.loop.loop_counter_name = parser_forward(parser, ID)->value;
            parser_forward(parser, COLON);

            start = atoi(parser_forward(parser, INT)->value);
            prev = parser_forward_with_list(parser, (TokenType[]) {TO_KEYWORD, TIMES_KEYWORD}, 2,
                                            "loop range. Try using `loop i: 3 times` to loop from 0 to 3, "
                                            "or `loop i: 5 to 10 times` to loop from 5 to 9. "
                                            "In both methods you have a named loop counter you can access inside the loop.");
            if (prev->type == TIMES_KEYWORD) { // i: ▨ times
                node->data.loop.end = start;
            } else { // i: _ to ▨ times
                node->data.loop.start = start;
                node->data.loop.end = atoi(parser_forward(parser, INT)->value);
                parser_forward(parser, TIMES_KEYWORD);
            }
            break;
        default:
            // TODO: maybe replace with shorter message
            throw_exception_with_trace(PARSER, parser->lexer,
                                       "Illegal use of the loop statement.\n"
                                       "Usage:\n"
                                       "* `loop 3 times` to execute the loop body 3 times\n"
                                       "* `loop i: 3 times` to also have access to a named loop counter inside the loop that runs from 0 to 2\n"
                                       "* `loop i: 5 to 10 times` to set a custom range to the loop counter (here from 5 to 9)");
            break;
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
