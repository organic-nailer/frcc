#include "frcc.h"

Token *token;

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

bool consume(char* op) {
    if(token->kind != TK_RESERVED
        || strlen(op) != token->length 
        || memcmp(token->str, op, token->length)) {
        return false;
    }
    token = token->next;
    return true;
}

Token* consume_identity() {
    if(token->kind != TK_IDENTITY || 'a' > token->str[0] || token->str[0] > 'z') {
        return NULL;
    }
    Token* ret = token;
    token = token->next;
    return ret;
}

void expect(char* op) {
    if(token->kind != TK_RESERVED
        || strlen(op) != token->length 
        || memcmp(token->str, op, token->length)) {
        error_at(token->str, "'%s'ではありません", op);
    }
    token = token->next;
}

int expect_number() {
    if(token->kind != TK_NUM) {
        error_at(token->str, "数ではありません %d", token->kind);
    }
    int value = token->value;
    token = token->next;
    return value;
}

bool at_eof() {
    return token->kind == TK_EOF;
}

Token *new_token(TokenKind kind, Token *cur, char *str, int length) {
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    tok->length = length;
    cur->next = tok;
    return tok;
}

bool start_with(char* p, char* q) {
    return memcmp(p, q, strlen(q)) == 0;
}

void tokenize(char *p) {
    Token head;
    head.next = NULL;
    Token *cur = &head;

    while(*p) {
        if(isspace(*p)) {
            p++;
            continue;
        }
        if(start_with(p, "<=")||start_with(p, ">=")||start_with(p, "==")||start_with(p, "!=")) {
            cur = new_token(TK_RESERVED, cur, p, 2);
            p += 2;
            continue;
        }
        if(*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')' || *p == '<' || *p == '>' || *p == ';' || *p == '=') {
            cur = new_token(TK_RESERVED, cur, p++, 1);
            continue;
        }
        if('a' <= *p && *p <= 'z') {
            cur = new_token(TK_IDENTITY, cur, p++, 1);
            continue;
        }
        if(isdigit(*p)) {
            cur = new_token(TK_NUM, cur, p, 0); //lengthは使わないので適当に
            cur->value = strtol(p, &p, 10);
            continue;
        }
        error_at(p, "tokenize出来ません");
    }
    new_token(TK_EOF, cur, p, 0);
    token = head.next;
}

//*********構文解析***********

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

Node* code[100];

//program ::= stmt*
Node* stmt(); //::= expr ";"
Node* expr(); //::= assign
Node* assign(); //::= equality ( "=" assign)?
Node* equality(); //::= relational ("==" relational | "!=" relational)*
Node* relational(); //::= relational ::= add ("<" add | "<=" add | ">" add | ">=" add)*
Node* add(); //::= mul ( "+" mul | "-" mul )*
Node* mul(); //::= unary ( "*" unary | "/" unary )*
Node* unary(); //::= ("+" | "-")? primary
Node* primary(); //:: num | identity | "(" expr ")"

void program() {
    int i = 0;
    while(!at_eof()) {
        code[i++] = stmt();
    }
    code[i] = NULL;
}

Node* stmt() {
    Node* node = expr();
    expect(";");
    return node;
}

Node* expr() {
    return assign();
}

Node* assign() {
    Node* node = equality();
    if(consume("=")) {
        node = new_node(ND_ASSIGN, node, assign());
    }
    return node;
}

Node* equality() {
    Node *node = relational();
    for(;;) {
        if(consume("==")) {
            node = new_node(ND_EQ, node, relational());
        }
        else if(consume("!=")) {
            node = new_node(ND_NEQ, node, relational());
        }
        else {
            return node;
        }
    }
}

Node* relational() {
    Node *node = add();
    for(;;) {
        if(consume("<")) {
            node = new_node(ND_LT, node, add());
        }
        else if(consume("<=")) {
            node = new_node(ND_LTE, node, add());
        }
        else if(consume(">")) {
            node = new_node(ND_LT, add(), node);
        }
        else if(consume(">=")) {
            node = new_node(ND_LTE, add(), node);
        }
        else {
            return node;
        }
    }
}

Node *add() {
    Node *node = mul();

    for(;;) {
        if(consume("+")) {
            node = new_node(ND_ADD, node, mul());
        }
        else if(consume("-")) {
            node = new_node(ND_SUB, node, mul());
        }
        else {
            return node;
        }
    }
}

Node *mul() {
    Node *node = unary();
    for(;;) {
        if(consume("*")) {
            node = new_node(ND_MUL, node, unary());
        }
        else if(consume("/")) {
            node = new_node(ND_DIV, node, unary());
        }
        else {
            return node;
        }
    }
}

Node *unary() {
    if(consume("+")) {
        return primary();
    }
    if(consume("-")) {
        return new_node(ND_SUB, new_node_num(0), primary());
    }
    return primary();
}

Node *primary() {
    if(consume("(")) {
        Node *node = expr();
        expect(")");
        return node;
    }
    Token* tok = consume_identity();
    if(tok) {
        Node* node = calloc(1, sizeof(Node));
        node->kind = ND_LVAR;
        node->offset = (tok->str[0] - 'a' + 1) * 8;
        return node;
    }
    return new_node_num(expect_number());
}