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
};

typedef enum {
    // Reserved symbols
    TK_RESERVED,
    // Identifiers (variables)
    TK_IDENTIFIER,
    // Number
    TK_NUM,
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

// Whole user input
char *user_input;

// Current token
Token *token;

// Each statements
Node *code[100];

// Reports error at the given location
void error_at(char *loc, char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    int pos = loc - user_input;
    fprintf(stderr, "%s\n", user_input);
    // space * pos times
    fprintf(stderr, "%*s", pos, "");
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

// consume returns true when the current token is the given expected operator, and precedes to the next token.
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

// consume_identifier consumes the next identifier token if exists, and precedes to the next token.
Token *consume_identifier() {
    if (token->kind != TK_IDENTIFIER) {
        return NULL;
    }
    Token *ret = token;
    token = token->next;
    return ret;
}

// expect precedes to the next token when the current token is the given expected operator.
// Reports error otherwise.
void expect(char *op) {
        // next token is not a symbol
    if (token->kind != TK_RESERVED ||
        // not correct expected token length
        strlen(op) != token->len ||
        // compare the first token->len bytes of two strings
        memcmp(token->str, op, token->len)) {
        error_at(token->str, "Next token is not \"%s\"\n", op);
    }
    token = token->next;
}

// expect_number returns number and precedes to the next token when the current token is represents a number.
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

// tokenize_next tokenizes the next characters.
Token *tokenize_next(char **p, Token *cur) {
    // Skip space characters
    if (isspace(**p)) {
        *p += 1;
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

    // Check for identifiers (variables)
    if ('a' <= **p && **p <= 'z') {
        Token *next = new_token(TK_IDENTIFIER, cur, *p);
        next->len = 1;
        *p += 1;
        return next;
    }

    // Check for number
    if (isdigit(**p)) {
        Token *next = new_token(TK_NUM, cur, *p);
        next->val = strtol(*p, p, 10);
        return next;
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
program    = stmt*
stmt       = expr ";"
expr       = assign
assign     = equality ("=" assign)?
equality   = relational ("==" relational | "!=" relational)*
relational = add ("<" add | "<=" add | ">" add | ">=" add)*
add        = mul ("+" mul | "-" mul)*
mul        = unary ("*" unary | "/" unary)*
unary      = ("+" | "-")? primary
primary    = num | ident | "(" expr ")"
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

Node *expr();

// primary parses the next 'primary' (in EBNF) as AST.
Node *primary() {
    // If the next token is opening parenthesis, expect 'expr' inside.
    if (consume("(")) {
        Node *node = expr();
        expect(")");
        return node;
    }
    // If the next token is identifier
    Token *tok = consume_identifier();
    if (tok) {
        // only support 26, 1 length local variable for now
        Node *node = calloc(1, sizeof(Node));
        node->kind = ND_LOCAL_VAR;
        node->offset = (tok->str[0] - 'a' + 1) * 8;
        return node;
    }
    // Otherwise expect a number.
    return new_node_num(expect_number());
}

// unary parses the next 'unary' (in EBNF) as AST.
Node *unary() {
    if (consume("+")) {
        return primary();
    } else if (consume("-")) {
        return new_node(ND_SUB, new_node_num(0), primary());
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

// stmt parses the next 'stmt' (in EBNF) as AST.
Node *stmt() {
    Node *node = expr();
    expect(";");
    return node;
}

// program parses the next 'program' (in EBNF) as AST, a.k.a. the whole program.
void program() {
    int i = 0;
    while (!at_eof()) {
        code[i++] = stmt();
    }
    code[i] = NULL;
}
