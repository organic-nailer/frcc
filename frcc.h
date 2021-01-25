#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>

#include "vector.h"

//**********LOG***********

void open_logger();
void close_logger();
void log_print(char* fmt, ...);

//**********字句解析*******

//トークンの種類
typedef enum {
    TK_RESERVED,
    TK_IDENTITY,
    TK_NUM,
    TK_RETURN,
    TK_IF,
    TK_ELSE,
    TK_WHILE,
    TK_FOR,
    TK_EOF,
} TokenKind;

typedef struct Token Token;

struct Token {
    TokenKind kind;
    Token *next;
    int value;
    char *str;
    int length;
};

extern Token *token;

extern char *user_input;

void tokenize(char *p);

//*********構文解析***********

typedef enum {
    ND_ADD,
    ND_SUB,
    ND_MUL,
    ND_DIV,
    ND_NUM,
    ND_EQ,
    ND_NEQ,
    ND_LT,
    ND_LTE,
    ND_ASSIGN,
    ND_LVAR,
    ND_RETURN,
    ND_IF_ELSE,
    ND_WHILE,
    ND_FOR,
    ND_BLOCK,
} NodeKind;

typedef struct Node Node;

struct Node {
    NodeKind kind;
    Node *left;
    Node *right;
    Node *condition;
    Node *then;
    Node *els;
    Node *initialize;
    Node *increment;
    Node *body;
    Vector* stmts;

    int value;
    int offset; //変数の格納場所
};

void print_node(Node* node, int indent);

extern Node* code[100];
void program();

//**********コード生成***********

void gen(Node* node);
