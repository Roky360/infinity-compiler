#include "compiler.h"
#include "../lexer/lexer.h"
#include "../parser/parser.h"
#include "../semantic_analyzer/semantic_analyzer.h"
#include "../logging/logging.h"
#include "../io/io.h"
#include "../config/globals.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void compiler_compile(char *src) {
    Lexer *lexer;
    Parser *parser;
    AstNode *root;
    SemanticAnalyzer *analyzer;
    init_globals();
    lexer = init_lexer(src);
    parser = init_parser(lexer);

    // parse
    root = parser_parse(parser);
    // analyze tree
    analyzer = init_semantic_analyzer(root);
    semantic_analyze_tree(analyzer);
    // generate code

//     Token *tok;
//     while ((tok = lexer_next_token(lexer))->type != EOF_TOKEN)
//     {
//         printf("token '%s'\t%s\t\t\trow %d, col %d\n", tok->value, token_type_to_str(tok->type), lexer->row, lexer->col);
//     }

    parser_dispose(parser);
    semantic_analyzer_dispose(analyzer);
    clean_globals();
}

void compiler_compile_file(const char *filename) {
    char *src;

#ifdef INF_DEBUG
    clock_t start, end;
    double elapsed_time_ms;
    char *done_msg;

    start = clock();
#endif

    /** Compiler Action */
    src = read_file(filename);

    compiler_compile(src);

    free(src);

#ifdef INF_DEBUG
    puts("");
    // Print done message with time elapsed
    end = clock();
    elapsed_time_ms = (double)(end - start) / CLOCKS_PER_SEC * 1000;
    if (elapsed_time_ms >= 100)
    {
        alsprintf(&done_msg, "Compiled successfully in %.1f seconds", elapsed_time_ms / 1000);
    }
    else
    {
        alsprintf(&done_msg, "Compiled successfully in %d ms", (int)elapsed_time_ms);
    }
    log_debug(COMPILER, done_msg);
#endif
}
