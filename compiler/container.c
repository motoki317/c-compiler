#include "main.h"

#include <stdlib.h>

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
