#include "main.h"

#include <stdio.h>
#include <stdlib.h>

// register names
char arguments[6][4] = {
    "rdi", "rsi", "rdx", "rcx", "r8", "r9",
};

// Reports error at the given location
void error(char *message) {
    fprintf(stderr, "%s\n", message);
    exit(1);
}

void gen_tree();

// gen_lvalue evaluates the next lvalue (prints error and exists if not), and pushes the address to the stack.
void gen_lvalue(Node *node) {
    switch(node->kind) {
    case ND_LOCAL_VAR:
        // calculate the variable address
        printf("        mov rax, rbp\n");
        printf("        sub rax, %d\n", node->offset);
        printf("        push rax\n");
        return;
    case ND_DEREF:
        gen_tree(node->left);
        return;
    }

    error("expected lvalue, but not a local variable");
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
    case ND_ADDR:
        gen_lvalue(node->left);
        return;
    case ND_DEREF:
        gen_tree(node->left);

        printf("        pop rax\n");
        printf("        mov rax, [rax]\n");
        printf("        push rax\n");
        return;
    case ND_IF:
        gen_tree(node->left);

        printf("        pop rax\n");
        printf("        cmp rax, 0\n");
        // If the evaluated condition is 0 (false),
        if (node->third) {
            // Jump to "else" block
            printf("        je .Lelse%d\n", node->label + 1);
        } else {
            // No "else" block, so jump to outside of the "if" statement
            printf("        je .Lend%d\n", node->label);
        }
        // otherwise, evaluate inside "if"
        gen_tree(node->right);
        // pop the result so it doesn't stay on stack
        printf("        pop rax\n");
        if (node->third) {
            // and go to end (if else block exists)
            printf("        jmp .Lend%d\n", node->label);
            // generate "else" block
            printf(".Lelse%d:\n", node->label + 1);
            gen_tree(node->third);
            // pop the result so it doesn't stay on stack
            printf("        pop rax\n");
        }
        // end block (next code block)
        printf(".Lend%d:\n", node->label);
        // leave something on stack (gen func expects each gen_tree to generate a value)
        printf("        push 0\n");
        return;
    case ND_WHILE:
        printf(".Lbegin%d:\n", node->label);

        gen_tree(node->left);
        printf("        pop rax\n");
        printf("        cmp rax, 0\n");
        // If the evaluated condition is false, break the loop
        printf("        je .Lend%d\n", node->label + 1);
        // otherwise, evaluate inside "while"
        gen_tree(node->right);
        // pop the result so it doesn't stay on stack
        printf("        pop rax\n");
        // then return back to the beginning
        printf("        jmp .Lbegin%d\n", node->label);

        // end block (next code block)
        printf(".Lend%d:\n", node->label + 1);
        // leave something on stack (gen func expects each gen_tree to generate a value)
        printf("        push 0\n");
        return;
    case ND_FOR:
        // init
        if (node->left) {
            gen_tree(node->left);
            // pop the result so it doesn't stay on stack
            printf("        pop rax\n");
        }
        // "for" block
        printf(".Lbegin%d:\n", node->label);
        if (node->right) {
            gen_tree(node->right);
        } else {
            printf("        push 1\n");
        }
        printf("        pop rax\n");
        printf("        cmp rax, 0\n");
        // If the evaluated condition is false, break the loop
        printf("        je .Lend%d\n", node->label + 1);
        // otherwise, evaluate inside "for"
        gen_tree(node->fourth);
        // pop the result so it doesn't stay on stack
        printf("        pop rax\n");
        // on continue
        if (node->third) {
            gen_tree(node->third);
            // pop the result so it doesn't stay on stack
            printf("        pop rax\n");
        }
        // then return back to the beginning
        printf("        jmp .Lbegin%d\n", node->label);

        // end block (next code block)
        printf(".Lend%d:\n", node->label + 1);
        // leave something on stack (gen func expects each gen_tree to generate a value)
        printf("        push 0\n");
        return;
    case ND_BLOCK:
        while(node->left) {
            gen_tree(node->left);
            // pop the result so it doesn't stay on stack
            printf("        pop rax\n");
            node = node->right;
        }
        printf("        push rax\n");
        return;
    case ND_FUNC_CALL: ;
        // Evaluate arguments
        Node *cur = node;
        int num_args = 0;
        while(cur->left) {
            gen_tree(cur->left);
            cur = cur->right;
            num_args++;
        }
        // Pop evaluated result into registers in order of: RDI, RSI, RDX, RCX, R8, R9
        while(num_args > 0) {
            num_args--;
            if (num_args < 6) {
                printf("        pop %s\n", arguments[num_args]);
            } else {
                printf("        pop rax\n");
            }
        }
        // 16-byte align rsp
        // 1. push the original rsp two times (which pushes rsp by 16 bytes)
        printf("        push rsp\n");
        printf("        push [rsp]\n");
        // 2. 16-byte align rsp, possible subtracting 8 bytes.
        printf("        and rsp, -0x10\n");
        // 3. Call function
        printf("        call %.*s\n", node->len, node->str);
        // 4. bring back the original rsp, which is always at [rsp + 8]
        printf("        add rsp, 8\n");
        printf("        mov rsp, [rsp]\n");
        // push the result to stack
        printf("        push rax\n");
        return;
    case ND_FUNC:
        // Function name
        printf("%.*s:\n", node->len, node->str);
        // Function Prologue
        printf("        push rbp\n");
        printf("        mov rbp, rsp\n");
        // allocate local variables
        if (node->local_vars) {
            printf("        sub rsp, %d\n", node->local_vars->offset);
        }

        // Copy function arguments from registers to stack
        Node *next_arg = node->arguments;
        if (next_arg) {
            // argument index
            for (int i = next_arg->offset / 8 - 1; i >= 0; i--) {
                // evaluate place as local variable
                // note: forcefully rewriting the offset here, not a good practice
                next_arg->offset = (i + 1) * 8;
                gen_lvalue(next_arg);
                printf("        pop rax\n");
                if (i < 6) {
                    printf("        mov [rax], %s\n", arguments[i]);
                }
            }
        }

        // Function body
        gen_tree(node->left);
        printf("        pop rax\n");

        // Function Epilogue
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

    // Calculate the result for each functions
    for (int i = 0; code[i]; i++) {
        gen_tree(code[i]);
    }
}
