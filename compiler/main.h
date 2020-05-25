struct Token;
typedef struct Token Token;

// Whole user input
extern char *user_input;
// Current token
extern Token *token;

Token *tokenize(char *p);

// Node kind for building AST (Abstract Syntax Tree)
typedef enum {
    ND_ASSIGN, // =
    ND_EQUAL, // ==
    ND_NOT_EQUAL, // !=
    ND_LESS_EQUAL, // <=
    ND_LESS, // <
    ND_GREATER_EQUAL, // >=
    ND_GREATER, // >
    ND_ADD, // +
    ND_SUB, // -
    ND_MUL, // *
    ND_DIV, // /
    ND_LOCAL_VAR, // Local variable
    ND_NUM, // Number, node is expected to be leaf if and only if kind == ND_NUM, as of now
} NodeKind;

typedef struct Node Node;

struct Node {
    NodeKind kind;
    Node *left;
    Node *right;
    // Value here if the kind is ND_NUM
    int val;
    // Offset here if the kind is ND_LVAR
    int offset;
};

extern Node *code[100];

void program();

void gen();
