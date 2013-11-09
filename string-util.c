//#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <limits.h>
#include "string-util.h"
#include <stdarg.h>

#define STRING_ZERO "0"

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


/* Here will replace another memory allocation char[] method after implementing "malloc"*/
char *itoa(int val, char * buf)
{
	if(val == 0){
		buf = STRING_ZERO;
		return buf;
	}

	int i = I2A_MAX_DIGI-1;
	for (;i >=0 ; i--, val /= 10)
        	buf[i] = "0123456789"[val % 10];

	char p;
	int numZero = 0;
	int s;
	p = buf[0];
	while (p == '0' && numZero < I2A_MAX_DIGI){
		numZero++;
		p = buf[numZero];
	}
	
	/* shift char to clear zeros in front end*/
	for ( s = numZero;s < I2A_MAX_DIGI;s++){
		buf[s - numZero] = buf[s];
	}
	buf[s - numZero] = '\0';

	return buf;
}

char *hextoa(int val, char * buf)
{
        if(val == 0){
		buf = STRING_ZERO;
                return buf;
        }


        int i = I2A_MAX_DIGI-1;
        for (;i >=0 ; i--, val /= 16)
                buf[i] = "0123456789abcdef"[val % 16];

        char p;
        int numZero = 0;
        int s;
        p = buf[0];
        while (p == '0' && numZero < I2A_MAX_DIGI){
                numZero++;
                p = buf[numZero];
        }

        /* shift char to clear zeros in front end*/
        for ( s = numZero;s < I2A_MAX_DIGI;s++){
                buf[s - numZero] = buf[s];
        }
        buf[s - numZero] = '\0';

        return buf;
}


int bounded_strcmp(char * src1 , char * src2 , int bound)
{
	int i = 0;
	char * pSrc1;
	char * pSrc2 ;
	pSrc1 = src1 ;
	pSrc2 = src2 ;

	while(*pSrc1 && *pSrc2 && (i < bound - 1) && (*pSrc1 == *pSrc2))
	{
		i++; pSrc1++; pSrc2++;
	}

	return !(*pSrc1 - *pSrc2); // result : 1 equal ; 0 unequal
}

int strlen(const char * str)
{
	int len = 0;
	while(*str!='\0'){
		str++;
		len++;
	}

	return len;
}

int sprintf(char * dest, const char * format, ... )
{
	va_list args;
	int count = 0;

	char * srcArrPtr;
	char * destArrPtr;	
	srcArrPtr = format;
	destArrPtr = dest;

	typedef union {
		unsigned int 	uni_arg;
		int		int_arg;
		char *		str_arg;
	} unn_arg;

	unn_arg va_item;

	char number[10];
	int numDigit = 0;
	char lenFormat;
	int ptr;

	va_start(args, format);

	while( *srcArrPtr != '\0')
	{
		if(*srcArrPtr == '%')
		{
			srcArrPtr++;
			switch(*srcArrPtr){
				case 'C':
				case 'c':{
					va_item.int_arg = va_arg(args, int);
					*destArrPtr = (char)va_item.int_arg;
					destArrPtr++;
					break;}
				case 'D':
				case 'd':
                                case 'i':{
					va_item.int_arg = va_arg(args, int);
					itoa(va_item.int_arg, destArrPtr);
					destArrPtr +=strlen(destArrPtr);
                                        break;  }
				case 'S':
                                case 's':{
					va_item.str_arg = va_arg(args, char *);
					strcpy(destArrPtr, va_item.str_arg);
					destArrPtr +=strlen(destArrPtr);
                                        break;}
				case 'X':
				case 'x':{
					va_item.int_arg = va_arg(args, int);
                                        hextoa(va_item.int_arg, destArrPtr);
                                        destArrPtr++;
					}
				case 'p':{
					 ptr = va_arg(args, int);
                                        if(ptr == NULL){
                                                strcpy(destArrPtr, "<nil>");
                                                destArrPtr += strlen(destArrPtr);
                                        }else{
                                                itoa(ptr,destArrPtr);
                                                destArrPtr += strlen(destArrPtr);
                                        }
                                        break;
					}
				case 'l':{
					if(*(srcArrPtr+1) == 'u'){
						srcArrPtr++;
					}
					}
				case 'u':{
                                        va_item.int_arg = va_arg(args, int);
                                        itoa(va_item.int_arg, destArrPtr);
                                        destArrPtr +=strlen(destArrPtr);
                                        break;
					}
                              	default:
                                        break;  

			}
		}else{
			*destArrPtr++ = *srcArrPtr;
		}
		srcArrPtr++;
	}
	*destArrPtr = '\0';
	va_end(args);
	return destArrPtr - dest;
}

char * strcat(char * dest, const char * src){

	char * sdest = dest;
	while(*dest)
		dest++;
	while(*dest++ = *src++)
		;

	return sdest;
}

void printf(const char * format, ... )
{

	char dest [100] = {'\0'};

	va_list args;
	int count = 0;

	char * srcArrPtr;
	char * destArrPtr;
	srcArrPtr = format;
	destArrPtr = dest;

	typedef union {
		unsigned int 	uni_arg;
		int		int_arg;
		char *		str_arg;
	} unn_arg;

	unn_arg va_item;

	char number[10];
	int numDigit = 0;
	char lenFormat;
	int ptr;

	va_start(args, format);

	while( *srcArrPtr != '\0')
	{
		if(*srcArrPtr == '%')
		{
			srcArrPtr++;
			switch(*srcArrPtr){
				case 'C':
				case 'c':{
					va_item.int_arg = va_arg(args, int);
					*destArrPtr = (char)va_item.int_arg;
					destArrPtr++;
					break;}
				case 'D':
				case 'd':
                                case 'i':{
					va_item.int_arg = va_arg(args, int);
					itoa(va_item.int_arg, destArrPtr);
					destArrPtr +=strlen(destArrPtr);
                                        break;  }
				case 'S':
                                case 's':{
					va_item.str_arg = va_arg(args, char *);
					strcpy(destArrPtr, va_item.str_arg);
					destArrPtr +=strlen(destArrPtr);
                                        break;}
				case 'X':
				case 'x':{
					va_item.int_arg = va_arg(args, int);
                                        hextoa(va_item.int_arg, destArrPtr);
                                        destArrPtr++;
					}
				case 'p':{
					 ptr = va_arg(args, int);
                                        if(ptr == NULL){
                                                strcpy(destArrPtr, "<nil>");
                                                destArrPtr += strlen(destArrPtr);
                                        }else{
                                                itoa(ptr,destArrPtr);
                                                destArrPtr += strlen(destArrPtr);
                                        }
                                        break;
					}
				case 'l':{
					if(*(srcArrPtr+1) == 'u'){
						srcArrPtr++;
					}
					}
				case 'u':{
                                        va_item.int_arg = va_arg(args, int);
                                        itoa(va_item.int_arg, destArrPtr);
                                        destArrPtr +=strlen(destArrPtr);
                                        break;
					}
                              	default:
                                        break;  

			}
		}else{
			*destArrPtr++ = *srcArrPtr;
		}
		srcArrPtr++;
	}
	*destArrPtr = '\0';
	va_end(args);

	puts(dest);

}
