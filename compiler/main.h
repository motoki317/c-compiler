#include <stddef.h>

struct Token;
typedef struct Token Token;

// Whole user input
extern char *user_input;
// Current token
extern Token *token;

typedef struct Type Type;

typedef enum {
    INT,
    PTR,
    ARRAY,
} TypeKind;

// Variable type
struct Type {
  TypeKind ty;
  // pointer to / array of what type
  struct Type *ptr_to;
  size_t array_size;
};

// size_of returns the size of the given type.
size_t size_of(Type *ty);

typedef struct LocalVar LocalVar;

// Local variables as linked list
struct LocalVar {
    LocalVar *next;
    // variable name
    char *name;
    // variable name length
    int len;
    // offset from rbp
    int offset;
    // type
    Type *type;
};

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
    ND_ADDR, // &
    ND_DEREF, // *
    ND_FUNC, // function
    ND_FUNC_CALL, // function call
    ND_RETURN, // "return" statement
    ND_IF, // "if" statement
    ND_WHILE, // "while" statement
    ND_FOR, // "for" statement
    ND_BLOCK, // "{ ~ }" block statement
    ND_LOCAL_VAR, // Local variable
    ND_GLOBAL_VAR, // Global variable
    ND_NUM, // Number, node is expected to be leaf if and only if kind == ND_NUM, as of now
} NodeKind;

typedef struct Node Node;

struct Node {
    NodeKind kind;
    Node *left;
    Node *right;
    Node *third;
    Node *fourth;
    // Value here if the kind is ND_NUM
    int val;
    // Offset and type here if the kind is ND_LOCAL_VAR or ND_GLOBAL_VAR,
    // total local vars offset here if the kind is ND_FUNC
    int offset;
    // function return type if the kind is ND_FUNC
    Type *type;
    // Label name sequencing here if the kind is ND_IF, ND_WHILE, or ND_FOR
    int label;
    // Function name if the kind is ND_FUNC_CALL or ND_FUNC
    // Variable name here if the kind is ND_LOCAL_VAR or ND_GLOBAL_VAR
    char *str;
    int len;
    // List of function arguments if the kind is ND_FUNC
    // as a linked list - the first one here, the next one in left
    Node *arguments;
};

extern Node *code[100];

typedef LocalVar GlobalVar;
// List of global variables as a linked list
extern GlobalVar *globals;

void program();

void gen();
