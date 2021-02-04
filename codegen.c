#include "frcc.h"

void gen_node(Node* node);

//左辺値解釈: 変数のアドレスを積む
void gen_lval(Node* node) {
    if(node->kind == ND_DEREF) { //子の値(つまり参照先のアドレス)を積む
        gen_node(node->left);
        return;
    }
    if(node->kind == ND_LVAR 
    || node->kind == ND_LVAR_REF) { //ローカル変数のアドレスを積む
        printf("mov rax, rbp\n");
        printf("sub rax, %d\n", node->offset);
        printf("push rax\n");
        return;
    }
    if(node->kind == ND_GVAR 
    || node->kind == ND_GVAR_REF) { //グローバル変数のアドレスを積む
        printf("mov rax, offset _%s\n", node->str);
        printf("push rax\n");
        return;
    }
    fprintf(stderr, "代入の左辺値が変数ではありません\n");
    exit(1);
}

void gen_gvar_dec() {
    printf("\n.data\n");
    for(int i = 0; i < global_variables->vals->length; i++) {
        GVar* gvar = global_variables->vals->data[i];
        int width;
        if(gvar->typ->typ == INT) {
            width = 4;
        }
        else if(gvar->typ->typ == PTR) {
            width = 8;
        }
        else if(gvar->typ->typ == ARRAY) {
            int in_width = gvar->typ->ptr_to->typ == INT ? 4 : 8;
            width = in_width * gvar->typ->array_size;
        }
        printf("_%s:\n", gvar->name);
        printf(".zero %d\n", width);
    }
}

int fit16(int x) {
    return (x + 15) & ~15;
}

int labelUnique = 0;

void gen_node(Node* node) {
    int uniq = labelUnique++;
    switch(node->kind) {
        case ND_NUM:
            printf("push %d\n", node->value);
            return;
        case ND_LVAR: //変数の値(ポインタならそのアドレス)を積む
            gen_lval(node);
            if(node->typ->typ == CHAR) {
                printf("pop rax\n");
                printf("movzx rax, BYTE PTR [rax]\n");
                printf("push rax\n");
            }
            else {
                printf("pop rax\n");
                printf("mov rax, [rax]\n");
                printf("push rax\n");
            }
            return;
        case ND_LVAR_REF:
            gen_lval(node);
            return;
        case ND_ASSIGN:
            gen_lval(node->left);
            gen_node(node->right);
            if(node->left->typ->typ == CHAR) {
                printf("pop rdi\n");
                printf("pop rax\n");
                printf("mov [rax], dil\n"); //RDIの8bitレジスタはDIL
                printf("push rdi\n");
            }
            else {
                printf("pop rdi\n");
                printf("pop rax\n");
                printf("mov [rax], rdi\n");
                printf("push rdi\n");
            }
            return;
        case ND_RETURN:
            gen_node(node->left);
            printf("pop rax\n");
            printf("mov rsp, rbp\n");
            printf("pop rbp\n");
            printf("ret\n");
            return;
        case ND_IF_ELSE:
            gen_node(node->condition);
            printf("pop rax\n");
            printf("cmp rax, 0\n");
            if(node->els) {
                printf("je .Lelse%d\n", uniq);
                gen_node(node->then);
                printf("jmp .Lend%d\n", uniq);
                printf(".Lelse%d:\n", uniq);
                gen_node(node->els);
                printf(".Lend%d:\n", uniq);
            }
            else {
                printf("je .Lend%d\n", uniq);
                gen_node(node->then);
                printf(".Lend%d:\n", uniq);
            }
            return;
        case ND_WHILE:
            printf(".Lbegin%d:\n", uniq);
            gen_node(node->condition);
            printf("pop rax\n");
            printf("cmp rax, 0\n");
            printf("je .Lend%d\n", uniq);
            gen_node(node->body);
            printf("jmp .Lbegin%d\n", uniq);
            printf(".Lend%d:\n", uniq);
            return;
        case ND_FOR:
            gen_node(node->initialize);
            printf(".Lbegin%d:\n", uniq);
            gen_node(node->condition);
            printf("pop rax\n");
            printf("cmp rax, 0\n");
            printf("je .Lend%d\n", uniq);
            gen_node(node->body);
            gen_node(node->increment);
            printf("jmp .Lbegin%d\n", uniq);
            printf(".Lend%d:\n", uniq);
            return;
        case ND_BLOCK:
            for(int i = 0; i < node->stmts->length; i++) {
                gen_node(node->stmts->data[i]);
                printf("pop rax\n");
            }
            printf("push rax\n");
            return;
        case ND_FUNCTION_CALL:
            if(node->args && node->args->length) {
                if(node->args->length >= 1) {
                    gen_node(node->args->data[0]);
                    printf("pop rdi\n");
                }
                if(node->args->length >= 2) {
                    gen_node(node->args->data[1]);
                    printf("pop rsi\n");
                }
                if(node->args->length >= 3) {
                    gen_node(node->args->data[2]);
                    printf("pop rdx\n");
                }
                if(node->args->length >= 4) {
                    gen_node(node->args->data[3]);
                    printf("pop rcx\n");
                }
                if(node->args->length >= 5) {
                    gen_node(node->args->data[4]);
                    printf("pop r8\n");
                }
                if(node->args->length >= 6) {
                    gen_node(node->args->data[5]);
                    printf("pop r9\n");
                }
            }
            // printf("push rbp\n");
            // printf("mov rbp, rsp\n");
            printf("mov r10, 16\n"); //RSPを16の倍数に揃える
            printf("not r10\n");
            printf("and rsp, r10\n");
            //printf("sub rsp, 16\n");
            printf("call %.*s\n", node->str_length, node->str);
            printf("push rax\n");
            return;
        case ND_ADDR:
            gen_lval(node->left);
            return;
        case ND_DEREF:
            gen_node(node->left);
            if(node->left->typ->typ == CHAR) {
                printf("pop rax\n");
                printf("movzx rax, BYTE PTR [rax]\n");
                printf("push rax\n");
            }
            else {
                printf("pop rax\n");
                printf("mov rax, [rax]\n");
                printf("push rax\n");
            }
            return;
        case ND_VAR_DEF:
            printf("push rax\n"); //無意味だけどスタックになんか残さないといけないので
            return;
        case ND_GVAR:
            gen_lval(node);
            if(node->typ->typ == CHAR) {
                printf("pop rax\n");
                printf("movzx rax, BYTE PTR [rax]\n");
                printf("push rax\n");
            }
            else {
                printf("pop rax\n");
                printf("mov rax, [rax]\n");
                printf("push rax\n");
            }
            return;
        case ND_GVAR_REF:
            gen_lval(node);
            return;
    }

    gen_node(node->left);
    gen_node(node->right);

    printf("pop rdi\n");
    printf("pop rax\n");

    if((node->left->typ->typ == INT || node->left->typ->typ == CHAR) 
    && (node->right->typ->typ == INT||node->right->typ->typ == CHAR)) {
        switch(node->kind) {
            case ND_ADD:
                printf("add rax, rdi\n");
                break;
            case ND_SUB:
                printf("sub rax, rdi\n");
                break;
            case ND_MUL:
                printf("imul rax, rdi\n");
                break;
            case ND_DIV:
                printf("cqo\n");
                printf("idiv rdi\n");
                break;
            case ND_EQ:
                printf("cmp rax, rdi\n");
                printf("sete al\n");
                printf("movzb rax, al\n");
                break;
            case ND_NEQ:
                printf("cmp rax, rdi\n");
                printf("setne al\n");
                printf("movzb rax, al\n");
                break;
            case ND_LT:
                printf("cmp rax, rdi\n");
                printf("setl al\n");
                printf("movzb rax, al\n");
                break;
            case ND_LTE:
                printf("cmp rax, rdi\n");
                printf("setle al\n");
                printf("movzb rax, al\n");
                break;
        }
    }
    else if((node->left->typ->typ == INT || node->left->typ->typ == CHAR) 
    && node->right->typ->typ == PTR) {
        int width = node->right->typ->ptr_to->typ == INT ? 4 : 8;
        if(node->kind == ND_ADD) {
            printf("imul rax, %d\n", width);
            printf("add rax, rdi\n");
        }
        else {
            fprintf(stderr, "無効な演算 nd:%d\n", node->kind);
            exit(1);
        }
    }
    else if(node->left->typ->typ == PTR 
    && (node->right->typ->typ == INT || node->right->typ->typ == CHAR)) {
        int width = node->left->typ->ptr_to->typ == INT ? 4 : 8;
        if(node->kind == ND_ADD) {
            printf("imul rdi, %d\n", width);
            printf("add rax, rdi\n");
        }
        else if(node->kind == ND_SUB) {
            printf("imul rdi, %d\n", width);
            printf("sub rax, rdi\n");
        }
        else {
            fprintf(stderr, "無効な演算 nd:%d\n", node->kind);
            exit(1);
        }
    }
    else {
        fprintf(stderr, "ポインタ同士の演算\n");
        exit(1);
    }
    printf("push rax\n");
}

void gen_function(Function* func) {
    printf("%.*s:\n", func->name_length, func->name);
    printf("push rbp\n");
    printf("mov rbp, rsp\n");
    if(func->arg_size >= 1) {
        printf("push rdi\n");
    }
    if(func->arg_size >= 2) {
        printf("push rsi\n");
    }
    if(func->arg_size >= 3) {
        printf("push rdx\n");
    }
    if(func->arg_size >= 4) {
        printf("push rcx\n");
    }
    if(func->arg_size >= 5) {
        printf("push r8\n");
    }
    if(func->arg_size >= 6) {
        printf("push r9\n");
    }
    int local_length = 0;
    LVar* loc = func->locals;
    local_length = loc->offset;
    // while(loc) {
    //     local_length = loc->offset;
    //     loc = loc->next;
    // }
    int offset = fit16(local_length);
    printf("sub rsp, %d\n", offset);
    gen_node(func->node);
    printf("mov rsp, rbp\n");
    printf("pop rbp\n");
    printf("ret\n");
}

void gen() {
    gen_gvar_dec();
    
    printf("\n.text\n");
    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");
    for(int i = 0; i < global_functions->vals->length; i++) {
        Function *func = global_functions->vals->data[i];
        gen_function(func);
        printf("pop rax\n");
    }
}
