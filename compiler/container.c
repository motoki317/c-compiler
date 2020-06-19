#include "main.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

// Reports error
void error(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

// Reports error at the given location
void error_at(char *loc, char *fmt, ...) {
    // Retrieve starting and ending point of the line in which 'loc' is included
    char *line = loc;
    while (user_input < line && line[-1] != '\n') line--;
    char *end = loc;
    while (*end != '\n') end++;

    // Retrieve line number
    int line_num = 1;
    for (char *p = user_input; p < line; p++) {
        if (*p == '\n') {
            line_num++;
        }
    }

    // Print file name, line number, and content of the line
    int indent = fprintf(stderr, "%s:%d: ", file_name, line_num);
    fprintf(stderr, "%.*s\n", (int) (end - line), line);

    va_list ap;
    va_start(ap, fmt);
    // Point to the error location with '^'
    int pos = loc - line + indent;
    fprintf(stderr, "%*s", pos, ""); // space 'pos' times
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

// Returns the content of the given file name
char *read_file(char *path) {
    // Open file
    FILE *fp = fopen(path, "r");
    if (!fp) {
        error("cannot open %s: %s", path, strerror(errno));
    }

    // Retrieve file size
    if (fseek(fp, 0, SEEK_END) == -1) {
        error("%s: fseek: %s", path, strerror(errno));
    }
    size_t size = ftell(fp);
    if (fseek(fp, 0, SEEK_SET) == -1) {
        error("%s: fseek: %s", path, strerror(errno));
    }

    // Read file content
    char *buf = calloc(1, size + 2);
    fread(buf, size, 1, fp);

    // Make sure the content always ends with "\n\0"
    if (size == 0 || buf[size - 1] != '\n') {
        buf[size++] = '\n';
    }
    buf[size] = '\0';
    fclose(fp);
    return buf;
}

// Code from https://gist.github.com/EmilHernvall/953968/0fef1b1f826a8c3d8cfb74b2915f17d2944ec1d0

Vector *new_vector() {
    return calloc(1, sizeof(Vector));
}

int vector_count(Vector *v) {
	return v->count;
}

void vector_add(Vector *v, void *elt) {
	if (v->size == 0) {
		v->size = 10;
		v->data = calloc(1, sizeof(void*) * v->size);
	}

	// condition to increase v->data:
	// last slot exhausted
	if (v->size == v->count) {
		v->size *= 2;
		v->data = realloc(v->data, sizeof(void*) * v->size);
	}

	v->data[v->count] = elt;
	v->count++;
}

void vector_set(Vector *v, int index, void *elt) {
	if (index >= v->count) {
		return;
	}

	v->data[index] = elt;
}

void *vector_get(Vector *v, int index) {
	if (index >= v->count) {
		return NULL;
	}

	return v->data[index];
}

void *vector_get_last(Vector *v) {
    return vector_get(v, vector_count(v) - 1);
}

void vector_delete(Vector *v, int index) {
	if (index >= v->count) {
		return;
	}

	for (int i = index; i < v->count - 1; i++) {
        v->data[i] = v->data[i + 1];
	}

	v->count--;
}

void vector_free(Vector *v) {
	free(v->data);
}
