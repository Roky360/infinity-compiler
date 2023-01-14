cmake_minimum_required(VERSION 3.24)
project(infinity_compiler C)

set(CMAKE_C_STANDARD 23)

add_executable(infinity_compiler main.c config/globals.c config/globals.h lexer/lexer.c lexer/lexer.h
        token/token.c token/token.h list/list.c list/list.h compiler/compiler.c compiler/compiler.h io/io.c io/io.h
        types/types.c types/types.h ast/ast.c ast/ast.h variable/variable.c variable/variable.h
        logging/logging.c logging/logging.h parser/parser.c parser/parser.h test_evals/expression_evaluator.h test_evals/expression_evaluator.c symbol_table/symbol_table.c symbol_table/symbol_table.h symbol_table/symbol_entry/symbol_entry.c symbol_table/symbol_entry/symbol_entry.h semantic_analyzer/semantic_analyzer.c semantic_analyzer/semantic_analyzer.h scope_stack/scope/scope.c scope_stack/scope/scope.h scope_stack/scope_stack.c scope_stack/scope_stack.h)
