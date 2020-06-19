#include <stddef.h>

// container.c

void error(char *fmt, ...);

// Reports error at the given location
void error_at(char *loc, char *fmt, ...);

char *read_file(char *path);

typedef struct Vector {
    // pointer to the first data
    // assume void* = 8 bytes
    void** data;
    // current data field size
    int size;
    // actual elements count
    int count;
} Vector;

Vector *new_vector();
int vector_count(Vector*);
void vector_add(Vector*, void*);
void vector_set(Vector*, int, void*);
void *vector_get(Vector*, int);
void *vector_get_last(Vector*);
void vector_delete(Vector*, int);
void vector_free(Vector*);

// main.c

// Given file name
extern char *file_name;
// Whole user input
extern char *user_input;

// parse.c

struct Token;
typedef struct Token Token;

// Current token
extern Token *token;

typedef struct Type Type;

typedef enum {
    CHAR,
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

typedef struct LocalVar LocalVar;

// Local variables
struct LocalVar {
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
    ND_STRING, // String literal
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
    // String literal label name here if the kind is ND_STRING
    int label;
    // Function name if the kind is ND_FUNC_CALL or ND_FUNC
    // Variable name here if the kind is ND_LOCAL_VAR or ND_GLOBAL_VAR
    // String literal here if the kind is ND_STRING
    char *str;
    int len;
    // List of function arguments if the kind is ND_FUNC or ND_FUNC_CALL, elements: Node*
    Vector *arguments;
};

// size_of returns the size of the given type.
size_t size_of(Type *ty);
// type_of returns the type of the given node.
Type *type_of(Node *node);

// Code vector, elements: Node*
extern Vector *code;

typedef LocalVar GlobalVar;
// Global variables, elements: GlobalVar*
extern Vector *globals;

// String literals
extern Vector *strings;

void program();

// codegen.c

void gen();
