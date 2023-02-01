#ifndef INFINITY_COMPILER_AST_H
#define INFINITY_COMPILER_AST_H

#include "../token/token.h"
#include "../types/types.h"
#include "../variable/variable.h"
#include "../list/list.h"

typedef struct astNode AstNode;

/**
\Compound
Used as the root node of a file.
*/
typedef struct {
    List *children; // list of children nodes
} Compound;

/**
\StartExpression
Used to declare the starting point of the application.
*/
typedef struct {
    char *starting_point; // the name of the function in which to start the program execution
} StartExpression;

/**
\VariableDeclaration
Statements that create new variables.
 Variables can be declared but not initialized.\n
 In this case, they will be initialized with default value_expr
*/
typedef struct {
    Variable *var;
    AstNode *value; // an optional expression to initialize the variable with
//    LiteralValue value_expr; // possible initial value_expr to the variable
} VariableDeclaration;

/**
\Assignment
Assign new value_expr to a variable.
*/
typedef struct {
    Token *dst_variable;
    AstNode *expression; // the expression that will be assigned to the variable (or not, if it is null)
} Assignment;

/**
\FunctionDefinition
 Define new function.
*/
typedef struct {
    char *func_name;
    List *args; // list of Variables (arguments)
    DataType returnType;
    List *body; // body of the function, list of ast expressions
} FunctionDefinition;

/**
\FunctionCall
 Call a function.
*/
typedef struct {
    char *func_name; // change to some struct..?
    List *args;      // list of AST nodes.
    // type is AST nodes because arguments can be variables, literals or expressions
} FunctionCall;

/**
\IfStatement
*/
typedef struct {
    List *body_node; // list of AST nodes
    List *else_node; // list of AST nodes
    AstNode *condition;
} IfStatement;

/**
\Loop
 Perform code block a certain amount of times
*/
typedef struct {
    Expression *start;
    Expression *end;
    char *loop_counter_name;
    unsigned int loop_counter_col;
    int forward; // whether the loop counter (if there is one) advances forwards or backwards
    List *body;
} Loop;

/**
\WhileLoop
 Perform code block while a condition is true
*/
typedef struct {
    AstNode *condition;
    List *body; // list of AST nodes
} WhileLoop;

/**
\ReturnStatement
 Return value_expr from a function.
*/
typedef struct {
    AstNode *value_expr;
    unsigned int parent_function_arg_count; // to be used by the code generator in the `ret` instruction
} ReturnStatement;

typedef enum {
    AST_COMPOUND,             // used as the root node of a file
    AST_START_EXPRESSION,
    AST_EXPRESSION,           // an expression (5+2 or 2*x)
    AST_VARIABLE_DECLARATION, // declaration of variables
    AST_ASSIGNMENT,
    AST_FUNCTION_DEFINITION,
    AST_FUNCTION_CALL,
    AST_IF_STATEMENT,
    AST_LOOP,
    AST_WHILE_LOOP,
    AST_RETURN_STATEMENT,
    AST_NOOP, // no operation
} AstType;

typedef union {
    Compound compound;
    StartExpression start_expr;
    Expression expression;
    VariableDeclaration variable_declaration;
    Assignment assignment;
    FunctionDefinition function_definition;
    FunctionCall function_call;
    IfStatement if_statement;
    Loop loop;
    WhileLoop while_loop;
    ReturnStatement return_statement;
} AstData;

typedef struct astNode {
    AstType type;
    AstData data;
} AstNode;

AstNode *init_ast(AstType type);

void ast_dispose(AstNode *node);

AstNode *init_ast_compound(AstNode *node);

AstNode *init_ast_start_expression(AstNode *node);

AstNode *init_ast_expression(AstNode *node);

AstNode *init_ast_var_declaration(AstNode *node);

AstNode *init_ast_assignment(AstNode *node);

AstNode *init_ast_function_definition(AstNode *node);

AstNode *init_ast_function_call(AstNode *node);

AstNode *init_ast_if_statement(AstNode *node);

AstNode *init_ast_loop(AstNode *node);

AstNode *init_ast_while_loop(AstNode *node);

AstNode *init_ast_return_statement(AstNode *node);

AstNode *init_ast_noop(AstNode *node);

#endif //INFINITY_COMPILER_AST_H
