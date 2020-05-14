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

        if (*p == '+' || *p == '-') {
            cur = new_token(TK_RESERVED, cur, p++);
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

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Invalid arguments length\n");
        return 1;
    }

    // Save the input
    user_input = argv[1];

    // Tokenize the input
    token = tokenize(argv[1]);

    // Base assembly syntax
    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    // Expect a number in the first token
    printf("        mov rax, %d\n", expect_number());

    while (!at_eof()) {
        if (consume('+')) {
            printf("        add rax, %d\n", expect_number());
        }

        // Expect '+' or '-', otherwise throw an error
        expect('-');
        printf("        sub rax, %d\n", expect_number());
    }

    printf("        ret\n");

    return 0;
}
