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
};

char keywords[][8] = {
    "return", "if", "else",
    "for", "while",
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

LocalVar *locals;

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

// consume_keyword returns true when the current token is TK_KEYWORD, and precedes to the next token.
bool consume_keyword(char *keyword) {
    if (token->kind != TK_KEYWORD ||
        strlen(keyword) != token->len ||
        memcmp(keyword, token->str, token->len)) {
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

// consume_identifier returns the next identifier token, otherwise reports an error.
Token *expect_identifier() {
    if (token->kind != TK_IDENTIFIER) {
        error_at(token->str, "Next token is not an identifier\n");
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
        while (is_variable_char(**p)) {
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
program    = func*
func       = ident "(" (expr ("," expr)*)? ")" "{" stmt* "}"
stmt       = expr ";"
            | "{" stmt* "}"
            | "if" "(" expr ")" stmt ("else" stmt)?
            | "while" "(" expr ")" stmt
            | "for" "(" expr? ";" expr? ";" expr? ")" stmt
            | "return" expr ";"
expr       = assign
assign     = equality ("=" assign)?
equality   = relational ("==" relational | "!=" relational)*
relational = add ("<" add | "<=" add | ">" add | ">=" add)*
add        = mul ("+" mul | "-" mul)*
mul        = unary ("*" unary | "/" unary)*
unary      = ("+" | "-")? primary
            | "*" unary
            | "&" unary
primary    = num
            | ident ("(" (expr ("," expr)*)? ")")?
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
    for (LocalVar *var = locals; var; var = var->next) {
        if (var->len == tok->len && memcmp(var->name, tok->str, var->len) == 0) {
            return var;
        }
    }
    return NULL;
}

// new_local_var returns a local variable as node. If this is a new variable, appends it to the list of local variables.
Node *new_local_var(Token *tok) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_LOCAL_VAR;
    // determine offset
    LocalVar *var = find_local_var(tok);
    if (!var) {
        // new variable
        var = calloc(1, sizeof(LocalVar));
        var->next = locals;
        var->name = tok->str;
        var->len = tok->len;
        var->offset = locals->offset + 8;
        locals = var;
    }
    node->offset = var->offset;
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

    // If the next token is identifier (local variable), consume it.
    Token *tok = consume_identifier();
    if (tok) {
        // Function call
        if (consume("(")) {
            Node *node = calloc(1, sizeof(Node));
            node->kind = ND_FUNC_CALL;
            node->str = tok->str;
            node->len = tok->len;

            Node *cur = node;
            while (!consume(")")) {
                cur->left = expr();
                Node *next = calloc(1, sizeof(Node));
                next->kind = ND_FUNC_CALL;
                cur->right = next;
                cur = next;
                if (!consume(",")) {
                    expect(")");
                    break;
                }
            }
            return node;
        }

        // Local variable
        return new_local_var(tok);
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

// stmt parses the next 'stmt' (in EBNF) as AST.
Node *stmt() {
    Node *node;

    if (consume_keyword("return")) {
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
Node *func() {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_FUNC;
    Token *tok = expect_identifier();
    node->str = tok->str;
    node->len = tok->len;

    // current function's local variables, for new_local_var function to append to this
    locals = calloc(1, sizeof(LocalVar));

    expect("(");
    // read function arguments
    while (!consume(")")) {
        Token *arg = expect_identifier();
        // treat each function argument as a local variable
        Node *local_var = new_local_var(arg);
        node->local_vars = local_var;
        node->arguments = local_var;

        if (!consume(",")) {
            expect(")");
            break;
        }
    }

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
    // final list of local variables after parsing function body
    Node *local_vars = calloc(1, sizeof(Node));
    local_vars->kind = ND_LOCAL_VAR;
    local_vars->offset = locals->offset;
    node->local_vars = local_vars;
}

// program parses the next 'program' (in EBNF) as AST, a.k.a. the whole program.
void program() {
    int i = 0;
    while (!at_eof()) {
        code[i++] = func();
    }
    code[i] = NULL;
}
