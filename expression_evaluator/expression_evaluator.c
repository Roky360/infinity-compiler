#include "expression_evaluator.h"
#include "../logging/logging.h"
#include "../io/io.h"
#include "../hash_table/hash_table.h"
#include "../config/table_initializers.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *print_ar_token(const void *item) {
    ArithmeticToken *t = (ArithmeticToken *) item;
    char *buf;
    switch (t->type) {
        case NUMBER:
            alsprintf(&buf, "%f", t->value.number);
            return buf;
        case OPERATOR:
        case PLACEHOLDER:
            return t->value.op;
        case PAREN:
            return (char[]) {t->value.paren, 0};
        case VAR:
            return t->value.var;
    }
}

ArithmeticToken *init_empty_arithmetic_token() {
    ArithmeticToken *tok = malloc(sizeof(ArithmeticToken));
    if (!tok)
        throw_memory_allocation_error(LEXER);
    return tok;
}

ArithmeticToken *init_arithmetic_token_with(ArithmeticTokenType type, ArithmeticTokenValue value, Token *original_tok) {
    ArithmeticToken *tok = init_empty_arithmetic_token();
    tok->type = type;
    tok->value = value;
    tok->original_tok = original_tok;
    return tok;
}

int is_operator(char *op) {
    return hash_table_lookup(precedence_map, op) != NULL;
}

int is_parentheses(TokenType type) {
    return type == L_PARENTHESES || type == R_PARENTHESES;
}

int get_precedence(char *op) {
    char *precedence = hash_table_lookup(precedence_map, op);
    if (precedence) {
        return atoi(precedence);
    } else {
        return -1;
    }
}

int is_right_associative(char *op) {
    // power and not operators are right associative
    return strstr(OP_POW OP_NOT, op) != NULL;
}

// returns if the expression can be parsed to a constant value (1), or it contains variables (0)
int parse_tokens(List *expression, List *tokens, Lexer *lexer) {
    Token *token;
    ArithmeticToken *arithmeticToken;
    ArithmeticTokenType prev_token_type = -1;
    char *conversion_res, prev_paren = 0;
    int i, paren_count = 0, should_close_paren = 0;
    int parsable = 1;

    for (i = 0; i < expression->size; i++) {
        // return false if expression contains variables
        token = expression->items[i];
        arithmeticToken = init_empty_arithmetic_token();

        if (token->type == ID) {
            parsable = 0;
            arithmeticToken->type = VAR;
            arithmeticToken->value.var = token->value; // put the name of the variable in the `var` value
            arithmeticToken->original_tok = token;
            list_push(tokens, arithmeticToken);
            prev_token_type = NUMBER;
            prev_paren = 0;
        } else if (token->type == INT || token->type == DOUBLE || token->type == CHAR) {
            // Number Token
            arithmeticToken->type = NUMBER;
            if (token->type == CHAR) {
                arithmeticToken->value.number = token->value[0];
            } else {
                arithmeticToken->value.number = strtod(token->value, &conversion_res);
                if (*conversion_res != 0) {
                    log_exception_with_trace(PARSER, lexer, token->line, token->column, token->length,
                                             "Error converting \"%s\" to float.", conversion_res);
                }
            }
            arithmeticToken->original_tok = token;

            // check that there are no two operands adjacent to each other, like 5 7 +
            if (prev_token_type == NUMBER) {
                if (arithmeticToken->value.number < 0) {
                    // for expressions like 1-7 where "-7" is read together as one number, and an operator is missing
                    list_push(tokens, init_arithmetic_token_with(OPERATOR, (ArithmeticTokenValue) {.op = "-"}, NULL));
                    arithmeticToken->value.number *= -1;
                } else {
                    log_exception_with_trace(PARSER, lexer, token->line, token->column, token->length,
                                             "Missing operator between operands.");
                }
            }
            list_push(tokens, arithmeticToken);

            // close helper parentheses
            if (should_close_paren && paren_count == 0) {
                list_push(tokens, init_arithmetic_token_with(PAREN, (ArithmeticTokenValue) {.paren = ')'}, NULL));
                should_close_paren--;
            }
            prev_token_type = NUMBER;
            prev_paren = 0;
        } else if (is_operator(token->value)) {
            // Operator or parenthesis arithmeticToken
            arithmeticToken->type = OPERATOR;
            arithmeticToken->value.op = token->value;

            // if current arithmeticToken is minus operator or not operator, and no operand was before it (like -1 or !true)
            if ((token->type == SUB_OP || token->type == NOT_OPERATOR_KEYWORD)
                && prev_token_type != NUMBER && prev_token_type != PLACEHOLDER && prev_paren != ')') {
                list_push(tokens, init_arithmetic_token_with(PAREN, (ArithmeticTokenValue) {.paren = '('}, NULL));
                if (token->type == NOT_OPERATOR_KEYWORD) { // placeholder token for "not" operator
                    list_push(tokens,
                              init_arithmetic_token_with(PLACEHOLDER,
                                                         (ArithmeticTokenValue) {.op = NOT_OP_PLACEHOLDER}, NULL));
                    prev_token_type = PLACEHOLDER;
                } else { // dummy zero for minus operator
                    list_push(tokens, init_arithmetic_token_with(NUMBER, (ArithmeticTokenValue) {.number = 0}, NULL));
                    prev_token_type = OPERATOR;
                }
                should_close_paren++;
            } else {
                prev_token_type = OPERATOR;
            }
            arithmeticToken->original_tok = token;
            list_push(tokens, arithmeticToken);

            if (token->type == FACTORIAL_OP) {
                list_push(tokens,
                          init_arithmetic_token_with(PLACEHOLDER, (ArithmeticTokenValue) {.op = FACT_OP_PLACEHOLDER},
                                                     NULL));
                prev_token_type = PLACEHOLDER;
            }
            prev_paren = 0;
        } else if (is_parentheses(token->type)) {
            arithmeticToken->type = PAREN;
            arithmeticToken->value.paren = *token->value;

            arithmeticToken->original_tok = token;
            list_push(tokens, arithmeticToken);
            prev_token_type = PAREN;
            prev_paren = arithmeticToken->value.paren;
            // update parentheses count
            paren_count += *token->value == '(' ? 1 : -1;
        } else {
            log_exception_with_trace(PARSER, lexer, token->line, token->column, token->length,
                                     "Invalid token for an expression: %s.", token->value);
        }
    }
    // check if all open brackets have a match
    if (paren_count != 0) {
        log_exception_with_trace(PARSER, lexer, token->line, token->column, token->length,
                                 "Mismatch brackets.");
    }
    // check if a helper parentheses should be added
    if (should_close_paren)
        list_push(tokens, init_arithmetic_token_with(PAREN, (ArithmeticTokenValue) {.paren = ')'}, NULL));

    return parsable;
}

void infix_to_postfix(List *infix, List *postfix, Lexer *lexer) {
    List *operator_stack;
    ArithmeticToken *prev_token, *curr_token;
    operator_stack = init_list(sizeof(ArithmeticToken *));

    for (int i = 0; i < infix->size; i++) {
        curr_token = infix->items[i];
        if (curr_token->type == NUMBER || curr_token->type == PLACEHOLDER || curr_token->type == VAR) {
            list_push(postfix, curr_token);
        } else if (curr_token->type == OPERATOR) {
            while (!list_is_empty(operator_stack) &&
                   ((ArithmeticToken *) list_get_last(operator_stack))->type == OPERATOR &&
                   (is_right_associative(curr_token->value.op) ?
                    get_precedence(curr_token->value.op) <
                    get_precedence(((ArithmeticToken *) list_get_last(operator_stack))->value.op)
                                                               : get_precedence(curr_token->value.op) <= get_precedence(
                                   ((ArithmeticToken *) list_get_last(operator_stack))->value.op))) {
                list_push(postfix, list_pop(operator_stack));
            }
            list_push(operator_stack, curr_token);
        } else if (curr_token->type == PAREN) {
            if (curr_token->value.paren == '(') {
                list_push(operator_stack, curr_token);
            } else {
                while (((ArithmeticToken *) list_get_last(operator_stack))->type != PAREN) {
                    list_push(postfix, list_pop(operator_stack));
                }
                list_pop(operator_stack);
                if (prev_token->type == PAREN && prev_token->value.paren == '(') {
                    log_exception_with_trace(PARSER, lexer, curr_token->original_tok->line,
                                             curr_token->original_tok->column, curr_token->original_tok->length,
                                             "Expected an expression inside parentheses.");
                }
            }
        }
        prev_token = curr_token;
    }

    while (!list_is_empty(operator_stack)) {
        list_push(postfix, list_pop(operator_stack));
    }

    list_dispose(operator_stack);
}

double evaluate_postfix(List *postfix, Lexer *lexer) {
    List *stack;
    ArithmeticToken *curr_token, *result_token, *left_token, *right_token;
    int i;
    double a, b, result;
    double (*applier_func)(double, double, char *, char *);
    stack = init_list(sizeof(ArithmeticToken *));

    for (i = 0; i < postfix->size; i++) {
        curr_token = postfix->items[i];

        if (curr_token->type == NUMBER || curr_token->type == PLACEHOLDER) {
            list_push(stack, curr_token);
        } else if (curr_token->type == OPERATOR) {
            result_token = init_empty_arithmetic_token();
            right_token = list_pop(stack);
            left_token = list_pop(stack);
            if (!right_token || !left_token) {
                log_exception_with_trace(PARSER, lexer, curr_token->original_tok->line,
                                         curr_token->original_tok->column, curr_token->original_tok->length,
                                         "Missing operands.");
            }
            b = right_token->value.number;
            a = left_token->value.number;

            applier_func = hash_table_lookup(operator_to_applier_function_map, curr_token->value.op);
            if (applier_func) {
                result = applier_func(a, b, left_token->type == PLACEHOLDER ? left_token->value.op : "",
                                      right_token->type == PLACEHOLDER ? right_token->value.op : "");
            } else {
                log_exception_with_trace(PARSER, lexer, curr_token->original_tok->line,
                                         curr_token->original_tok->column, curr_token->original_tok->length,
                                         "Invalid operator: %s.", curr_token->value.op);
            }

            result_token->type = NUMBER;
            result_token->value.number = result;
            list_push(stack, result_token);
        }
    }

    result = ((ArithmeticToken *) list_pop(stack))->value.number;
    if (!list_is_empty(stack)) {
        log_exception_with_trace(PARSER, lexer, curr_token->original_tok->line,
                                 curr_token->original_tok->column, curr_token->original_tok->length,
                                 "Invalid expression.");
    }

    list_dispose(stack);
    return result;
}

/* expression is an address to a list of tokens
 * Returns if the expression can be evaluated, or it contains variables.
 * If it can be evaluated - the result is stored in `res` and returns true
 * If not - `expression` will contain the postfix expression and false is returned
 * */
int evaluate_expression(List **expression, double *res, Lexer *lexer) {
    List *infix, *postfix;
    if ((*expression)->size == 0)
        return 0;
    infix = init_list(sizeof(ArithmeticToken *));
    postfix = init_list(sizeof(ArithmeticToken *));

    // if the expression contains variables, convert to postfix and return false
    if (!parse_tokens(*expression, infix, lexer)) {
        list_clear(*expression, 0);
        infix_to_postfix(infix, *expression, lexer);
        return 0;
    }
    infix_to_postfix(infix, postfix, lexer);
    *res = evaluate_postfix(postfix, lexer);
    *expression = postfix;
//    list_dispose(infix);
//    free(infix);
//    free(postfix);
    return 1;
}
