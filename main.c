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
    for(int i = 0; code[i]; i++) {
        print_node(code[i]->node, 0);
    }
    log_print("\n\n");
    close_logger();

    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");
    for(int i =0; code[i]; i++) {
        gen(code[i]);
        printf("pop rax\n");
    }
    return 0;
}
