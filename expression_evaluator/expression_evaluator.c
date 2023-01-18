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
            return t->value.op;
        case PAREN:
            return (char[]) {t->value.paren, 0};
    }
}

ArithmeticToken *init_empty_arithmetic_token() {
    ArithmeticToken *tok = malloc(sizeof(ArithmeticToken));
    if (!tok)
        throw_memory_allocation_error(LEXER);
    return tok;
}

ArithmeticToken *init_arithmetic_token_with(ArithmeticTokenType type, ArithmeticTokenValue value) {
    ArithmeticToken *tok = init_empty_arithmetic_token();
    tok->type = type;
    tok->value = value;
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
    /*if (strstr("^!", op) != NULL) {
        return 8;
    } else if (strstr("not", op) != NULL) {
        return 7;
    } else if (strstr("/*%%", op) != NULL) {
        return 6;
    } else if (strstr("+-", op) != NULL) {
        return 5;
    } else if (strstr("><", op) != NULL) {
        return 4;
    } else if (strstr("==", op) != NULL) {
        return 3;
    } else if (strstr("and", op) != NULL) {
        return 2;
    } else if (strstr("or", op) != NULL) {
        return 1;
    } else {
        return -1;
    }*/
}

int is_right_associative(char *op) {
    // power and not operators are right associative
    return strstr(OP_POW OP_NOT, op) != NULL;
}

// returns if the expression can be parsed to a constant value (1), or it contains variables (0)
int parse_tokens(const List *expression, List *tokens) {
    Token *token;
    ArithmeticToken *arithmeticToken;
    ArithmeticTokenType prev_token_type = -1;
    char *conversion_res;
    int i, paren_count = 0, should_close_paren = 0;

    for (i = 0; i < expression->size; i++) {
        // return false if expression contains variables
        token = expression->items[i];
        if (token->type == ID) {
            return 0;
        }
        arithmeticToken = init_empty_arithmetic_token();

        if (token->type == INT || token->type == DOUBLE || token->type == CHAR) {
            // Number Token
            arithmeticToken->type = NUMBER;
            if (token->type == CHAR) {
                arithmeticToken->value.number = token->value[0];
            } else {
                arithmeticToken->value.number = strtod(token->value, &conversion_res);
                if (*conversion_res != 0) {
                    fprintf(stderr, "Error converting \"%s\" to float", conversion_res);
                    exit(1);
                }
            }
            list_push(tokens, arithmeticToken);

            if (should_close_paren && paren_count == 0) {
                list_push(tokens, init_arithmetic_token_with(PAREN, (ArithmeticTokenValue) {.paren = ')'}));
                should_close_paren = 0;
            }
            prev_token_type = NUMBER;
        } else if (is_operator(token->value)) {
            // Operator or parenthesis arithmeticToken
            arithmeticToken->type = OPERATOR;
            arithmeticToken->value.op = token->value;

            // if current arithmeticToken is minus operator or not operator, and no operand was before it (like -1 or !true)
            if ((token->type == SUB_OP || token->type == NOT_OPERATOR_KEYWORD) && prev_token_type != NUMBER) {
                list_push(tokens, init_arithmetic_token_with(PAREN, (ArithmeticTokenValue) {.paren = '('}));
                if (token->type == NOT_OPERATOR_KEYWORD) { // placeholder token for "not" operator
                    list_push(tokens,
                              init_arithmetic_token_with(NUMBER, (ArithmeticTokenValue) {.op = NOT_OP_PLACEHOLDER}));
                } else { // dummy zero for minus operator
                    list_push(tokens, init_arithmetic_token_with(NUMBER, (ArithmeticTokenValue) {.number = 0}));
                }
                should_close_paren = 1;
            }
            list_push(tokens, arithmeticToken);
            prev_token_type = OPERATOR;

            if (token->type == FACTORIAL_OP)
                list_push(tokens,
                          init_arithmetic_token_with(NUMBER, (ArithmeticTokenValue) {.op = FACT_OP_PLACEHOLDER}));
        } else if (is_parentheses(token->type)) {
            arithmeticToken->type = PAREN;
            arithmeticToken->value.paren = *token->value;

            list_push(tokens, arithmeticToken);
            prev_token_type = PAREN;
            // update parentheses count
            paren_count += *token->value == '(' ? 1 : -1;
        } else {
            fprintf(stderr, "Invalid character: %s\n", token->value);
            exit(1);
        }
    }
    // check if all open brackets have a match
    if (paren_count != 0) {
        fprintf(stderr, "Mismatch brackets.\n");
        exit(1);
    }
    // check if a helper parentheses should be added
    if (should_close_paren)
        list_push(tokens, init_arithmetic_token_with(PAREN, (ArithmeticTokenValue) {.paren = ')'}));

    return 1;
}

void infix_to_postfix(List *infix, List *postfix) {
    List *operator_stack;
    ArithmeticToken *prev_token, *currToken;
    operator_stack = init_list(sizeof(ArithmeticToken *));

    for (int i = 0; i < infix->size; i++) {
        currToken = infix->items[i];
        if (currToken->type == NUMBER) {
            list_push(postfix, currToken);
        } else if (currToken->type == OPERATOR) {
            while (!list_is_empty(operator_stack) &&
                   ((ArithmeticToken *) list_get_last(operator_stack))->type == OPERATOR &&
                   (is_right_associative(currToken->value.op) ?
                    get_precedence(currToken->value.op) <
                    get_precedence(((ArithmeticToken *) list_get_last(operator_stack))->value.op)
                                                              : get_precedence(currToken->value.op) <= get_precedence(
                                   ((ArithmeticToken *) list_get_last(operator_stack))->value.op))) {
                list_push(postfix, list_pop(operator_stack));
            }
            list_push(operator_stack, currToken);
        } else if (currToken->type == PAREN) {
            if (currToken->value.paren == '(') {
                list_push(operator_stack, currToken);
            } else {
                while (((ArithmeticToken *) list_get_last(operator_stack))->type != PAREN) {
                    list_push(postfix, list_pop(operator_stack));
                }
                list_pop(operator_stack);
                if (prev_token->type == PAREN && prev_token->value.paren == '(') {
                    fprintf(stderr, "Expected an expression inside parentheses.\n");
                    exit(1);
                }
            }
        }
        prev_token = currToken;
    }

    while (!list_is_empty(operator_stack)) {
        list_push(postfix, list_pop(operator_stack));
    }

    list_dispose(operator_stack);
}

double evaluate_postfix(List *postfix) {
    List *stack;
    ArithmeticToken *curr_token, *result_token, *left_token, *right_token;
    int i;
    double a, b, result;
    double (*applier_func)(double, double, char *, char *);
    stack = init_list(sizeof(ArithmeticToken *));

    for (i = 0; i < postfix->size; i++) {
        curr_token = postfix->items[i];
        result_token = init_empty_arithmetic_token();

        if (curr_token->type == NUMBER) {
            list_push(stack, curr_token);
        } else if (curr_token->type == OPERATOR) {
            right_token = list_pop(stack);
            left_token = list_pop(stack);
            if (!right_token || !left_token) {
                fprintf(stderr, "Missing operands\n");
                exit(1);
            }
            b = right_token->value.number;
            a = left_token->value.number;

            applier_func = hash_table_lookup(operator_to_applier_function_map, curr_token->value.op);
            if (applier_func) {
                result = applier_func(a, b, left_token->value.op, right_token->value.op);
            } else {
                fprintf(stderr, "Invalid operator: %s\n", curr_token->value.op);
                exit(1);
            }

            /*if (*curr_token->value.op == '+') {
                result = a + b;
            } else if (*curr_token->value.op == '-') {
                result = a - b;
            } else if (*curr_token->value.op == '*') {
                result = a * b;
            } else if (*curr_token->value.op == '/') {
                if (b == 0) {
                    fprintf(stderr, "Attempting to divide by zero (%.2f / 0)\n", a);
                    exit(1);
                }
                result = a / b;
            } else if (*curr_token->value.op == '^') {
                result = pow(a, b);
            } else if (*curr_token->value.op == '%') {
                result = fmod(a, b);
            } else if (*curr_token->value.op == '!') {
                if (strcmp(right_token->value.op, FACT_OP_PLACEHOLDER) != 0) {
                    fprintf(stderr, "Invalid expression\n");
                    exit(1);
                }
                result = factorial(a);
            } else if (!strcmp(curr_token->value.op, "and")) {
                result = a && b;
            } else if (!strcmp(curr_token->value.op, "or")) {
                result = a || b;
            } else if (!strcmp(curr_token->value.op, "not")) {
                if (strcmp(left_token->value.op, NOT_OP_PLACEHOLDER) != 0) {
                    fprintf(stderr, "Invalid expression\n");
                    exit(1);
                }
                result = !b;
            } else if (!strcmp(curr_token->value.op, "==")) {
                result = a == b;
            } else if (*curr_token->value.op == '>') {
                result = a > b;
            } else if (!strcmp(curr_token->value.op, ">=")) {
                result = a >= b;
            } else if (*curr_token->value.op == '<') {
                result = a < b;
            } else if (!strcmp(curr_token->value.op, "<=")) {
                result = a <= b;
            } else {
                fprintf(stderr, "Invalid operator: %s\n", curr_token->value.op);
                exit(1);
            }*/
            result_token->type = NUMBER;
            result_token->value.number = result;
            list_push(stack, result_token);
        }
    }

    result = ((ArithmeticToken *) list_pop(stack))->value.number;
    if (!list_is_empty(stack)) {
        fprintf(stderr, "Invalid expression\n");
        exit(1);
    }

    list_dispose(stack);
    return result;
}

/* expression is a list of tokens
 * Returns if the expression can be parsed, or it contains variables.
 * */
int evaluate_expression(const List *expression, double *res) {
    // TODO: if the expression contains vars, update the containing_vars field in the expression instance
    List *infix, *postfix;
    infix = init_list(sizeof(ArithmeticToken *));
    postfix = init_list(sizeof(ArithmeticToken *));

    if (!parse_tokens(expression, infix))
        return 0;
    infix_to_postfix(infix, postfix);
    *res = evaluate_postfix(postfix);

//    list_print(infix, print_ar_token);
//    list_print(postfix, print_ar_token);
//    printf(" = %f\n", *res);

    list_dispose(infix);
    free(postfix);
    return 1;
}
