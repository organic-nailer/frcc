#include "frcc.h"

char *user_input;

int main(int argc, char **argv) {
    if(argc != 2) {
        fprintf(stderr, "引数の個数が正しくありません\n");
        return 1;
    }
    
    open_logger();
    user_input = argv[1];
    log_print("input: %s\n", user_input);
    tokenize(argv[1]);
    log_print("tokenized\n");
    char dst[20];
    for(Token* tkn = token; tkn; tkn = tkn->next) {
        if(tkn->value) {
            log_print("%d ", tkn->value);
        }
        else {
            strncpy(dst, tkn->str, tkn->length);
            dst[tkn->length] = NULL;
            log_print("%d_%s ", tkn->kind, dst);
        }
    }
    log_print("\n");
    program();
    log_print("parsed\n");
    for(int i = 0; i < global_functions->vals->length; i++) {
        Function* func = global_functions->vals->data[i];
        print_node(func->node, 0);
        LVar* loc = func->locals;
        while(loc) {
            log_print("%.*s :%d\n", loc->length, loc->name, loc->offset);
            loc = loc->next;
        }
    }
    log_print("\n\n");
    close_logger();

    gen();
    return 0;
}
