#include "main.h"

#include <stdio.h>
#include <stdlib.h>

// Reports error at the given location
void error(char *message) {
    fprintf(stderr, "%s\n", message);
    exit(1);
}

// gen_lvalue evaluates the next lvalue (prints error and exists if not), and pushes the address to the stack.
void gen_lvalue(Node *node) {
    if (node->kind != ND_LOCAL_VAR) {
        error("lvalue in assignment is not a local variable");
    }

    // calculate the variable address
    printf("        mov rax, rbp\n");
    printf("        sub rax, %d\n", node->offset);
    printf("        push rax\n");
}

// gen_tree walks the given tree and prints out the assembly calculating the given tree.
void gen_tree(Node *node) {
    switch (node->kind) {
    case ND_NUM:
        printf("        push %d\n", node->val);
        return;
    case ND_LOCAL_VAR:
        // evaluate the variable
        gen_lvalue(node);

        printf("        pop rax\n");
        printf("        mov rax, [rax]\n");
        printf("        push rax\n");
        return;
    case ND_ASSIGN:
        gen_lvalue(node->left);
        gen_tree(node->right);

        printf("        pop rdi\n");
        printf("        pop rax\n");
        printf("        mov [rax], rdi\n");
        printf("        push rdi\n");
        return;
    case ND_RETURN:
        gen_tree(node->left);

        // Pop the result to rax
        printf("        pop rax\n");
        // Function epilogue
        printf("        mov rsp, rbp\n");
        printf("        pop rbp\n");
        printf("        ret\n");
        return;
    }

    // Calculate children and push them onto the 'rsp', register stack pointer.
    gen_tree(node->left);
    gen_tree(node->right);

    // Pop the first result into rax and the second result into rax.
    printf("        pop rdi\n");
    printf("        pop rax\n");

    // Perform the operation
    switch (node->kind) {
    // Basic arithmetic operations
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
    // Comparison operations
    case ND_EQUAL:
        // Compare
        printf("        cmp rax, rdi\n");
        // Set compare result to al (lower 8-bit of rax register)
        printf("        sete al\n");
        // Set rax register from al with zero-extension
        printf("        movzb rax, al\n");
        break;
    case ND_NOT_EQUAL:
        printf("        cmp rax, rdi\n");
        printf("        setne al\n");
        printf("        movzb rax, al\n");
        break;
    case ND_LESS:
        printf("        cmp rax, rdi\n");
        printf("        setl al\n");
        printf("        movzb rax, al\n");
        break;
    case ND_LESS_EQUAL:
        printf("        cmp rax, rdi\n");
        printf("        setle al\n");
        printf("        movzb rax, al\n");
        break;
    case ND_GREATER:
        // opposite of less equal
        printf("        cmp rdi, rax\n");
        printf("        setle al\n");
        printf("        movzb rax, al\n");
        break;
    case ND_GREATER_EQUAL:
        // opposite of less
        printf("        cmp rdi, rax\n");
        printf("        setl al\n");
        printf("        movzb rax, al\n");
        break;
    }

    // Push rax to the stack for the callee.
    printf("        push rax\n");
}

// gen reads the parsed code in AST, and prints out the assembly to complete the compilation.
void gen() {
    // Consume tokens to build multiple ASTs (Abstract Syntax Tree)
    program();

    // Base assembly syntax
    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    // Function Prologue
    printf("        push rbp\n");
    printf("        mov rbp, rsp\n");
    // allocate local variables
    if (locals->offset > 0) {
        printf("        sub rsp, %d\n", locals->offset);
    }

    // Calculate the result for each statements
    for (int i = 0; code[i]; i++) {
        gen_tree(code[i]);
        // just pop the result for now
        printf("        pop rax\n");
    }

    // Function Epilogue
    printf("        mov rsp, rbp\n");
    printf("        pop rbp\n");
    printf("        ret\n");
}
