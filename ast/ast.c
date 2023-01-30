#include "ast.h"
#include "../logging/logging.h"
#include <stdio.h>
#include <stdlib.h>

AstNode *init_ast(AstType type) {
    AstNode *ast = malloc(sizeof(AstNode));
    if (!ast) {
        log_error(COMPILER, "Can't allocate memory for AST.");
    }
    ast->type = type;

    switch (ast->type) {
        case AST_COMPOUND:
            return init_ast_compound(ast);
        case AST_START_EXPRESSION:
            return init_ast_start_expression(ast);
        case AST_EXPRESSION:
            return init_ast_expression(ast);
        case AST_VARIABLE_DECLARATION:
            return init_ast_var_declaration(ast);
        case AST_ASSIGNMENT:
            return init_ast_assignment(ast);
        case AST_FUNCTION_DEFINITION:
            return init_ast_function_definition(ast);
        case AST_FUNCTION_CALL:
            return init_ast_function_call(ast);
        case AST_IF_STATEMENT:
            return init_ast_if_statement(ast);
        case AST_LOOP:
            return init_ast_loop(ast);
        case AST_RETURN_STATEMENT:
            return init_ast_return_statement(ast);
        case AST_NOOP:
            return init_ast_noop(ast);
        default:
            return NULL;
    }
}

void ast_dispose(AstNode *node) {
    free(node);
}

AstNode *init_ast_compound(AstNode *node) {
    node->data = (AstData) {
            .compound = (Compound) {
                    .children = init_list(sizeof(AstNode *))
            }
    };
    return node;
}

AstNode *init_ast_start_expression(AstNode *node) {
    node->data = (AstData) {
            .start_expr = (StartExpression) {}
    };
    return node;
}

AstNode *init_ast_expression(AstNode *node) {
    node->data = (AstData) {.expression = init_expression()};
    return node;
}

AstNode *init_ast_var_declaration(AstNode *node) {
    node->data = (AstData) {.variable_declaration = (VariableDeclaration) {}};
    return node;
}

AstNode *init_ast_assignment(AstNode *node) {
    node->data = (AstData) {.assignment = (Assignment) {}};
    return node;
}

AstNode *init_ast_function_definition(AstNode *node) {
    node->data = (AstData) {.function_definition = (FunctionDefinition) {
            .args = init_list(sizeof(Variable *)),
            .body = init_list(sizeof(AstNode *))
    }};
    return node;
}

AstNode *init_ast_function_call(AstNode *node) {
    node->data = (AstData) {.function_call = (FunctionCall) {
            .args = init_list(sizeof(AstNode *)),
    }};
    return node;
}

AstNode *init_ast_if_statement(AstNode *node) {
    node->data = (AstData) {.if_statement = (IfStatement) {
            .body_node = init_list(sizeof(AstNode *)),
            .else_node = init_list(sizeof(AstNode *)),
    }};
    return node;
}

AstNode *init_ast_loop(AstNode *node) {
    node->data = (AstData) {.loop = (Loop) {
            .start = init_expression_p(),
            .end = init_expression_p(),
            .loop_counter_name = NULL,
            .forward = 1,
            .body = init_list(sizeof(AstNode *))
    }};
    node->data.loop.start->value = init_literal_value(TYPE_INT, (Value) {.double_value = 0});
    node->data.loop.end->value = init_literal_value(TYPE_INT, (Value) {.double_value = 0});
    return node;
}

AstNode *init_ast_return_statement(AstNode *node) {
    node->data = (AstData) {.return_statement = (ReturnStatement) {}};
    return node;
}

AstNode *init_ast_noop(AstNode *node) {
    node->data = (AstData) {};
    return node;
}
