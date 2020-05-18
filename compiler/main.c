#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    // Symbol
    TK_RESERVED,
    // Number
    TK_NUM,
    // End of the tokens
    TK_EOF,
} TokenKind;

typedef struct Token Token;

struct Token {
    TokenKind kind;
    Token *next;
    int val;
    char *str;
};

// Whole user input
char *user_input;

// Current token
Token *token;

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
bool consume(char op) {
    if (token->kind != TK_RESERVED || token->str[0] != op) {
        return false;
    }
    token = token->next;
    return true;
}

// expect precedes to the next token when the current token is the given expected operator.
// Reports error otherwise.
void expect(char op) {
    if (token->kind != TK_RESERVED || token->str[0] != op) {
        error_at(token->str, "Next token is not '%c'\n", op);
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

// tokenize tokenizes the given character sequence and returns it.
Token *tokenize(char *p) {
    Token head;
    head.next = NULL;
    Token *cur = &head;

    // While the next character is not a null character
    while (*p) {
        // Skip space characters
        if (isspace(*p)) {
            p++;
            continue;
        }

        char *tokens = "+-*/()";

        bool foundToken = false;
        while (*tokens != '\0') {
            if (*p == *tokens) {
                cur = new_token(TK_RESERVED, cur, p++);
                foundToken = true;
                break;
            }
            tokens++;
        }
        if (foundToken) {
            continue;
        }

        if (isdigit(*p)) {
            cur = new_token(TK_NUM, cur, p);
            cur->val = strtol(p, &p, 10);
            continue;
        }

        error_at(p, "Cannot tokenize");
    }

    new_token(TK_EOF, cur, p);
    return head.next;
}

/**
Basic four arithmetic operations parsing, as EBNF
expr    = mul ("+" mul | "-" mul)*
mul     = primary ("*" primary | "/" primary)*
primary = num | "(" expr ")"
*/

// Node kind for building AST (Abstract Syntax Tree)
typedef enum {
    ND_ADD, // +
    ND_SUB, // -
    ND_MUL, // *
    ND_DIV, // /
    ND_NUM, // Number, node is expected to be leaf if and only if kind == ND_NUM, as of now
} NodeKind;

typedef struct Node Node;

struct Node {
    NodeKind kind;
    Node *left;
    Node *right;
    // Value here if the kind is ND_NUM
    int val;
};

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

Node *mul();

// expr parses the next 'expr' (in EBNF) as AST.
Node *expr() {
    Node *node = mul();
    for (;;) {
        // If there are '+' or '-' tokens, include the current node as left child and create a new node.
        if (consume('+')) {
            node = new_node(ND_ADD, node, mul());
        } else if (consume('-')) {
            node = new_node(ND_SUB, node, mul());
        } else {
            return node;
        }
    }
}

Node *primary();

// mul parses the next 'mul' (in EBNF) as AST.
Node *mul() {
    Node *node = primary();
    for (;;) {
        if (consume('*')) {
            node = new_node(ND_MUL, node, primary());
        } else if (consume('/')) {
            node = new_node(ND_DIV, node, primary());
        } else {
            return node;
        }
    }
}

// primary parses the next 'primary' (in EBNF) as AST.
Node *primary() {
    // If the next token is opening parenthesis, expect 'expr' inside.
    if (consume('(')) {
        Node *node = expr();
        expect(')');
        return node;
    }
    // Otherwise expect a number.
    return new_node_num(expect_number());
}

// gen walks the given tree and prints out the assembly calculating the given tree.
void gen(Node *node) {
    if (node->kind == ND_NUM) {
        printf("        push %d\n", node->val);
        return;
    }

    // Calculate children and push them onto the 'rsp', register stack pointer.
    gen(node->left);
    gen(node->right);

    // Pop the first result into rax and the second result into rax.
    printf("        pop rdi\n");
    printf("        pop rax\n");

    // Perform the operation
    switch (node->kind) {
    case ND_ADD:
        printf("        add rax, rdi\n");
        break;
    case ND_SUB:
        printf("        sub rax, rdi\n");
        break;
    case ND_MUL:
        printf("        imul rax, rdi\n");
        break;
    case ND_DIV:
        // cqo expands 64-bit rax into 128-bit rdx, rax
        printf("        cqo\n");
        // idiv divides 128-bit rdx, rax by the 64-bit given register (rdi here),
        // and sets the quotient to rax and remainder to rdx.
        printf("        idiv rdi\n");
        break;
    }

    // Push rax to the stack for the callee.
    printf("        push rax\n");
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Invalid arguments length\n");
        return 1;
    }

    // Save the input
    user_input = argv[1];

    // Tokenize the input
    token = tokenize(argv[1]);

    // Consume tokens to build AST (Abstract Syntax Tree)
    Node *root = expr();

    // Base assembly syntax
    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    // Calculate the result
    gen(root);

    // Pop the result from stack
    printf("        pop rax\n");
    printf("        ret\n");

    return 0;
}
