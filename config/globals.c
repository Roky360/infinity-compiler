#include "globals.h"

TokenType data_types[] = {VOID_KEYWORD, INT_KEYWORD, BOOL_KEYWORD, CHAR_KEYWORD, STRING_KEYWORD};
int data_types_len = ARRLEN(data_types);

void init_globals() {
//    int i;
//    TokenType *tp;
//    data_types = init_list(sizeof(TokenType *));
//
//    for (i = 0; i < ARRLEN(tTypes); i++) {
//        tp = tTypes + i;
//        list_push(data_types, (void *) (tp));
//    }
}

void clean_globals() {
//    list_dispose(data_types);
}
