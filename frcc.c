#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>

//トークンの種類
typedef enum {
    TK_RESERVED,
    TK_NUM,
    TK_EOF,
} TokenKind;

typedef struct Token Token;

struct Token {
    TokenKind kind;
    Token *next;
    int value;
    char *str;
};

Token *token;

char *user_input;

void error_at(char *loc, char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    int position = loc - user_input;
    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s", position, " ");
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

bool consume(char op) {
    if(token->kind != TK_RESERVED || token->str[0] != op) {
        return false;
    }
    token = token->next;
    return true;
}

void expect(char op) {
    if(token->kind != TK_RESERVED || token->str[0] != op) {
        error_at(token->str, "'%c'ではありません", op);
    }
    token = token->next;
}

int expect_number() {
    if(token->kind != TK_NUM) {
        error_at(token->str, "数ではありません");
    }
    int value = token->value;
    token = token->next;
    return value;
}

bool at_eof() {
    return token->kind == TK_EOF;
}

Token *new_token(TokenKind kind, Token *cur, char *str) {
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    cur->next = tok;
    return tok;
}

Token *tokenize(char *p) {
    Token head;
    head.next = NULL;
    Token *cur = &head;

    while(*p) {
        if(isspace(*p)) {
            p++;
            continue;
        }
        if(*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')') {
            cur = new_token(TK_RESERVED, cur, p++);
            continue;
        }
        if(isdigit(*p)) {
            cur = new_token(TK_NUM, cur, p);
            cur->value = strtol(p, &p, 10);
            continue;
        }
        error_at(p, "tokenize出来ません");
    }
    new_token(TK_EOF, cur, p);
    return head.next;
}

//*********構文解析***********

typedef enum {
    ND_ADD,
    ND_SUB,
    ND_MUL,
    ND_DIV,
    ND_NUM,
} NodeKind;

typedef struct Node Node;

struct Node {
    NodeKind kind;
    Node *left;
    Node *right;
    int value;
};

Node *new_node(NodeKind kind, Node *left, Node *right) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->left = left;
    node->right = right;
    return node;
}

Node *new_node_num(int value) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_NUM;
    node->value = value;
    return node;
}
Node* expr();
Node* mul();
Node* primary();

//expr ::= mul ( "+" mul | "-" mul )*
Node *expr() {
    Node *node = mul();

    for(;;) {
        if(consume('+')) {
            node = new_node(ND_ADD, node, mul());
        }
        else if(consume('-')) {
            node = new_node(ND_SUB, node, mul());
        }
        else {
            return node;
        }
    }
}

//mul ::= primary ( "*" primary | "/" primary )*
Node *mul() {
    Node *node = primary();
    for(;;) {
        if(consume('*')) {
            node = new_node(ND_MUL, node, primary());
        }
        else if(consume('/')) {
            node = new_node(ND_DIV, node, primary());
        }
        else {
            return node;
        }
    }
}

//primary ::= num | "(" expr ")"
Node *primary() {
    if(consume('(')) {
        Node *node = expr();
        expect(')');
        return node;
    }
    return new_node_num(expect_number());
}

void gen(Node* node) {
    if(node->kind == ND_NUM) {
        printf("push %d\n", node->value);
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
    }
    printf("push rax\n");
}

int main(int argc, char **argv) {
    if(argc != 2) {
        fprintf(stderr, "引数の個数が正しくありません\n");
        return 1;
    }

    user_input = argv[1];
    token = tokenize(argv[1]);
    Node* node = expr();

    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");
    printf("main:\n");
    
    gen(node);

    printf("pop rax\n");
    printf("ret\n");
    return 0;
}
