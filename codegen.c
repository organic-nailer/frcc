#include "frcc.h"

void gen_lval(Node* node) {
    if(node->kind != ND_LVAR) {
        fprintf(stderr, "代入の左辺値が変数ではありません\n");
        exit(1);
    }
    printf("mov rax, rbp\n");
    printf("sub rax, %d\n", node->offset);
    printf("push rax\n");
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
        case ND_LVAR:
            gen_lval(node);
            printf("pop rax\n");
            printf("mov rax, [rax]\n");
            printf("push rax\n");
            return;
        case ND_ASSIGN:
            gen_lval(node->left);
            gen_node(node->right);
            printf("pop rdi\n");
            printf("pop rax\n");
            printf("mov [rax], rdi\n");
            printf("push rdi\n");
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
            printf("pop rax\n");
            printf("mov rax, [rax]\n");
            printf("push rax\n");
            return;
    }

    gen_node(node->left);
    gen_node(node->right);

    printf("pop rdi\n");
    printf("pop rax\n");

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
    printf("push rax\n");
}

void gen(Function* func) {
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
    while(loc) {
        local_length += 8;
        loc = loc->next;
    }
    int offset = fit16(local_length);
    printf("sub rsp, %d\n", offset);
    gen_node(func->node);
    printf("mov rsp, rbp\n");
    printf("pop rbp\n");
    printf("ret\n");
}
