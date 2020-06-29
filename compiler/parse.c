#include "main.h"

#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char symbols[][3] = {
    "==", "!=",
    // notice the check order
    "<=", "<", ">=", ">",
    "+", "-",
    "*", "/",
    "(", ")",
    ";", "=",
    "{", "}",
    ",", "&",
    "[", "]",
};

char keywords[][8] = {
    "return", "if", "else",
    "for", "while", "int",
    "sizeof", "char",
};

int next_label = 0;

typedef enum {
    // Reserved symbols
    TK_RESERVED,
    // Reserved keywords such as "return", "if"
    TK_KEYWORD,
    // Identifiers (variables)
    TK_IDENTIFIER,
    // Number
    TK_NUM,
    // String literal
    TK_STRING,
    // End of the tokens
    TK_EOF,
} TokenKind;

typedef struct Token Token;

struct Token {
    TokenKind kind;
    Token *next;
    // Value if kind == TK_NUM
    int val;
    // Token
    char *str;
    // Token length
    int len;
};

// Current function's local variables
// elements: LocalVar*
Vector *locals;

// Global variables
// elements: GlobalVar*
Vector *globals;

// Current token
Token *token;

// Each statements
Vector *code;

// String literals
Vector *strings;

// consume returns true when the current token is the given expected operator, and proceeds to the next token.
// Returns false otherwise.
bool consume(char *op) {
        // next token is not a symbol
    if (token->kind != TK_RESERVED ||
        // not correct expected token length
        strlen(op) != token->len ||
        // compare the first token->len bytes of two strings
        memcmp(token->str, op, token->len)) {
        return false;
    }
    token = token->next;
    return true;
}

// expect proceeds to the next token when the current token is the given expected operator.
// Reports error otherwise.
void expect(char *op) {
    if (!consume(op)) {
        error_at(token->str, "Next token is not \"%s\"\n", op);
    }
}

// consume_keyword returns true when the current token is TK_KEYWORD, and proceeds to the next token.
bool consume_keyword(char *keyword) {
    if (token->kind != TK_KEYWORD ||
        strlen(keyword) != token->len ||
        memcmp(keyword, token->str, token->len)) {
        return false;
    }
    token = token->next;
    return true;
}

// expect_keyword consumes keyword and proceeds to the next token. If the expected keyword was not found, raises an error.
void expect_keyword(char *keyword) {
    if (!consume_keyword(keyword)) {
        error_at(token->str, "Expected %s", keyword);
    }
}

// consume_identifier consumes the next identifier token if exists, and proceed to the next token.
Token *consume_identifier() {
    if (token->kind != TK_IDENTIFIER) {
        return NULL;
    }
    Token *ret = token;
    token = token->next;
    return ret;
}

// consume_identifier returns the next identifier token, otherwise reports an error.
Token *expect_identifier() {
    Token *ret = consume_identifier();
    if (ret == NULL) {
        error_at(token->str, "Next token is not an identifier\n");
    }
    return ret;
}

// consume_string returns the next string literal and proceeds to the next token, if found.
Token *consume_string() {
    if (token->kind != TK_STRING) {
        return NULL;
    }
    Token *ret = token;
    token = token->next;
    return ret;
}

// consume_number returns pointer to the consumed number and proceed to the next token.
// Returns NULL otherwise.
int *consume_number() {
    if (token->kind != TK_NUM) {
        return NULL;
    }
    int *ret = calloc(1, sizeof(int));
    *ret = token->val;
    token = token->next;
    return ret;
}

// expect_number returns number and proceed to the next token when the current token is represents a number.
// Reports error otherwise.
int expect_number() {
    if (token->kind != TK_NUM) {
        error_at(token->str, "Not a number\n");
    }
    int val = token->val;
    token = token->next;
    return val;
}

// at_eof returns true if the current token is the last token.
bool at_eof() {
    return token->kind == TK_EOF;
}

// new_token generates a new token and links it to the given current token.
Token *new_token(TokenKind kind, Token *cur, char *str) {
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    cur->next = tok;
    return tok;
}

// is_variable_char returns true if the given character is a valid first character for a variable.
bool is_variable_char(char c) {
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '_';
}

// is_alnum returns true if the given character is an alphabet, number, or underscore.
bool is_alnum(char c) {
  return ('a' <= c && c <= 'z') ||
         ('A' <= c && c <= 'Z') ||
         ('0' <= c && c <= '9') ||
         (c == '_');
}

// has_char_length checks if the given string length is greater than or equal to the given length.
bool has_char_length(char *p, size_t len) {
    while (*p && len > 0) {
        len--;
        p += 1;
    }
    return len == 0;
}

// tokenize_next tokenizes the next characters.
Token *tokenize_next(char **p, Token *cur) {
    // Skip space characters
    if (isspace(**p)) {
        *p += 1;
        return NULL;
    }

    // Skip line comment
    if (strncmp(*p, "//", 2) == 0) {
        *p += 2;
        while (**p != '\n') {
            *p += 1;
        }
        *p += 1;
        return NULL;
    }

    // Skip block comment
    if (strncmp(*p, "/*", 2) == 0) {
        char *q = strstr(*p + 2, "*/");
        if (!q) {
            error_at(*p, "couldn't find comment closer");
        }
        *p = q + 2;
        return NULL;
    }

    // Check for symbols
    for (size_t i = 0; i < sizeof(symbols) / sizeof(symbols[0]); i++) {
        if (memcmp(symbols[i], *p, strlen(symbols[i])) == 0) {
            Token *next = new_token(TK_RESERVED, cur, *p);
            next->len = strlen(symbols[i]);
            // proceed the pointer
            *p += strlen(symbols[i]);
            return next;
        }
    }

    // Check for reserved keywords
    for (size_t i = 0; i < sizeof(keywords) / sizeof(keywords[0]); i++) {
        size_t len = strlen(keywords[i]);
        if (has_char_length(*p, len) &&
            memcmp(*p, keywords[i], len) == 0 &&
            !is_alnum((*p)[len])) {
            Token *next = new_token(TK_KEYWORD, cur, *p);
            next->len = len;
            *p += len;
            return next;
        }
    }

    // Check for identifiers (local variables)
    if (is_variable_char(**p)) {
        int len = 0;
        Token *next = new_token(TK_IDENTIFIER, cur, *p);
        while (is_alnum(**p)) {
            len++;
            *p += 1;
        }
        next->len = len;
        return next;
    }

    // Check for number
    if (isdigit(**p)) {
        Token *next = new_token(TK_NUM, cur, *p);
        next->val = strtol(*p, p, 10);
        return next;
    }

    // Check for string literal (quoted in double quote " character)
    if (**p == '"') {
        *p += 1;
        Token *literal = new_token(TK_STRING, cur, *p);
        int len = 0;
        while (**p != '"') {
            if (**p == '\0') {
                error_at(literal->str, "unexpected EOF in string literal");
            }
            len++;
            *p += 1;
        }
        // consume the last double quote " character
        *p += 1;
        literal->len = len;
        return literal;
    }

    error_at(*p, "Cannot tokenize");
}

// tokenize tokenizes the given character sequence and returns it.
Token *tokenize(char *p) {
    Token head;
    head.next = NULL;
    Token *cur = &head;

    // While the next character is not a null character
    while (*p) {
        Token *next = tokenize_next(&p, cur);
        if (next) {
            cur = next;
        }
    }

    new_token(TK_EOF, cur, p);
    return head.next;
}

/**
Program syntax in EBNF
program    = (var ";" | var "=" expr ";" | func)*
ptr_type   = "int" | "char" | type "*"
var        = ptr_type ident ("[" num "]")*
func       = ptr_type ident "(" (var ("," var)*)? ")" "{" stmt* "}"
stmt       = expr ";"
            | var ";"
            | "{" stmt* "}"
            | "if" "(" expr ")" stmt ("else" stmt)?
            | "while" "(" expr ")" stmt
            | "for" "(" expr? ";" expr? ";" expr? ")" stmt
            | "return" expr ";"
init       = "{" (expr ("," expr)*)? "}" | expr
expr       = assign
assign     = equality ("=" assign)?
equality   = relational ("==" relational | "!=" relational)*
relational = add ("<" add | "<=" add | ">" add | ">=" add)*
add        = mul ("+" mul | "-" mul)*
mul        = unary ("*" unary | "/" unary)*
unary      = "sizeof" unary
            | ("+" | "-")? primary
            | "*" unary
            | "&" unary
primary    = num
            | ident
            | string_literal
            | ident "(" (expr ("," expr)*)? ")"
            | ident "[" expr "]"
            | "(" expr ")"
*/

// new_node creates a new AST node according to the given right and left children.
Node *new_node(NodeKind kind, Node *left, Node *right) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->left = left;
    node->right = right;
    return node;
}

// new_node_num creates a new leaf number node for AST.
Node *new_node_num(int val) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_NUM;
    node->val = val;
    return node;
}

// find_local_var returns local var struct if name in the given token has been used before; returns NULL otherwise.
LocalVar *find_local_var(Token *tok) {
    for (int i = 0; i < vector_count(locals); i++) {
        LocalVar *var = (LocalVar*) vector_get(locals, i);
        if (var->len == tok->len && memcmp(var->name, tok->str, var->len) == 0) {
            return var;
        }
    }
    return NULL;
}

// find_global_var returns global var if the global var has already been declared; returns NULL otherwise.
GlobalVar *find_global_var(Token *tok) {
    for (int i = 0; i < vector_count(globals); i++) {
        GlobalVar *var = (GlobalVar*) vector_get(globals, i);
        if (var->len == tok->len && memcmp(var->name, tok->str, var->len) == 0) {
            return var;
        }
    }
    return NULL;
}

// new_local_var returns a local variable as node.
// Always generates a new local variable and appends it to the current local variables.
Node *new_local_var(Token *tok, Type *ty) {
    LocalVar *var = calloc(1, sizeof(LocalVar));
    var->name = tok->str;
    var->len = tok->len;
    if (vector_count(locals) == 0) {
        var->offset = size_of(ty);
    } else {
        LocalVar *last_var = (LocalVar*) vector_get_last(locals);
        var->offset = last_var->offset + size_of(ty);
    }
    var->type = ty;
    vector_add(locals, var);

    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_LOCAL_VAR;
    node->offset = var->offset;
    node->type = ty;
    return node;
}

// new_type constructs struct type with the given type kind.
Type *new_type(TypeKind kind) {
    Type *ty = calloc(1, sizeof(Type));
    ty->ty = kind;
    return ty;
}

// type_of returns the type this node represents.
Type *type_of(Node *node) {
    if (node->type) {
        return node->type;
    }

    switch (node->kind) {
        case ND_ASSIGN:
            return type_of(node->left);
        case ND_EQUAL:
        case ND_NOT_EQUAL:
        case ND_LESS_EQUAL:
        case ND_LESS:
        case ND_GREATER_EQUAL:
        case ND_GREATER:
            return new_type(INT);
        case ND_ADD:
        case ND_SUB:
        case ND_MUL:
        case ND_DIV:
            return type_of(node->left);
        case ND_ADDR: ;
            Type *to = type_of(node->left);
            Type *ty = new_type(PTR);
            ty->ptr_to = to;
            return ty;
        case ND_DEREF: ;
            ty = type_of(node->left);
            // implicit conversion of array to pointer
            if (ty->ty != PTR && ty->ty != ARRAY) {
                error_at(node->str, "Dereference not to a pointer or an array");
            }
            return ty->ptr_to;
        case ND_FUNC_CALL:
            for (int i = 0; i < vector_count(code); i++) {
                Node *stmt = (Node*) vector_get(code, i);
                if (memcmp(stmt->str, node->str, node->len) == 0) {
                    return stmt->type;
                }
            }
            error_at(node->str, "Unknown function");
        case ND_GLOBAL_VAR:
        case ND_LOCAL_VAR:
            if (node->type == NULL) {
                error_at(node->str, "Local variable of unknown type");
            }
            return node->type;
        case ND_NUM:
            return new_type(INT);
        case ND_STRING:
            ty = new_type(ARRAY);
            ty->ptr_to = new_type(CHAR);
            // +1: null sequence
            ty->array_size = node->len + 1;
            return ty;
    }

    error_at(node->str, "unknown type at %.*s", node->len, node->str);
}

// type_equals deeply checks the type equality.
bool type_equals(Type *first, Type *second) {
    if (first->ty != second->ty) {
        return false;
    }
    switch (first->ty) {
    case CHAR:
    case INT:
        return true;
    case PTR:
    case ARRAY:
        return type_equals(first->ptr_to, second->ptr_to);
    }
    error("type equals not implemented");
}

// size_of returns the size of the given type in bytes.
size_t size_of(Type *ty) {
    switch (ty->ty) {
    case CHAR:
        return 1;
    case INT:
        return 4;
    case PTR:
        return 8;
    case ARRAY:
        if (ty->array_size == -1) {
            error("array size is undefined");
        }
        return ty->array_size * size_of(ty->ptr_to);
    }
}

// Helper function for eval_global_init
// Checks if the given nodes (ND_NUM, ND_GLOBAL_VAR, ND_ADD or ND_SUB) are equal.
bool init_node_equals(Node *first, Node *second) {
    if (first->kind != second->kind) return false;

    switch (first->kind) {
    case ND_NUM:
        return first->val == second->val;
    case ND_GLOBAL_VAR:
        return first->len == second->len
            && strncmp(first->str, second->str, first->len) == 0
            && first->offset == second->offset;
    case ND_ADD:
    case ND_SUB:
        return init_node_equals(first->left, second->left) && init_node_equals(first->right, second->right);
    }

    error_at(first->str, "expected ND_NUM, ND_GLOBAL_VAR, ND_ADD or ND_SUB but got %d", first->kind);
}

// Helper function for eval_global_init
// Returns the number of the node with ND_NUM. If the kind of the node is not ND_NUM, throws an error.
int eval_number(Node *num) {
    if (num->kind != ND_NUM) {
        error_at(num->str, "expected number");
    }
    return num->val;
}

// Helper function for eval_global_init
// Returns the global variable inside this node.
Node *retrieve_global_var(Node *node) {
    switch (node->kind) {
    case ND_GLOBAL_VAR:
        return node;
    case ND_ADD:
    case ND_SUB:
        if (node->left->kind == ND_GLOBAL_VAR || node->left->kind == ND_ADD || node->left->kind == ND_SUB) {
            return retrieve_global_var(node->left);
        } else if (node->right->kind == ND_GLOBAL_VAR || node->right->kind == ND_ADD || node->right->kind == ND_SUB) {
            return retrieve_global_var(node->right);
        }
    }
    error_at(node->str, "expected global variable inside this node, but not found");
}

// Helper function for eval_global_init
// Returns the size of type which this pointer (or array) points to, if this node includes a pointer or array variable.
int size_of_ptr(Node *node) {
    Node *global_var = retrieve_global_var(node);

    Type *type = node->type;
    if ((type->ty == PTR || type->ty == ARRAY) && type->ptr_to) {
        return size_of(type->ptr_to);
    }
    return 1;
}

// Evaluates global variable initializer.
// Returns node of one of: ND_NUM, ND_GLOBAL_VAR (address of other global variable), ND_STRING, ND_ARRAY,
// ND_ADD/ND_SUB (with left node of ND_GLOBAL_VAR (address) and right node of ND_NUM (already multiplied by ptr size))
Node *eval_global_init(Node *node) {
    switch (node->kind) {
    case ND_ASSIGN: // =
        error_at(node->str, "assigning at initializer not allowed");
    case ND_EQUAL: // == // NOTE: equality comparison of only numbers
        return new_node_num(init_node_equals(eval_global_init(node->left), eval_global_init(node->right)));
    case ND_NOT_EQUAL: // !=
        return new_node_num(!init_node_equals(eval_global_init(node->left), eval_global_init(node->right)));
    case ND_LESS_EQUAL: // <=
        return new_node_num(eval_number(eval_global_init(node->left)) <= eval_number(eval_global_init(node->right)));
    case ND_LESS: // <
        return new_node_num(eval_number(eval_global_init(node->left)) < eval_number(eval_global_init(node->right)));
    case ND_GREATER_EQUAL: // >=
        return new_node_num(eval_number(eval_global_init(node->left)) >= eval_number(eval_global_init(node->right)));
    case ND_GREATER: // >
        return new_node_num(eval_number(eval_global_init(node->left)) > eval_number(eval_global_init(node->right)));
    case ND_ADD: // +
    case ND_SUB: // -
        ;
        Node *first = eval_global_init(node->left);
        Node *second = eval_global_init(node->right);
        if (first->kind == ND_NUM && second->kind == ND_NUM) {
            if (node->kind == ND_ADD) {
                return new_node_num(eval_number(first) + eval_number(second));
            } else {
                return new_node_num(eval_number(first) - eval_number(second));
            }
        } else if (first->kind == ND_GLOBAL_VAR && second->kind == ND_NUM) {
            node->left = first;
            node->right = second;
            return node;
        } else if (first->kind == ND_NUM && second->kind == ND_GLOBAL_VAR) {
            node->left = second;
            node->right = first;
            return node;
        } else if ((first->kind == ND_ADD || first->kind == ND_SUB) && second->kind == ND_NUM) {
            node->left = retrieve_global_var(first);
            if (node->kind == ND_ADD) {
                node->right = new_node_num(eval_number(first->right) + eval_number(second) * size_of_ptr(first));
            } else {
                node->right = new_node_num(eval_number(first->right) - eval_number(second) * size_of_ptr(first));
            }
            return node;
        } else if (first->kind == ND_NUM && (second->kind == ND_ADD || second->kind == ND_SUB)) {
            node->left = retrieve_global_var(second);
            if (node->kind == ND_ADD) {
                node->right = new_node_num(eval_number(second->right) + eval_number(first) * size_of_ptr(second));
            } else {
                node->right = new_node_num(eval_number(second->right) - eval_number(first) * size_of_ptr(second));
            }
            return node;
        } else {
            error_at(node->str, "unsupported operand types");
        }
    case ND_MUL: // *
        first = eval_global_init(node->left);
        second = eval_global_init(node->right);
        return new_node_num(eval_number(first) * eval_number(second));
    case ND_DIV: // /
        first = eval_global_init(node->left);
        second = eval_global_init(node->right);
        return new_node_num(eval_number(first) / eval_number(second));
    case ND_ADDR: // &
        if (node->left->kind != ND_GLOBAL_VAR) {
            error_at(node->left->str, "expected global variable");
        }
        return node->left;
    case ND_FUNC: // function
        // TODO: support taking address of functions
        error_at(node->str, "taking address of functions is not supported");
    case ND_GLOBAL_VAR: // Global variable
        if (node->type->ty == PTR || node->type->ty == ARRAY) {
            return node;
        }
        error_at(node->str, "initializer element is not constant");
    case ND_STRING: // String literal
        return node;
    case ND_NUM: // Number
        return node;
    case ND_ARRAY:
        // validate
        if (!node->type || node->type->ty != ARRAY) {
            error_at(node->str, "expected array type");
        }
        for (int i = 0; i < vector_count(node->arguments); i++) {
            Node *elt = (Node*) vector_get(node->arguments, i);
            if (!type_equals(node->type->ptr_to, type_of(elt))) {
                error_at(elt->str, "unmatched type in array initializer element");
            }
            vector_set(node->arguments, i, eval_global_init(elt));
        }
        return node;
    }

    error_at(node->str, "invalid initializer");
}

Node *expr();

// primary parses the next 'primary' (in EBNF) as AST.
Node *primary() {
    // If the next token is opening parenthesis, expect 'expr' inside.
    if (consume("(")) {
        Node *node = expr();
        expect(")");
        return node;
    }

    // String literal
    Token *tok = consume_string();
    if (tok) {
        Node *node = calloc(1, sizeof(Node));
        node->kind = ND_STRING;
        node->str = tok->str;
        node->len = tok->len;
        node->label = vector_count(strings);
        vector_add(strings, node);
        return node;
    }

    // If the next token is identifier (local variable), consume it.
    tok = consume_identifier();
    if (tok) {
        // Function call
        if (consume("(")) {
            Node *node = calloc(1, sizeof(Node));
            node->kind = ND_FUNC_CALL;
            node->str = tok->str;
            node->len = tok->len;

            Vector *arguments = new_vector();
            while (!consume(")")) {
                vector_add(arguments, expr());
                if (!consume(",")) {
                    expect(")");
                    break;
                }
            }
            node->arguments = arguments;
            return node;
        }

        // Local variable or global variable
        Node *node = calloc(1, sizeof(Node));
        LocalVar *var = find_local_var(tok);
        if (var) {
            node->kind = ND_LOCAL_VAR;
            node->offset = var->offset;
            node->type = var->type;
            node->str = var->name;
            node->len = var->len;
        } else {
            GlobalVar *var = find_global_var(tok);
            // If the variable has not been declared, raise an error
            if (!var) {
                error_at(tok->str, "Variable %.*s has not been declared", tok->len, tok->str);
            }

            node->kind = ND_GLOBAL_VAR;
            node->offset = var->offset;
            node->type = var->type;
            node->str = var->name;
            node->len = var->len;
        }

        if (consume("[")) {
            // parse "a[b]" syntax (array indexing) as "*(a + b)"
            Node *parent = calloc(1, sizeof(Node));
            Node *right = expr();
            expect("]");

            parent->kind = ND_DEREF;
            Node *adder = calloc(1, sizeof(Node));
            adder->kind = ND_ADD;
            adder->left = node;
            adder->right = right;
            parent->left = adder;
            return parent;
        }
        return node;
    }

    // Otherwise expect a number.
    return new_node_num(expect_number());
}

// unary parses the next 'unary' (in EBNF) as AST.
Node *unary() {
    if (consume_keyword("sizeof")) {
        Node *node = unary();
        return new_node_num(size_of(type_of(node)));
    } else if (consume("+")) {
        return primary();
    } else if (consume("-")) {
        return new_node(ND_SUB, new_node_num(0), primary());
    } else if (consume("*")) {
        Node *node = calloc(1, sizeof(Node));
        node->kind = ND_DEREF;
        node->left = unary();
        return node;
    } else if (consume("&")) {
        Node *node = calloc(1, sizeof(Node));
        node->kind = ND_ADDR;
        node->left = unary();
        return node;
    } else {
        return primary();
    }
}

// mul parses the next 'mul' (in EBNF) as AST.
Node *mul() {
    Node *node = unary();
    for (;;) {
        if (consume("*")) {
            node = new_node(ND_MUL, node, unary());
        } else if (consume("/")) {
            node = new_node(ND_DIV, node, unary());
        } else {
            return node;
        }
    }
}

// add parses the next 'add' (in EBNF) as AST.
Node *add() {
    Node *node = mul();
    for (;;) {
        if (consume("+")) {
            node = new_node(ND_ADD, node, mul());
        } else if (consume("-")) {
            node = new_node(ND_SUB, node, mul());
        } else {
            return node;
        }
    }
}

// relational parses the next 'relational' (in EBNF) as AST.
Node *relational() {
    Node *node = add();
    for (;;) {
        if (consume("<=")) {
            node = new_node(ND_LESS_EQUAL, node, add());
        } else if (consume("<")) {
            node = new_node(ND_LESS, node, add());
        } else if (consume(">=")) {
            node = new_node(ND_GREATER_EQUAL, node, add());
        } else if (consume(">")) {
            node = new_node(ND_GREATER, node, add());
        } else {
            return node;
        }
    }
}

// equality parses the next 'equality' (in EBNF) as AST.
Node *equality() {
    Node *node = relational();
    for (;;) {
        if (consume("==")) {
            node = new_node(ND_EQUAL, node, relational());
        } else if (consume("!=")) {
            node = new_node(ND_NOT_EQUAL, node, relational());
        } else {
            return node;
        }
    }
}

// assign parses the next 'assign' (in EBNF) as AST.
Node *assign() {
    Node *node = equality();
    if (consume("=")) {
        node = new_node(ND_ASSIGN, node, assign());
    }
    return node;
}

// expr parses the next 'expr' (in EBNF) as AST.
Node *expr() {
    return assign();
}

// base_type consumes the next token and returns the base type if found.
// Returns NULL otherwise.
Type *base_type() {
    if (consume_keyword("int")) {
        return new_type(INT);
    } else if (consume_keyword("char")) {
        return new_type(CHAR);
    }
    return NULL;
}

// ptr_type parses the (pointer part of the) type.
Type *ptr_type(TypeKind base) {
    if (consume("*")) {
        Type *ty = calloc(1, sizeof(Type));
        ty->ty = PTR;
        ty->ptr_to = ptr_type(base);
        return ty;
    }
    Type *ty = calloc(1, sizeof(Type));
    ty->ty = base;
    return ty;
}

// array_type parses the (array part of the) type.
Type *array_type(Type *base) {
     if (consume("[")) {
        int *num = consume_number();
        int size = -1;
        if (num) {
            size = *num;
            if (size < 0) {
                error_at(token->str, "Array size must not be negative");
            }
        }
        expect("]");
        Type *ty = calloc(1, sizeof(Type));
        ty->ty = ARRAY;
        ty->array_size = size;
        ty->ptr_to = array_type(base);
        return ty;
    }
    return base;
}

// var parses the next 'var' (in EBNF) as AST.
Node *var(TypeKind base) {
    Type *ptr_ty = ptr_type(base);
    Token *tok = expect_identifier();
    Type *ty = array_type(ptr_ty);
    return new_local_var(tok, ty);
}

// stmt parses the next 'stmt' (in EBNF) as AST.
Node *stmt() {
    Node *node;

    Type *base_ty = base_type();
    if (base_ty) {
        // local variable declaration
        node = var(base_ty->ty);
        expect(";");
        return node;
    } else if (consume_keyword("return")) {
        node = calloc(1, sizeof(Node));
        node->kind = ND_RETURN;
        node->left = expr();
        expect(";");
    } else if (consume_keyword("if")) {
        expect("(");
        Node *cond = expr();
        expect(")");
        Node *inside = stmt();

        node = calloc(1, sizeof(Node));
        node->kind = ND_IF;
        node->left = cond;
        node->right = inside;
        // Label: end
        node->label = next_label;
        next_label++;

        if (consume_keyword("else")) {
            Node *els = stmt();
            node->third = els;
            // Label: else
            next_label++;
        }
    } else if (consume_keyword("while")) {
        expect("(");
        Node *cond = expr();
        expect(")");
        Node *inside = stmt();

        node = calloc(1, sizeof(Node));
        node->kind = ND_WHILE;
        node->left = cond;
        node->right = inside;
        // Labels: begin, end
        node->label = next_label;
        next_label += 2;
    } else if (consume_keyword("for")) {
        expect("(");

        Node *init;
        Node *cond;
        Node *cont;
        Node *inside;

        if (consume(";")) {
            init = NULL;
        } else {
            init = expr();
            expect(";");
        }
        if (consume(";")) {
            cond = NULL;
        } else {
            cond = expr();
            expect(";");
        }
        if (consume(")")) {
            cont = NULL;
        } else {
            cont = expr();
            expect(")");
        }
        inside = stmt();

        node = calloc(1, sizeof(Node));
        node->kind = ND_FOR;
        node->left = init;
        node->right = cond;
        node->third = cont;
        node->fourth = inside;
        // Labels: begin, end
        node->label = next_label;
        next_label += 2;
    } else if (consume("{")) {
        node = calloc(1, sizeof(Node));
        node->kind = ND_BLOCK;
        Node *cur = node;

        while (!consume("}")) {
            cur->left = stmt();
            Node *next = calloc(1, sizeof(Node));
            next->kind = ND_BLOCK;
            cur->right = next;
            cur = next;
        }
    } else {
        node = expr();
        expect(";");
    }

    return node;
}

// func parses the next 'func' (in EBNF) as AST.
Node *func(Type *return_type, Token *name) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_FUNC;
    node->type = return_type;
    node->str = name->str;
    node->len = name->len;

    // current function's local variables, for new_local_var function to append to this
    locals = new_vector();

    // read function arguments
    Vector *arguments = new_vector();
    while (!consume(")")) {
        Type *base_ty = base_type();
        if (base_ty == NULL) {
            error_at(token->str, "Base type expected, but got %.*s", token->len, token->str);
        }
        // treat each function argument as a local variable
        Node *local_var = var(base_ty->ty);
        vector_add(arguments, local_var);

        if (!consume(",")) {
            expect(")");
            break;
        }
    }
    node->arguments = arguments;

    // parse function body
    expect("{");
    Node *block = calloc(1, sizeof(Node));
    block->kind = ND_BLOCK;
    Node *cur = block;

    while (!consume("}")) {
        cur->left = stmt();
        Node *next = calloc(1, sizeof(Node));
        next->kind = ND_BLOCK;
        cur->right = next;
        cur = next;
    }

    node->left = block;
    // final local vars offset
    if (vector_count(locals) > 0) {
        LocalVar *last_local_var = (LocalVar*) vector_get_last(locals);
        node->offset = last_local_var->offset;
    }

    return node;
}

// init parses the next 'init' (in EBNF) as AST.
// For variable initialization.
Node *init() {
    if (consume("{")) {
        Vector *elements = new_vector();
        while (!consume("}")) {
            vector_add(elements, expr());
            if (!consume(",")) {
                expect("}");
                break;
            }
        }

        Node *node = calloc(1, sizeof(Node));
        node->kind = ND_ARRAY;
        node->type = new_type(ARRAY);
        if (node->type->array_size == -1) {
            node->type->array_size = vector_count(elements);
        }
        node->arguments = elements;
        return node;
    }

    return expr();
}

// global parses the next global variable and defines it.
void global(Type *ptr_ty, Token *name) {
    Type *ty = array_type(ptr_ty);
    // Check if the name has already been used
    if (find_global_var(name)) {
        error_at(name->str, "Global variable %.*s has already been declared", name->len, name->str);
    }

    GlobalVar *var = calloc(1, sizeof(GlobalVar));
    var->name = name->str;
    var->len = name->len;
    var->type = ty;

    // initializer
    if (consume("=")) {
        Node *init_node_before = init();
        if (ty->ty == ARRAY && init_node_before->type && init_node_before->type->ty == ARRAY) {
            // for eval_global_init to know which type this array is composed of
            init_node_before->type->ptr_to = ty->ptr_to;
        }
        Node *init_node = eval_global_init(init_node_before);
        var->init = init_node;

        // support excluding array length: e.g. "int arr[] = {1, 2, 3};"
        if (ty->ty == ARRAY && ty->array_size == -1) {
            // ND_STRING (string literal) represents char array
            if ((init_node->type && init_node->type->ty != ARRAY) && init_node->kind == ND_STRING) {
                error_at(name->str, "expected array initializer");
            }
            if (init_node->type && init_node->type->ty == ARRAY) {
                ty->array_size = init_node->type->array_size;
            } else if (init_node->kind == ND_STRING) {
                // string literal length + null sequence
                ty->array_size = init_node->len + 1;
            } else {
                error_at(name->str, "unknown array type");
            }
        }
    }

    // calculate offset after parsing (possible) initialization
    var->offset = size_of(ty);

    vector_add(globals, var);
}

// program parses the next 'program' (in EBNF) as AST, a.k.a. the whole program.
void program() {
    code = new_vector();
    globals = new_vector();
    strings = new_vector();

    while (!at_eof()) {
        // Check if the next token is a global variable, or a function.
        Type *base_ty = base_type();
        Type *type = ptr_type(base_ty->ty);
        Token *tok = expect_identifier();

        if (consume("(")) {
            // function
            vector_add(code, func(type, tok));
        } else {
            // global variable
            global(type, tok);
            expect(";");
        }
    }
}
