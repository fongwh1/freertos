#ifndef _STRING_H
#define _STRING_H
void *memset(void *dest, int c, size_t n);
void *memcpy(void *dest, const void * src, size_t n);
char *strchr(const char *s, int c);
char *strcpy(char *dest, const char *src);
char *strncpy(char *dest, const char *src, size_t n);
#endif
