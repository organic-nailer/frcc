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

int labelUnique = 0;

void gen(Node* node) {
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
            gen(node->right);
            printf("pop rdi\n");
            printf("pop rax\n");
            printf("mov [rax], rdi\n");
            printf("push rdi\n");
            return;
        case ND_RETURN:
            gen(node->left);
            printf("pop rax\n");
            printf("mov rsp, rbp\n");
            printf("pop rbp\n");
            printf("ret\n");
            return;
        case ND_IF_ELSE:
            gen(node->condition);
            printf("pop rax\n");
            printf("cmp rax, 0\n");
            if(node->els) {
                printf("je .Lelse%d\n", uniq);
                gen(node->then);
                printf("jmp .Lend%d\n", uniq);
                printf(".Lelse%d:\n", uniq);
                gen(node->els);
                printf(".Lend%d:\n", uniq);
            }
            else {
                printf("je .Lend%d\n", uniq);
                gen(node->then);
                printf(".Lend%d:\n", uniq);
            }
            return;
        case ND_WHILE:
            printf(".Lbegin%d:\n", uniq);
            gen(node->condition);
            printf("pop rax\n");
            printf("cmp rax, 0\n");
            printf("je .Lend%d\n", uniq);
            gen(node->body);
            printf("jmp .Lbegin%d\n", uniq);
            printf(".Lend%d:\n", uniq);
            return;
        case ND_FOR:
            gen(node->initialize);
            printf(".Lbegin%d:\n", uniq);
            gen(node->condition);
            printf("pop rax\n");
            printf("cmp rax, 0\n");
            printf("je .Lend%d\n", uniq);
            gen(node->body);
            gen(node->increment);
            printf("jmp .Lbegin%d\n", uniq);
            printf(".Lend%d:\n", uniq);
            return;
        case ND_BLOCK:
            for(int i = 0; i < node->stmts->length; i++) {
                gen(node->stmts->data[i]);
                printf("pop rax\n");
            }
            printf("push rax\n");
            return;
    }

    gen(node->left);
    gen(node->right);

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
