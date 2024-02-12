#pragma once

#include <stdbool.h>
#include <stddef.h>

#define UUIDSTR_LENGTH 36
#define UUIDSTR_CAPACITY 37

typedef struct uuidstr_t {
    char data[UUIDSTR_LENGTH];
    char zero;
} uuidstr_t;

void uuidstr_fromstr(uuidstr_t *dest, const char *src);

void uuidstr_fromchars(uuidstr_t *dest, size_t len, const char *src);

bool uuidstr_random(uuidstr_t *dest);

char *uuidstr_tostr(const uuidstr_t *src);

bool uuidstr_t_equals_s(const uuidstr_t *a, const char *b);

bool uuidstr_t_equals_t(const uuidstr_t *a, const uuidstr_t *b);

bool uuidstr_is_empty(const uuidstr_t *uuid);