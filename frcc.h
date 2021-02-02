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
    TK_INT,
    TK_SIZEOF,
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


typedef struct Type Type;

struct Type {
    enum { INT, PTR } typ;
    struct Type *ptr_to;
};

typedef enum {
    ND_ADD, //0
    ND_SUB, //1
    ND_MUL, //2
    ND_DIV, //3
    ND_NUM, //4
    ND_EQ, //5
    ND_NEQ, //6
    ND_LT, //7
    ND_LTE, //8
    ND_ASSIGN, //9
    ND_LVAR, //10
    ND_RETURN,//11
    ND_IF_ELSE,//12
    ND_WHILE,//13
    ND_FOR,//14
    ND_BLOCK,//15
    ND_FUNCTION_CALL,//16
    ND_FUNCTION_IMPL,//17
    ND_ADDR, //18
    ND_DEREF, //19
    ND_VAR_DEF, //20
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
    Vector* args;

    char* str;
    int str_length;
    int value;
    int offset; //変数の格納場所
    Type *typ;
};

void print_node(Node* node, int indent);

void program();

typedef struct LVar LVar;

struct LVar {
    LVar *next;
    char *name;
    int length;
    int offset;
    Type *typ;
};

typedef struct Function Function;

struct Function {
    char* name;
    int name_length;
    Node* node;
    LVar* locals;
    int arg_size;
};

extern Function* code[100];

//**********コード生成***********

void gen(Function* func);
