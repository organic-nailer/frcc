#include "frcc.h"

char *user_input;

char *filename;

char *read_file(char *path) {
    FILE *fp = fopen(path, "r");
    if(!fp) error("cannot open %s: %s", path, strerror(errno));

    if(fseek(fp, 0, SEEK_END) == -1) {
        error("%s: fseek: %s", path, strerror(errno));
    }
    size_t size = ftell(fp);
    if(fseek(fp, 0, SEEK_SET) == -1) {
        error("%s: fseek: %s", path, strerror(errno));
    }

    char *buf = calloc(1, size+2);
    fread(buf, size, 1, fp);

    if(size == 0 || buf[size-1] != '\n') {
        buf[size++] = '\n';
    }
    buf[size] = '\0';
    fclose(fp);
    return buf;
}

int main(int argc, char **argv) {
    if(argc != 2) {
        fprintf(stderr, "引数の個数が正しくありません\n");
        return 1;
    }
    
    open_logger();
    user_input = read_file(argv[1]);
    log_print("input: %s\n", user_input);
    tokenize(user_input);
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
