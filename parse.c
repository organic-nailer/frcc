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

bool consume_token(TokenKind kind) {
    if(token->kind != kind) {
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

void expect_token(TokenKind kind) {
    if(token->kind != kind) {
        error_at(token->str, "TK_%dではありません", kind);
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

int is_alnum(char c) {
    return ('a' <= c && c <= 'z')
        || ('A' <= c && c <= 'Z')
        || ('0' <= c && c <= '9')
        || (c == '_');
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
        if(strncmp(p, "return", 6) == 0 && !is_alnum(p[6])) {
            cur = new_token(TK_RETURN, cur, p, 6);
            p += 6;
            continue;
        }
        if(strncmp(p, "if", 2) == 0 && !is_alnum(p[2])) {
            cur = new_token(TK_IF, cur, p, 2);
            p += 2;
            continue;
        }
        if(strncmp(p, "else", 4) == 0 && !is_alnum(p[4])) {
            cur = new_token(TK_ELSE, cur, p, 4);
            p += 4;
            continue;
        }
        if(strncmp(p, "while", 5) == 0 && !is_alnum(p[5])) {
            cur = new_token(TK_WHILE, cur, p, 5);
            p += 5;
            continue;
        }
        if(strncmp(p, "for", 3) == 0 && !is_alnum(p[3])) {
            cur = new_token(TK_FOR, cur, p, 3);
            p += 3;
            continue;
        }
        if('a' <= *p && *p <= 'z') {
            int idLength = 0;
            char* cp = p;
            while('a' <= *cp && *cp <= 'z') {
                idLength++;
                cp++;
            }
            cur = new_token(TK_IDENTITY, cur, p, idLength);
            p += idLength;
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

typedef struct LVar LVar;

struct LVar {
    LVar *next;
    char *name;
    int length;
    int offset;
};

LVar *locals;

LVar* find_lvar(Token* tok) {
    for(LVar* var = locals; var; var = var->next) {
        if(var->length == tok->length && !memcmp(tok->str, var->name, var->length)) {
            return var;
        }
    }
    return NULL;
}

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

void print_node(Node* node, int indent) {
    char tabs[indent+1];
    for(int i = 0; i < indent; i++) {
        tabs[i] = '\t';
    }
    tabs[indent] = NULL;
    if(node) {
        log_print("%s node: %d\n", tabs, node->kind);
        if(node->left) {
            log_print("%s left:\n", tabs);
            print_node(node->left, indent+1);
        }
        if(node->right) {
            log_print("%s right:\n", tabs);
            print_node(node->right, indent+1);
        }
    }
}

Node* code[100];

//program ::= stmt*
Node* stmt(); //::= expression ";" 
                            // | "return" expression ";"
                            // | "if" "(" expression ")" stmt ("else" stmt)?
                            // | "while" "(" expression ")" stmt
                            // | "for" "(" expression? ";" expression? ";" expression? ")" stmt
Node* expression(); //::= assign
Node* assign(); //::= equality ( "=" assign)?
Node* equality(); //::= relational ("==" relational | "!=" relational)*
Node* relational(); //::= relational ::= add ("<" add | "<=" add | ">" add | ">=" add)*
Node* add(); //::= mul ( "+" mul | "-" mul )*
Node* mul(); //::= unary ( "*" unary | "/" unary )*
Node* unary(); //::= ("+" | "-")? primary
Node* primary(); //:: num | identity | "(" expression ")"

void program() {
    locals = calloc(1, sizeof(LVar));
    int i = 0;
    while(!at_eof()) {
        code[i++] = stmt();
    }
    code[i] = NULL;
}

Node* stmt() {
    Node* node;
    if(consume_token(TK_RETURN)) {
        node = calloc(1, sizeof(Node));
        node->kind = ND_RETURN;
        node->left = expression();
        if(!consume(";")) {
            error_at(token->str, "';'ではないトークンです");
        }
    }
    else if(consume_token(TK_IF)) {
        expect("(");
        node = calloc(1, sizeof(Node));
        node->kind = ND_IF_ELSE;
        node->condition = expression();
        expect(")");
        node->then = stmt();
        if(consume_token(TK_ELSE)) {
            node->els = stmt();
        }
    }
    else if(consume_token(TK_WHILE)) {
        expect("(");
        node = calloc(1, sizeof(Node));
        node->kind = ND_WHILE;
        node->condition = expression();
        expect(")");
        node->body = stmt();
    }
    else if(consume_token(TK_FOR)) {
        expect("(");
        node = calloc(1, sizeof(Node));
        node->kind = ND_FOR;
        if(!consume(";")) {
            node->initialize = expression();
            expect(";");
        }
        if(!consume(";")) {
            node->condition = expression();
            expect(";");
        }
        if(!consume(")")) {
            node->increment = expression();
            expect(")");
        }
        node->body = stmt();
    }
    else {
        node = expression();
        if(!consume(";")) {
            error_at(token->str, "';'ではないトークンです");
        }
    }
    return node;
}

Node* expression() {
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
        Node *node = expression();
        expect(")");
        return node;
    }
    Token* tok = consume_identity();
    if(tok) {
        Node* node = calloc(1, sizeof(Node));
        node->kind = ND_LVAR;

        LVar* lvar = find_lvar(tok);
        if(lvar) {
            node->offset = lvar->offset;
        }
        else {
            lvar = calloc(1, sizeof(LVar));
            lvar->next = locals;
            lvar->name = tok->str;
            lvar->length = tok->length;
            lvar->offset = locals->offset+8;
            node->offset = lvar->offset;
            locals = lvar;
        }
        return node;
    }
    // if(tok) {
    //     Node* node = calloc(1, sizeof(Node));
    //     node->kind = ND_LVAR;
    //     node->offset = (tok->str[0] - 'a' + 1) * 8;
    //     return node;
    // }
    return new_node_num(expect_number());
}
