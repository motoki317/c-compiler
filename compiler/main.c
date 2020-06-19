#include "main.h"

#include <stdio.h>

// Given file name
char *file_name;

// Whole user input
char *user_input;

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Invalid arguments length\n");
        return 1;
    }

    // Read from file
    file_name = argv[1];
    user_input = read_file(file_name);

    // Tokenize the input
    token = tokenize(user_input);

    // Generate the output
    gen();

    return 0;
}
