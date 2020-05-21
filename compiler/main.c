#include "main.h"

#include <stdio.h>

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Invalid arguments length\n");
        return 1;
    }

    // Save the input
    user_input = argv[1];

    // Tokenize the input
    token = tokenize(argv[1]);

    // Generate the output
    gen();

    return 0;
}
