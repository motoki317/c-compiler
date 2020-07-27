//#include <stddef.h>
//#include <stdlib.h>
//#include <string.h>
//#include <stdio.h>
//#include <time.h>

// sudoku board size
int blockSize = 3;
int size = 3 * 3;
typedef int Board[9][9];

int called = 0;

void *NULL = 0;

typedef struct {
    int *possibilities;
    int size;
} Possibilities;

Possibilities *possibleNumbersAt(Board *b, int i, int j);
int isSolved(Board *b);
void printBoard(Board *b);

void freePossibilitiesMap(Possibilities ***map) {
    int i;
    int j;
    for (i = 0; i < size; ++i) {
        if (!map[i]) break;
        for (j = 0; j < size; ++j) {
            if (!map[i][j]) {
                continue;
            }
            free(map[i][j]->possibilities);
            free(map[i][j]);
        }
        free(map[i]);
    }
}

// Solves the sudoku game, returns the board if successfully solved the game.
Board *Solve(Board *b) {
    ++called;
    Possibilities ***possibilitiesMap = calloc(9, sizeof(Possibilities**));

    int i;
    int j;
    Possibilities **row;
    Possibilities *possibilities;
    for (i = 0; i < size; ++i) {
        possibilitiesMap[i] = calloc(9, sizeof(Possibilities*));
        row = possibilitiesMap[i];
        for (j = 0; j < size; ++j) {
            if ((*b)[i][j] != 0) {
                continue;
            }

            // check possible numbers for each unfilled square
            // if any of them were not fillable, then return immediately
            possibilities = possibleNumbersAt(b, i, j);
            int *p = possibilities->possibilities;
            if (possibilities->size == 0) {
                freePossibilitiesMap(possibilitiesMap);
                return NULL;
            }
            row[j] = possibilities;
        }
    }

    // fill if there's only one possibility
    Board *solvedBoard;
    for (i = 0; i < size; ++i) {
        row = possibilitiesMap[i];
        for (j = 0; j < size; ++j) {
            if ((*b)[i][j] != 0) {
                continue;
            }

            possibilities = row[j];
            if (possibilities->size == 1) {
                (*b)[i][j] = possibilities->possibilities[0];
                // fill the number and check
                solvedBoard = Solve(b);
                if (solvedBoard != NULL) {
                    return solvedBoard;
                }
                // not solvable
                (*b)[i][j] = 0;
                freePossibilitiesMap(possibilitiesMap);
                return NULL;
            }
        }
    }

    // if multiple numbers are possible, check each of them one by one
    for (i = 0; i < size; ++i) {
        row = possibilitiesMap[i];
        for (j = 0; j < size; ++j) {
            if ((*b)[i][j] != 0) {
                continue;
            }

            possibilities = row[j];
            if (possibilities->size > 1) {
                int k;
                for (k = 0; k < possibilities->size; ++k) {
                    int num = possibilities->possibilities[k];
                    (*b)[i][j] = num;
                    solvedBoard = Solve(b);
                    if (solvedBoard != NULL) {
                        return solvedBoard;
                    }
                }
                // not solvable
                (*b)[i][j] = 0;
                freePossibilitiesMap(possibilitiesMap);
                return NULL;
            }
        }
    }

    // if none of above code returned true of false, then all numbers must have been filled
    if (isSolved(b)) {
        return b;
    } else {
        freePossibilitiesMap(possibilitiesMap);
        return NULL;
    }
}

// Checks row validity
int checkRowValidity(Board *b, int i) {
    char checked[9] =  {0, 0, 0, 0, 0, 0, 0, 0, 0};
    int col;
    for (col = 0; col < size; ++col) {
        int num = (*b)[i][col];
        if (num == 0) {
            continue;
        }

        if (checked[num-1]) {
            return 0;
        }
        checked[num-1] = 1;
    }
    return 1;
}

// Checks column validity
int checkColumnValidity(Board *b, int j) {
    char checked[9] =  {0, 0, 0, 0, 0, 0, 0, 0, 0};
    int row;
    for (row = 0; row < size; ++row) {
        int num = (*b)[row][j];
        if (num == 0) {
            continue;
        }

        if (checked[num-1]) {
            return 0;
        }
        checked[num-1] = 1;
    }
    return 1;
}

// Checks block validity of the given coords (NOT block-wise coords)
int checkBlockValidity(Board *b, int i, int j) {
    int blockI = i / 3;
    int blockJ = j / 3;

    char checked[9] =  {0, 0, 0, 0, 0, 0, 0, 0, 0};
    for (i = blockI * 3; i < (blockI+1)*3; ++i) {
        for (j = blockJ * 3; j < (blockJ+1)*3; ++j) {
            int num = (*b)[i][j];
            if (num == 0) {
                continue;
            }

            if (checked[num-1]) {
                return 0;
            }
            checked[num-1] = 1;
        }
    }
    return 1;
}

// Checks the entire board is filled and is valid
int isSolved(Board *b) {
    // check if all is filled
    int i;
    int j;
    for (i = 0; i < size; ++i) {
        for (j = 0; j < size; ++j) {
            if ((*b)[i][j] == 0) {
                return 0;
            }
        }
    }
    // check blocks
    for (i = 0; i < blockSize; ++i) {
        for (j = 0; j < blockSize; ++j) {
            if (!checkBlockValidity(b, i*blockSize, j*blockSize)) {
                return 0;
            }
        }
    }
    // check rows
    for (i = 0; i < size; ++i) {
        if (!checkRowValidity(b, i)) {
            return 0;
        }
    }
    // check columns
    for (j = 0; j < size; ++j) {
        if (!checkColumnValidity(b, j)) {
            return 0;
        }
    }
    return 1;
}

Possibilities *possibleNumbersAt(Board *b, int i, int j)  {
    char taken[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};

    // row
    int col;
    int num;
    for (col = 0; col < size; ++col) {
        num = (*b)[i][col];
        if (num == 0) {
            continue;
        }
        taken[num-1] = 1;
    }

    // column
    int row;
    for (row = 0; row < 9; ++row) {
        num = (*b)[row][j];
        if (num == 0) {
            continue;
        }
        taken[num-1] = 1;
    }

    // block
    int blockI = i / 3;
    int blockJ = j / 3;
    for (row = blockI * 3; row < (blockI+1)*3; ++row) {
        for (col = blockJ * 3; col < (blockJ+1)*3; ++col) {
            num = (*b)[row][col];
            if (num == 0) {
                continue;
            }
            taken[num-1] = 1;
        }
    }

    Possibilities *possibilities = calloc(1, sizeof(Possibilities));
    int k;
    int count = 0;
    for (k = 0; k < size; ++k) {
        if (!taken[k]) {
            ++count;
        }
    }

    possibilities->size = count;
    int *p = calloc(1, count * sizeof(int));
    possibilities->possibilities = p;
    count = 0;
    for (k = 0; k < size; ++k) {
        if (!taken[k]) {
            p[count] = k + 1;
            ++count;
        }
    }

    return possibilities;
}

Board *NewBoard(char **input) {
    Board *board = calloc(1, sizeof(Board));
    int i;
    for (i = 0; i < size; ++i) {
        char *line = input[i];
        if (strlen(line) != 9) {
            printf("invalid length at line %d, please input a 9x9 sudoku Board\n", i+1);
            return NULL;
        }

        int j;
        for (j = 0; j < size; ++j) {
            if (line[j] == '_') {
                (*board)[i][j] = 0;
            } else {
                (*board)[i][j] = line[j] - '0';
            }
        }
    }

    return board;
}

// Prints the board, printing '_' for blanks.
void printBoard(Board *b) {
    int i;
    int j;
    for (i = 0; i < 9; ++i) {
        for (j = 0; j < 9; ++j) {
            int num = (*b)[i][j];
            if (num == 0) {
                printf("_");
            } else {
                printf("%d", num);
            }
        }
        printf("\n");
    }
}

int main() {
    printf("Please input a 9x9 sudoku board:\n");
    char *input[9] = {
        "__42_36__",
        "1_______7",
        "_________",
        "__26_93__",
        "_8__3__5_",
        "__34_28__",
        "_________",
        "3_______6",
        "__65_42__"
    };
    // process #include <stdio.h> correctly to read from stdin
//    int i;
//    for (i = 0; i < size; ++i) {
//        input[i] = calloc(11, sizeof(char));
//        if (!fgets(input[i], 11, stdin)) {
//            printf("failed to read line %d\n", i + 1);
//            exit(1);
//        }
//        // null terminator
//        input[i][strcspn(input[i], "\n")] = 0;
//    }

    Board *board = NewBoard(input);
    printf("initialized board:\n");
    printBoard(board);

    printf("Solving...\n");
    Board *solvedBoard = Solve(board);
    if (solvedBoard != NULL) {
        printf("Solved!\n");
        printf("Called Solve() %d times\n", called);

        printBoard(solvedBoard);
    } else {
        printf("Not solvable...\n");
    }
}
