#include "frcc.h"

//alternative to strndup
char* duplicate(char *str, size_t len) {
    char *buffer = malloc(len + 1);
    memcpy(buffer, str, len);
    buffer[len] = '\0';

    return buffer;
}

char *format(char *fmt, ...) {
  char buf[2048];
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(buf, sizeof(buf), fmt, ap);
  va_end(ap);
  return duplicate(buf, sizeof(buf));
}

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

Token* consume_literal() {
    if(token->kind != TK_STRING) {
        return NULL;
    }
    Token *ret = token;
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
    char* text = duplicate(str, length);
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = text;
    tok->str_origin = str;
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
        if(*p == '+' || *p == '-' || *p == '*' || *p == '/' 
            || *p == '(' || *p == ')' || *p == '<' || *p == '>' || *p == ';' || *p == '='
            || *p == '{' || *p == '}' || *p == ',' || *p == '&'
            || *p == '[' || *p == ']') {
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
        if(strncmp(p, "int", 3) == 0 && !is_alnum(p[3])) {
            cur = new_token(TK_INT, cur, p, 3);
            p += 3;
            continue;
        }
        if(strncmp(p, "char", 4) == 0 && !is_alnum(p[4])) {
            cur = new_token(TK_CHAR, cur, p, 4);
            p += 4;
            continue;
        }
        if(strncmp(p, "sizeof", 6) == 0 && !is_alnum(p[6])) {
            cur = new_token(TK_SIZEOF, cur, p, 6);
            p += 6;
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
        if(*p == '"') {
            *p++;
            int str_length = 0;
            char *cp = p;
            while(*cp != '"') {
                str_length++;
                cp++;
            }
            cur = new_token(TK_STRING, cur, p, str_length);
            p += str_length;
            p++;
            continue;
        }
        error_at(p, "tokenize出来ません");
    }
    new_token(TK_EOF, cur, p, 0);
    token = head.next;
}

//*********構文解析***********

Map *global_variables;
Map *global_functions;
Vector *literals;

LVar* local;

LVar* find_lvar(Token* tok) {
    for(LVar* var = local; var; var = var->next) {
        if(var->length == tok->length && !memcmp(tok->str, var->name, var->length)) {
            return var;
        }
    }
    return NULL;
}

GVar* find_gvar(Token* tok) {
    if(map_exists(global_variables, tok->str)) {
        return map_get(global_variables, tok->str);
    }
    return NULL;
}

Type* expect_type(TypeKind kind) {
    Type* ret = calloc(1, sizeof(Type));
    Type* tp = calloc(1, sizeof(Type));
    tp->typ = kind;
    ret = tp;
    while(consume("*")) {
        tp = calloc(1, sizeof(Type));
        tp->typ = PTR;
        tp->ptr_to = ret;
        ret = tp;
    }
    return ret;
}

Type* consume_type() {
    if(consume_token(TK_INT)) {
        return expect_type(INT);
    }
    if(consume_token(TK_CHAR)) {
        return expect_type(CHAR);
    }
    return NULL;
}

int type_width(Type *t) {
    if(t->typ == INT) {
        return 4;
    }
    if(t->typ == CHAR) {
        return 1;
    }
    if(t->typ == PTR) {
        return 8;
    }
    if(t->typ == ARRAY) {
        return type_width(t->ptr_to) * t->array_size;
    }
    error_at(token->str_origin, "不明な型です: %d", t->typ);
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
    node->typ = calloc(1, sizeof(Type));
    node->typ->typ = INT;
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
        if(node->typ) {
            log_print("%s type: %d\n", tabs, node->typ->typ);
        }
        if(node->left) {
            log_print("%s left:\n", tabs);
            print_node(node->left, indent+1);
        }
        if(node->right) {
            log_print("%s right:\n", tabs);
            print_node(node->right, indent+1);
        }
        if(node->condition) {
            log_print("%s condition:\n", tabs);
            print_node(node->condition, indent+1);
        }
        if(node->then) {
            log_print("%s then:\n", tabs);
            print_node(node->then, indent+1);
        }
        if(node->els) {
            log_print("%s els:\n", tabs);
            print_node(node->els, indent+1);
        }
        if(node->initialize) {
            log_print("%s initialize:\n", tabs);
            print_node(node->initialize, indent+1);
        }
        if(node->increment) {
            log_print("%s increment:\n", tabs);
            print_node(node->increment, indent+1);
        }
        if(node->body) {
            log_print("%s body:\n", tabs);
            print_node(node->body, indent+1);
        }
        if(node->stmts) {
            log_print("%s stmts:\n", tabs);
            for(int i = 0; i < node->stmts->length; i++) {
                print_node(node->stmts->data[i], indent+1);
            }
        }
        if(node->args) {
            log_print("%s args:\n", tabs);
            for(int i = 0; i < node->args->length; i++) {
                print_node(node->args->data[i], indent+1);
            }
        }
        if(node->offset) {
            log_print("%s offset: %d\n", tabs, node->offset);
        }
    }
}

Function* code[100];

//program ::= function*
//type ::= ("int" | "char") "*"*
Function* function(); //::= 
        //type identity "(" ( type identity, ( "," type identity )* )? ")" "{" stmt* "}"
Node* stmt(); //::= expression ";" 
            // | "{" stmt* "}"
            // | "return" expression ";"
            // | "if" "(" expression ")" stmt ("else" stmt)?
            // | "while" "(" expression ")" stmt
            // | "for" "(" expression? ";" expression? ";" expression? ")" stmt
Node* expression(); //::= assign
Node* assign(); //::= equality ( "=" assign)?
                // | type identity ("[" num "]")?
Node* equality(); //::= relational ("==" relational | "!=" relational)*
Node* relational(); //::= add ("<" add | "<=" add | ">" add | ">=" add)*
Node* add(); //::= mul ( "+" mul | "-" mul )*
Node* mul(); //::= unary ( "*" unary | "/" unary )*
Node* unary(bool u_amp, bool u_sizeof); //::= "+"? postfix
                // | "-"? postfix
                // | "*" unary
                // | "&" unary
                // | "sizeof" unary
Node* postfix(bool u_amp, bool u_sizeof); //::= primary ("[" expression "]")?
Node* primary(bool u_amp, bool u_sizeof); //::= num 
                // | identity ( "(" ( expression ( "," expression )* )? ")" )?
                // | "(" expression ")"
                // | literal

void program() {
    global_variables = new_map();
    global_functions = new_map();
    literals = new_vec();
    int i = 0;
    while(!at_eof()) {
        Type *tp = consume_type();
        Token *tk = consume_identity();
        if(consume("(")) { //関数定義
            Function* func = calloc(1, sizeof(Function));
            func->name = tk->str;
            func->name_length = tk->length;
            func->locals =  calloc(1, sizeof(LVar));
            local = func->locals;
            if(!consume(")")) {
                Type* tp = consume_type();
                int width = 8;
       
                Token* identity = consume_identity();
                LVar* var = calloc(1, sizeof(LVar));
                var->name = identity->str;
                var->length = identity->length;
                var->offset = local->offset + width;
                var->next = local;
                var->typ = tp;
                local = var;
                func->arg_size++;
                while(consume(",")) {
                    tp = consume_type();
                    width = 8;
                    var = calloc(1, sizeof(LVar));
                    Token* t = consume_identity();
                    var->name = t->str;
                    var->length = t->length;
                    var->offset = local->offset + width;
                    var->next = local;
                    var->typ = tp;
                    local = var;
                    func->arg_size++;
                }
                expect(")");
            }
            expect("{");
            Node* node = calloc(1, sizeof(Node));
            node->kind = ND_BLOCK;
            node->stmts = new_vec();
            for(int i = 0; !consume("}"); i++) {
                vec_push(node->stmts, stmt());
            }
            func->node = node;
            func->locals = local;
            map_put(global_functions, func->name, func);
        }
        else { //グローバル変数定義
            int width = 8;
            if(consume("[")) {
                Type* type_array = calloc(1, sizeof(Type));
                type_array->typ = ARRAY;
                type_array->array_size = expect_number();
                type_array->ptr_to = tp;
                tp = type_array;
                width *= type_array->array_size;
                expect("]");
            }
            GVar *gvar = calloc(1, sizeof(GVar));
            gvar->name = tk->str;
            gvar->typ = tp;
            map_put(global_variables, gvar->name, gvar);
            expect(";");
        }
    }
}

// Function* function() {
//     expect_token(TK_INT);
//     Token* tok = consume_identity();
//     if(!tok) error_at(tok->str, "トップレベルは関数が必要です");
//     Function* func = calloc(1, sizeof(Function));
//     func->name = tok->str;
//     func->name_length = tok->length;
//     expect("(");
//     func->locals =  calloc(1, sizeof(LVar));
//     local = func->locals;
//     if(!consume(")")) {
//         expect_token(TK_INT);
//         Type* tp = expect_type();
//         int width = 8;
       
//         Token* identity = consume_identity();
//         LVar* var = calloc(1, sizeof(LVar));
//         var->name = identity->str;
//         var->length = identity->length;
//         var->offset = local->offset + width;
//         var->next = local;
//         var->typ = tp;
//         local = var;
//         func->arg_size++;
//         while(consume(",")) {
//             expect_token(TK_INT);
//             tp = expect_type();
//             width = 8;
//             var = calloc(1, sizeof(LVar));
//             Token* t = consume_identity();
//             var->name = t->str;
//             var->length = t->length;
//             var->offset = local->offset + width;
//             var->next = local;
//             var->typ = tp;
//             local = var;
//             func->arg_size++;
//         }
//         expect(")");
//     }
//     expect("{");
//     Node* node = calloc(1, sizeof(Node));
//     node->kind = ND_BLOCK;
//     node->stmts = new_vec();
//     for(int i = 0; !consume("}"); i++) {
//         vec_push(node->stmts, stmt());
//     }
//     func->node = node;
//     func->locals = local;
//     return func;
// }

Node* stmt() {
    Node* node;
    if(consume("{")) {
        node = calloc(1, sizeof(Node));
        node->kind = ND_BLOCK;
        node->stmts = new_vec();
        for(int i = 0; !consume("}") && i < 100; i++) {
            vec_push(node->stmts, stmt());
        }
    }
    else if(consume_token(TK_RETURN)) {
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
    //print_node(node, 1);
    return node;
}

Node* expression() {
    return assign();
}

Node* assign() {
    Type *tp = consume_type();
    if(tp) {
        Token* tok = consume_identity();
        if(!tok) {
            error_at(token->str, "宣言ミス");
        }
        int width = 8;
        if(consume("[")) {
            Type* type_array = calloc(1, sizeof(Type));
            type_array->typ = ARRAY;
            type_array->array_size = expect_number();
            type_array->ptr_to = tp;
            tp = type_array;
            width *= type_array->array_size;
            expect("]");
        }
        Node* node = calloc(1, sizeof(Node));
        node->kind = ND_VAR_DEF;
        LVar* lvar = calloc(1, sizeof(LVar));
        lvar->next = local;
        lvar->name = tok->str;
        lvar->length = tok->length;
        lvar->offset = local->offset + width;
        lvar->typ = tp;
        node->offset = lvar->offset;
        local = lvar;
        return node;
    }
    Node* node = equality();
    if(consume("=")) {
        node = new_node(ND_ASSIGN, node, assign());
        node->typ = node->left->typ;
    }
    return node;
}

Node* equality() {
    Node *node = relational();
    for(;;) {
        if(consume("==")) {
            node = new_node(ND_EQ, node, relational());
            node->typ = calloc(1, sizeof(Type));
            node->typ = INT;
        }
        else if(consume("!=")) {
            node = new_node(ND_NEQ, node, relational());
            node->typ = calloc(1, sizeof(Type));
            node->typ = INT;
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
            node->typ = calloc(1, sizeof(Type));
            node->typ = INT;
        }
        else if(consume("<=")) {
            node = new_node(ND_LTE, node, add());
            node->typ = calloc(1, sizeof(Type));
            node->typ = INT;
        }
        else if(consume(">")) {
            node = new_node(ND_LT, add(), node);
            node->typ = calloc(1, sizeof(Type));
            node->typ = INT;
        }
        else if(consume(">=")) {
            node = new_node(ND_LTE, add(), node);
            node->typ = calloc(1, sizeof(Type));
            node->typ = INT;
        }
        else {
            return node;
        }
    }
}

Type *parent_type(char op, Type *left, Type *right) {
    if(op == '+') {
        if(left->typ == PTR && right->typ == PTR) {
            error_at(token->str_origin, "ポインタ同士の演算は無効です");
        }
        if(left->typ == PTR) return left;
        if(right->typ == PTR) return right;
        if(left->typ == CHAR && right->typ == CHAR) return left;
        Type *ret_int = calloc(1, sizeof(Type));
        ret_int->typ = INT;
        return ret_int;
    }
    if(op == '-') {
        if(left->typ == PTR && right->typ == PTR) {
            error_at(token->str_origin, "ポインタ同士の演算は無効です");
        }
        if(left->typ == PTR) return left;
        if(right->typ == PTR) {
            error_at(token->str, "マイナスの右辺にポインタ");
        }
        if(left->typ == CHAR && right->typ == CHAR) return left;
        Type *ret_int = calloc(1, sizeof(Type));
        ret_int->typ = INT;
        return ret_int;
    }
    if(op == '*' || op == '/') {
        if(left->typ == PTR || right->typ == PTR) {
            error_at(token->str, "無効な演算");
        }
        if(left->typ == CHAR && right->typ == CHAR) return left;
        Type *ret_int = calloc(1, sizeof(Type));
        ret_int->typ = INT;
        return ret_int;
    }
    if(op == '[') {
        if(left->typ == PTR && right->typ == PTR) {
            error_at(token->str_origin, "ポインタ同士の演算は無効です");
        }
        if(left->typ == PTR) return left;
        if(right->typ == PTR) return right;
        error_at(token->str, "[]内外のどちらかはポインタの必要が");
    }
    error_at(token->str, "不明な演算子 %c", op);
}

Node *add() {
    Node *node = mul();

    for(;;) {
        if(consume("+")) {
            node = new_node(ND_ADD, node, mul());
            node->typ = parent_type('+',node->left->typ, node->right->typ);
        }
        else if(consume("-")) {
            node = new_node(ND_SUB, node, mul());
            node->typ = parent_type('-',node->left->typ,node->right->typ);
        }
        else {
            return node;
        }
    }
}

Node *mul() {
    Node *node = unary(false, false);
    for(;;) {
        if(consume("*")) {
            node = new_node(ND_MUL, node, unary(false, false));
            node->typ = parent_type('*',node->left->typ,node->right->typ);
        }
        else if(consume("/")) {
            node = new_node(ND_DIV, node, unary(false, false));
            node->typ = parent_type('/',node->left->typ,node->right->typ);
        }
        else {
            return node;
        }
    }
}

Node *unary(bool u_amp, bool u_sizeof) {
    if(consume("+")) {
        return postfix(false, false);
    }
    if(consume("-")) {
        Node* node = new_node(ND_SUB, new_node_num(0), postfix(false, false));
        node->typ = calloc(1, sizeof(Type));
        node->typ->typ = INT;
        return node;
    }
    if(consume("*")) {
        Node* node = calloc(1, sizeof(Node));
        node->kind = ND_DEREF;
        node->left = unary(false, false);
        node->typ = node->left->typ->ptr_to;
        return node;
    }
    if(consume("&")) {
        Node* node = calloc(1, sizeof(Node));
        node->kind = ND_ADDR;
        node->left = unary(true, false);
        node->typ = calloc(1, sizeof(Type));
        node->typ->ptr_to = node->left->typ;
        node->typ->typ = PTR;
        return node;
    }
    if(consume_token(TK_SIZEOF)) {
        Node* node = unary(false, true);
        int width = type_width(node->typ);
        return new_node_num(width);
    }
    return postfix(u_amp, u_sizeof);
}

Node *postfix(bool u_amp, bool u_sizeof) {
    Node *left = primary(u_amp, u_sizeof);
    if(consume("[")) {
        Node *node = new_node(ND_ADD, left, expression());
        node->typ = parent_type('[',node->left->typ,node->right->typ);
        Node *node2 = calloc(1, sizeof(Node));
        node2->kind = ND_DEREF;
        node2->left = node;
        node2->typ = node->typ->ptr_to;
        left = node2;
        expect("]");
    }
    return left;
}

Node *primary(bool u_amp, bool u_sizeof) {
    if(consume("(")) {
        Node *node = expression();
        expect(")");
        return node;
    }
    Token* tok = consume_identity();
    if(tok) {
        Node* node = calloc(1, sizeof(Node));
        if(consume("(")) {
            if(!consume(")")) {
                node->args =  new_vec();
                vec_push(node->args, expression());
                while(consume(",")) {
                    vec_push(node->args, expression());
                }
                expect(")");
            }
            node->kind = ND_FUNCTION_CALL;
            node->str = tok->str;
            node->str_length = tok->length;
            return node;
        }
        else {

            LVar* lvar = find_lvar(tok);
            if(lvar) {
                node->kind = ND_LVAR;
                node->offset = lvar->offset;
                if(lvar->typ->typ == ARRAY && !u_amp && !u_sizeof) {
                    node->kind = ND_LVAR_REF;
                    node->typ = calloc(1, sizeof(Type));
                    node->typ->typ = PTR;
                    node->typ->ptr_to = lvar->typ->ptr_to;
                }
                else {
                    node->typ = lvar->typ;
                }
                return node;
            }
            GVar *gvar = find_gvar(tok);
            if(gvar) {
                node->kind = ND_GVAR;
                node->str = tok->str;
                if(gvar->typ->typ == ARRAY && !u_amp && !u_sizeof) {
                    node->kind = ND_GVAR_REF;
                    node->typ = calloc(1, sizeof(Type));
                    node->typ->typ = PTR;
                    node->typ->ptr_to = gvar->typ->ptr_to;
                }
                else {
                    node->typ = gvar->typ;
                }
                return node;
            }
            else {
                error_at(tok->str, "不明な識別子です");
            }
        }
    }
    tok = consume_literal();
    if(tok) {
        int lit_index = vec_index_of(literals, tok->str);
        if(lit_index < 0) {
            vec_push(literals, tok->str);
            lit_index = literals->length - 1;
        }
        Node* node = calloc(1, sizeof(Node));
        node->str = format("LIT%d", lit_index);
        if(!u_amp && !u_sizeof) {
            node->kind = ND_GVAR_REF;
            node->typ = calloc(1, sizeof(Type));
            node->typ->typ = PTR;
            node->typ->ptr_to = calloc(1, sizeof(Type));
            node->typ->ptr_to->typ = CHAR;
        }
        else {
            node->kind = ND_GVAR;
            node->typ = calloc(1, sizeof(Type));
            node->typ->typ = ARRAY;
            node->typ->ptr_to = calloc(1, sizeof(Type));
            node->typ->ptr_to->typ = CHAR;
            node->typ->array_size = tok->length;
        }
        return node;
    }
    return new_node_num(expect_number());
}
