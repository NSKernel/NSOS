#ifndef _STRING_H_
#define _STRING_H_

#include <am.h>

void *memset(void *b, int c, int n);
void *memcpy(void *dest, const void *src, size_t n);
size_t strlen(const char* s);
char *strcpy(char *dest, const char *src);
char *strncpy(char *dest, const char *src, size_t n);
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);

#endif
