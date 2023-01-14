#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#define MAX_TOKEN_LEN 100
#define NOT_OP_PLACEHOLDER 'n'

typedef enum {
    NUMBER,
    OPERATOR,
    PAREN,
} TokenType;

typedef struct {
    TokenType type;
    union {
        double number;
        char op;
        char paren;
    } value;
} Token;

typedef struct {
    Token *tokens;
    int size;
    int capacity;
} TokenArray;

void token_array_init(TokenArray *array) {
    array->size = 0;
    array->capacity = 8;
    array->tokens = malloc(sizeof(Token) * array->capacity);
}

void token_array_free(TokenArray *array) {
    free(array->tokens);
}

void token_array_push(TokenArray *array, Token token) {
    if (array->size == array->capacity) {
        array->capacity *= 2;
        array->tokens = realloc(array->tokens, sizeof(Token) * array->capacity);
    }
    array->tokens[array->size++] = token;
}

int token_array_is_empty(TokenArray *array) {
    return array->size == 0;
}

Token token_array_pop(TokenArray *array) {
    if (token_array_is_empty(array)) {
        fprintf(stderr, "Missing operands\n");
        exit(1);
    }
    return array->tokens[--array->size];
}

Token token_array_top(TokenArray *array) {
    return array->tokens[array->size - 1];
}

int get_precedence(char op) {
    switch (op) {
        case '^':
            return 8;
        case '*':
        case '/':
        case '%':
        case '!':
            return 7;
        case '+':
        case '-':
            return 6;
        case '>':
        case '<':
            return 5;
        case '=':
            return 4;
        case '&':
            return 3;
        case '|':
            return 2;
        default:
            return -1;
    }
}

int is_right_associative(char op) {
    return op == '^' || op == '!';
}

void parse_tokens(const char *expression, TokenArray *tokens) {
    Token token;
    const char *p = expression;
    char current_token[MAX_TOKEN_LEN];
    TokenType prev_token_type = -1;
    int should_close_paren = 0;
    int current_token_len = 0;
    int paren_count = 0;

    while (*p) {
        if (isspace(*p)) {
            // Ignore whitespace
        } else if (isdigit(*p) || *p == '.') {
            // Number token
            current_token[current_token_len++] = *p;
            prev_token_type = NUMBER;
        } else if (strchr("+-*/^%%()|&=!><", *p)) { // TODO: add <= , >=
            // Operator or parenthesis token
            if (current_token_len > 0) {
                // Flush current number token
                current_token[current_token_len] = '\0';
                token.type = NUMBER;
                token.value.number = atof(current_token);
                token_array_push(tokens, token);
                current_token_len = 0;

                if (should_close_paren && paren_count == 0) {
                    token_array_push(tokens, (Token) {.type = PAREN, .value.paren = ')'});
                    should_close_paren = 0;
                }
            }
            token.type = (*p == '(' || *p == ')') ? PAREN : OPERATOR;
            token.value.op = *p;

            // if current token is minus operator or not operator, and no operand was before it (like -1 or !true)
            if (token.type == OPERATOR && (token.value.op == '-' || token.value.op == '!') &&
                prev_token_type != NUMBER) {
                token_array_push(tokens, (Token) {.type = PAREN, .value.paren = '('});
                if (token.value.op == '!') // placeholder token for "not" operator
                    token_array_push(tokens, (Token) {.type = NUMBER, .value.op = NOT_OP_PLACEHOLDER});
                else // dummy zero for minus operator
                    token_array_push(tokens, (Token) {.type = NUMBER, .value.number = 0});
                should_close_paren = 1;
            }
            token_array_push(tokens, token);
            prev_token_type = token.type;

            // if current token is parentheses - update parentheses count
            if (token.type == PAREN)
                paren_count += token.value.paren == '(' ? 1 : -1;
        } else {
            fprintf(stderr, "Invalid character: %c\n", *p);
            exit(1);
        }
        p++;
    }

    if (current_token_len > 0) {
        // Flush final number token
        current_token[current_token_len] = '\0';
        token.type = NUMBER;
        token.value.number = atof(current_token);
        token_array_push(tokens, token);
    }

    if (paren_count != 0) {
        fprintf(stderr, "Mismatch brackets.\n");
        exit(1);
    }

    if (should_close_paren)
        token_array_push(tokens, (Token) {.type = PAREN, .value.paren = ')'});

    token_array_push(tokens, (Token) {.value.op = '$'});
    Token *t = tokens->tokens;
    while ((*t).value.op != '$') {
        switch ((*t).type) {
            case NUMBER:
                printf("%f ", (*t).value.number);
                break;
            case OPERATOR:
                printf("%c ", (*t).value.op);
                break;
            case PAREN:
                printf("%c ", (*t).value.paren);
                break;
        }
        t++;
    }
    puts("");
    token_array_pop(tokens);
}

void infix_to_postfix(TokenArray *infix, TokenArray *postfix) {
    TokenArray operator_stack;
    Token prev_token;
    token_array_init(&operator_stack);

    for (int i = 0; i < infix->size; i++) {
        Token token = infix->tokens[i];
        if (token.type == NUMBER) {
            token_array_push(postfix, token);
        } else if (token.type == OPERATOR) {
            while (!token_array_is_empty(&operator_stack) &&
                   token_array_top(&operator_stack).type == OPERATOR &&
                   (is_right_associative(token.value.op) ? get_precedence(token.value.op) <
                                                           get_precedence(token_array_top(&operator_stack).value.op) :
                    get_precedence(token.value.op) <= get_precedence(token_array_top(&operator_stack).value.op))) {
                token_array_push(postfix, token_array_pop(&operator_stack));
            }
            token_array_push(&operator_stack, token);
        } else if (token.type == PAREN) {
            if (token.value.paren == '(') {
                token_array_push(&operator_stack, token);
            } else {
                while (token_array_top(&operator_stack).type != PAREN) {
                    token_array_push(postfix, token_array_pop(&operator_stack));
                }
                token_array_pop(&operator_stack);
                if (prev_token.type == PAREN && prev_token.value.paren == '(') {
                    fprintf(stderr, "Expected an expression inside parentheses.\n");
                    exit(1);
                }
            }
        }
        prev_token = token;
    }

    while (!token_array_is_empty(&operator_stack)) {
        token_array_push(postfix, token_array_pop(&operator_stack));
    }

    token_array_free(&operator_stack);
}

double evaluate_postfix(TokenArray *postfix) {
    TokenArray stack;
    Token result_token, left_token;
    token_array_init(&stack);

    token_array_push(postfix, (Token) {.value.op = '$'});
    Token *t = postfix->tokens;
    while ((*t).value.op != '$') {
        switch ((*t).type) {
            case NUMBER:
                printf("%f ", (*t).value.number);
                break;
            case OPERATOR:
                printf("%c ", (*t).value.op);
                break;
            case PAREN:
                printf("%c ", (*t).value.paren);
                break;
        }
        t++;
    }
    puts("");
    token_array_pop(postfix);

    for (int i = 0; i < postfix->size; i++) {
        Token token = postfix->tokens[i];
        if (token.type == NUMBER) {
            token_array_push(&stack, token);
        } else if (token.type == OPERATOR) {
            double b = token_array_pop(&stack).value.number;
            left_token = token_array_pop(&stack);
            double a = left_token.value.number;
            double result;
            switch (token.value.op) {
                case '+':
                    result = a + b;
                    break;
                case '-':
                    result = a - b;
                    break;
                case '*':
                    result = a * b;
                    break;
                case '/':
                    if (b == 0) {
                        fprintf(stderr, "Attempting to divide by zero (%.2f / 0)\n", a);
                        exit(1);
                    }
                    result = a / b;
                    break;
                case '^':
                    result = pow(a, b);
                    break;
                case '%':
                    result = fmod(a, b);
                    break;
                case '&':
                    result = a && b;
                    break;
                case '|':
                    result = a || b;
                    break;
                case '!':
                    if (left_token.value.op != NOT_OP_PLACEHOLDER) {
                        fprintf(stderr, "Invalid expression\n");
                        exit(1);
                    }
                    result = !b;
                    break;
                case '=':
                    result = a == b;
                    break;
                case '>':
                    result = a > b;
                    break;
                case '<':
                    result = a < b;
                    break;
                default:
                    fprintf(stderr, "Invalid operator: %c\n", token.value.op);
                    exit(1);
            }
            result_token.type = NUMBER;
            result_token.value.number = result;
            token_array_push(&stack, result_token);
        }
    }

    double result = token_array_pop(&stack).value.number;
    if (!token_array_is_empty(&stack)) {
        fprintf(stderr, "Invalid expression\n");
        exit(1);
    }

    token_array_free(&stack);
    return result;
}

double evaluate(const char *expression) {
    TokenArray infix, postfix;
    token_array_init(&infix);
    token_array_init(&postfix);
    parse_tokens(expression, &infix);
    infix_to_postfix(&infix, &postfix);
    double result = evaluate_postfix(&postfix);
    token_array_free(&infix);
    token_array_free(&postfix);
    return result;
}

/*
***
Shunting yard algorithm
***
*/
