#include <stddef.h>
#include <stdint.h>
#include <limits.h>
#include "string-util.h"

#define ALIGN (sizeof(size_t))
#define ONES ((size_t)-1/UCHAR_MAX)                                                                      
#define HIGHS (ONES * (UCHAR_MAX/2+1))
#define HASZERO(x) ((x)-ONES & ~(x) & HIGHS)

#define SS (sizeof(size_t))
void *memset(void *dest, int c, size_t n)
{
	unsigned char *s = dest;
	c = (unsigned char)c;
	for (; ((uintptr_t)s & ALIGN) && n; n--) *s++ = c;
	if (n) {
		size_t *w, k = ONES * c;
		for (w = (void *)s; n>=SS; n-=SS, w++) *w = k;
		for (s = (void *)w; n; n--, s++) *s = c;
	}
	return dest;
}

void *memcpy(void *dest, const void *src, size_t n)
{
	void *ret = dest;
	
	//Cut rear
	uint8_t *dst8 = dest;
	const uint8_t *src8 = src;
	switch (n % 4) {
		case 3 : *dst8++ = *src8++;
		case 2 : *dst8++ = *src8++;
		case 1 : *dst8++ = *src8++;
		case 0 : ;
	}
	
	//stm32 data bus width
	uint32_t *dst32 = (void *)dst8;
	const uint32_t *src32 = (void *)src8;
	n = n / 4;
	while (n--) {
		*dst32++ = *src32++;
	}
	
	return ret;
}

char *strchr(const char *s, int c)
{
	for (; *s && *s != c; s++);
	return (*s == c) ? (char *)s : NULL;
}

char *strcpy(char *dest, const char *src)
{
	const unsigned char *s = src;
	unsigned char *d = dest;
	while ((*d++ = *s++));
	return dest;
}

char *strncpy(char *dest, const char *src, size_t n)
{
	const unsigned char *s = src;
	unsigned char *d = dest;
	while (n-- && (*d++ = *s++));
	return dest;
}

char* itoa(int val)
{
    static char buf[10] = { "0" };
    int i = 9;
    for (;i >= 0 ; i--, val /= 10)   
        buf[i] = "0123456789"[val % 10];
    return &buf; 
}

int bounded_strcmp(char * src1 , char * src2 , int bound)
{
	int i = 0;
	char * pSrc1;
	char * pSrc2 ;
	pSrc1 = src1 ;
	pSrc2 = src2 ;

	while(*pSrc1 && *pSrc2 && (i < bound) && ( *pSrc1 == *pSrc2))
	{
		i++; pSrc1++; pSrc2++;
	}

	return !(*pSrc1 - *pSrc2); // result : 1 equal ; 0 unequal
}
