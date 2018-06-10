#include <am.h>

void *memset(void *b, int c, size_t n) {
    char realc = c;
    char *B = (char*)b;
    while (--n >= 0) {
        B[n] = realc;
    }
    return b;
}

void *memcpy(void *dest, const void *src, int n) {
    char *DEST = (char*)dest;
    char *SRC = (char*)src;
    while (--n >= 0) {
        DEST[n] = SRC[n];
    }
    return dest;
}

size_t strlen(const char* s) {
    int len = 0;
    while (s[len] != 0) len += 1;
    return len;
}

char *strcpy(char *dest, const char *src) {
    size_t i;
    for (i = 0; src[i] != '\0'; i++)
        dest[i] = src[i];
    
    return dest;
}

char *strncpy(char *dest, const char *src, size_t n) {
    size_t i;
    for (i = 0; i < n && src[i] != '\0'; i++)
        dest[i] = src[i];
    for ( ; i < n; i++)
        dest[i] = '\0';
    
    return dest;
}

int strcmp(const char *cs, const char *ct) {
	unsigned char c1, c2;

	while (1) {
		c1 = *cs++;
		c2 = *ct++;
		if (c1 != c2)
			return c1 < c2 ? -1 : 1;
		if (!c1)
			break;
	}
	return 0;
}

int strncmp(const char *s1, const char *s2, size_t n) {
    while(n--)
        if(*s1++!=*s2++)
            return *(unsigned char*)(s1 - 1) - *(unsigned char*)(s2 - 1);
    return 0;
}
