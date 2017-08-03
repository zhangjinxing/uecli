/***********************************************************************************************
_____________________________________杭州大仁科技有限公司_______________________________________
@文 件 名: estring.c
@日    期: 2016.08.02
@作    者: 张诗星

@文件说明:

estring是嵌入式安全字符串操作函数库
所有的复制、附加等字符串操作均会指定缓冲区的大小，防止缓冲区溢出错误

@修订说明:

2016.08.02	初始版本

***********************************************************************************************/
#ifndef _E_STRING_H_
#define _E_STRING_H_

/*-----------------------------------------头文件-----------------------------------------------*/
#include <stdint.h>
#include <stdarg.h>
#include <assert.h>


#define ISDIGIT(c) ( ((c) >= '0') && ((c) <= '9') )

#define ESTR_ASSERT(X) assert(X)
/*----------------------------------------接口函数----------------------------------------------*/

//兼容C C++混合编程
#ifdef __cplusplus
extern "C" {
#endif

unsigned int estrncpy(char* dest, const char* source, unsigned int size);
unsigned int estrlen(const char* str);
uint32_t estrnlen(const char* str, uint32_t maxlen);
void estrncat(char* dest, const char* source, unsigned int size);
void estrJoin(char* dest, unsigned int size, const char* separator, const char* pstr[], unsigned int count);
void estrJoinArg(char* dest, unsigned int size, const char* separator, unsigned int count, ...);
void eStrToLower(char* str);
void eStrToUpper(char* str);
int estrcmp(const char* src, const char* dst);
int estrcmpNocase(const char* src, const char* dst);
unsigned int eStrSplit(char* str, char separator, char* substr[], unsigned int size);
char* estrchr(const char* str, char chr);
void eStrExpand(char* pdes, char chr, int width);
int estrtoi(const char *str, int32_t * varl);
int estrtof(const char *str, float * varl);
void evprintf(void(*pfun)(char), const char*fmt, va_list arp);
void eprintf(const char*fmt, ...);
void esnprintf(char* buff, uint32_t size, const char*fmt, ...);
void ePrintMem(uint8_t* pbuff, uint32_t size, uint32_t addr, uint8_t linsize);

	//兼容C C++混合编程
#ifdef __cplusplus 
}
#endif

#endif
