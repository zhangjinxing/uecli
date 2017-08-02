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

/*----------------------------------------头文件----------------------------------------------*/
#include "estring.h"

static const char UCode[] = "0123456789ABCDEFX";
static const char LCode[] = "0123456789abcdefx";

/*----------------------------------------接口函数----------------------------------------------*/

/******************************************************************************************
说明:			安全拷贝字符串
参数:
// @dest		目的
// @source		源
// @size		目的缓冲区大小
返回值:	void
******************************************************************************************/
void estrncpy(char* dest, const char* source, int size)
{
    //参数断言
    ESTR_ASSERT(dest);
    ESTR_ASSERT(source);

    if (dest != source && size)
    {
        for (; --size && '\0' != (*dest = *source); ++dest, ++source);
        *dest = '\0';
    }
}
/******************************************************************************************
说明:			获取指定的字符串长度
参数:
// @str			所需处理的字符串
返回值:	int		长度结果
******************************************************************************************/
uint32_t estrlen(const char* str)
{
    //参数断言
    ESTR_ASSERT(str);

    const char* eos = str;
    while (*eos++);
    return (uint32_t)(eos - str - 1);
}
/******************************************************************************************
说明:			获取指定的字符串长度
参数:
// @str			所需处理的字符串
// @maxlen      最大允许长度
返回值:	int		长度结果
******************************************************************************************/
uint32_t estrnlen(const char* str, uint32_t maxlen)
{
    register uint32_t i = maxlen;

    //参数断言
    ESTR_ASSERT(str);

    while (*str++ && i)
    {
        --i;
    }

    return (uint32_t)(maxlen - i);
}
/******************************************************************************************
说明:			将字符串安全的添加在指定字符串结尾
参数:
// @dest		目的字符串
// @source		源字符串
// @size		目的缓冲区大小
返回值:	void
******************************************************************************************/
void estrncat(char* dest, const char* source, uint32_t size)
{
    uint32_t len;

    //参数断言
    ESTR_ASSERT(dest);
    ESTR_ASSERT(source);
    ESTR_ASSERT(size);

    len = estrlen(dest);
    ESTR_ASSERT(size >= len + 1);
    if (size >= len + 1)
        estrncpy(&dest[len], source, size - len - 1);
}
/******************************************************************************************
说明:			串联对象数组的各个元素，其中在每个元素之间使用指定的分隔符
参数:
// @dest		目的指针
// @size		缓冲区大小
// @separator	分隔符字符串
// @pstr		字符串数组
// @count		字符串数组大小
返回值:	void
******************************************************************************************/
void estrJoin(char* dest, int size, const char* separator, const char* pstr[], int count)
{
    const char* source;
    char temp;
    int i = 0;

    //参数断言
    ESTR_ASSERT(dest);
    ESTR_ASSERT(size);

    //字符串数组数量为0直接退出
    if (!count || !pstr) return;

    //复制字符串和分隔符
    for (;;)
    {
        source = pstr[i];
        ESTR_ASSERT(source);
        while ('\0' != (temp = *source++))
        {
            if (!--size) { goto STR_JOIN_LABLE_END; }
            *dest++ = temp;
        }
        //检查字符串数量
        if (++i >= count) break;
        //添加分隔符
        if (separator)
        {
            source = separator;
            while ('\0' != (temp = *source++))
            {
                if (!--size) { goto STR_JOIN_LABLE_END; }
                *dest++ = temp;
            }
        }
    }

STR_JOIN_LABLE_END:
    *dest = '\0';
}
/******************************************************************************************
说明:			基于可变参数 串联字符串数组的各个元素
参数:
// @dest		目的指针
// @size		缓冲区大小
// @separator	分隔符字符串
// @count		字符串数量
返回值:	void
******************************************************************************************/
void estrJoinArg(char* dest, int size, const char* separator, int count, ...)
{
    const char* pstr[8];
    register int i;
    va_list argptr;

    ESTR_ASSERT(count <= 8);
    if (!count) return;

    va_start(argptr, count);
    //提取指针
    for (i = 0; i < count; i++)
        pstr[i] = va_arg(argptr, char*);

    //串联字符串
    estrJoin(dest, size, separator, pstr, count);
    va_end(argptr);
}
/******************************************************************************************
说明:			将字符串转换为小写
参数:
// @str		字符串
返回值:	void
******************************************************************************************/
void eStrToLower(char* str)
{
    register char temp;

    ESTR_ASSERT(str);

    while ('\0' != (temp = *str))
    {
        if (temp >= 'A' && temp <= 'Z')
            *str += 'a' - 'A';

        ++str;
    }
}
/******************************************************************************************
说明:		将字符串转换为大写
参数:
// @str		字符串
返回值:	void
******************************************************************************************/
void eStrToUpper(char* str)
{
    register char temp;

    ESTR_ASSERT(str);

    while (0 != (temp = *str))
    {
        if (temp >= 'a' && temp <= 'z')
            *str += 'A' - 'a';

        ++str;
    }
}
/******************************************************************************************
说明:		字符串比较
参数:
// @src		字符串1
// @dst		字符串2
返回值:	int	比较结果值 0相等 1大于 -1小于
******************************************************************************************/
int estrcmp(const char* src, const char* dst)
{
    int ret = 0;
    char temp;

    //参数断言
    ESTR_ASSERT(src);
    ESTR_ASSERT(dst);

    //判断指针是否相等
    if (src == dst) return 0;

    for (; '\0' != (temp = *dst); ++src, ++dst)
    {
        if (0 != (ret = *src - temp))
            break;
    }

    //格式化结果数值
    if (ret < 0)
        ret = -1;
    else if (ret > 0)
        ret = 1;
    return(ret);
}
/******************************************************************************************
说明:		忽略大小写的字符串比较
参数:
// @src		字符串1
// @dst		字符串2
返回值:	int	比较结果值 0相等 1大于 -1小于
******************************************************************************************/
int estrcmpNocase(const char* src, const char* dst)
{
    char temp1, temp2;
    int ret = 0;

    //参数断言
    ESTR_ASSERT(src);
    ESTR_ASSERT(dst);

    //判断指针是否相等
    if (src == dst) return 0;

    for (; temp2 = *src, '\0' != (temp1 = *dst); ++src, ++dst)
    {
        if (temp1 >= 'A' && temp1 <= 'Z')
            temp1 += 32;
        if (temp2 >= 'A' && temp2 <= 'Z')
            temp2 += 32;

        if (temp2 != temp1)
            break;
    }

    //格式化结果数值
    ret = temp2 - temp1;
    if (ret < 0)
        ret = -1;
    else if (ret > 0)
        ret = 1;
    return(ret);
}
/******************************************************************************************
说明:		使用指定的字符分割字符串
参数:
// @str			待分割的字符串
// @separator	分隔符
// @substr		子字符串数组
// @size		子字符串数组大小
返回值:	int		分割出的子字符串数量
******************************************************************************************/
int eStrSplit(char* str, char separator, char* substr[], int size)
{
    int count = 0;
    register char c;
    char ischar = 0;		/* 字符标记 */

                            //参数断言
    ESTR_ASSERT(str);
    ESTR_ASSERT(substr);

    while ('\0' != (c = *str))
    {
        //将控制字符转换为空白
        if (c < ' ' || c > '\x7f')
        {
            *str = ' ';
        }

        //分割字符
        if (' ' == c || separator == c)
        {
            ischar = 0;
            *str = '\0';    /* 将所有的空白填充\0 */
        }
        else
        {
            //在由空格到字符的变化时记录为子字符串
            if (!ischar)
            {
                ischar = 1;
                substr[count++] = str;	/* 记录子字符串指针 */
                ESTR_ASSERT(count < size);
                if (count >= size) break;
            }
        }

        ++str;
    }
    return count;
}
/******************************************************************************************
说明:			搜索字符第一次出现在字符串中的位置
参数:
// @str			待搜索的字符串
// @chr			待搜索的字符
返回值:	char*		查找到的位置指针 0表示为找到
******************************************************************************************/
char* estrchr(const char* str, char chr)
{
    char temp;
    ESTR_ASSERT(str);

    for (; '\0' != (temp = *str); ++str)
    {
        if (temp == chr)
            return((char *)str);
    }

    return (0);
}
/******************************************************************************************
说明:			在字符串后面附加字符使字符串长度达到一定长度
参数:
// @pdes		字符串
// @chr			扩展字符
// @width		所需扩展的宽度
返回值:	void
******************************************************************************************/
void eStrExpand(char* pdes, char chr, int width)
{
    int len = estrlen(pdes);

    //参数检查
    ESTR_ASSERT(pdes);

    if (len >= width) return;

    pdes = &pdes[len];
    width = width - len;
    //拷贝字符串 字符串结束或到达最大长度
    while (--width)
    {
        *pdes = chr;
        ++pdes;
    }
    //设置字符串结束标志
    *pdes = '\0';
}

/*----------------------------------------数据转换----------------------------------------------*/
/******************************************************************************************
说明:字符串转换为整数
--0x或0X开头将会被按16进制转换为数值
--0b或0B开头将会按2进制转换数值
--0后跟数值将会按照8进制转换数值
--否则按照10进制转换数值
参数:
// @str			待转换的字符串
// @varl		待保存的数据指针
返回值:	int		转换结果 0失败 1成功
******************************************************************************************/
int estrtoi(const char *str, int32_t * varl)
{
    char ibase = 10, flags = 0, c;
    int32_t  val = 0;

    //参数断言
    ESTR_ASSERT(str);
    ESTR_ASSERT(varl);

    //跳过前导空格
    while (' ' == *str) ++str;
    *varl = 0;
    c = *str;
    //判断符号标志
    if ('-' == c)
        flags = *str++;
    else if ('+' == c)
        str++;

    c = *str;
    //判断16进制、8进制、二进制
    if ('0' == c)
    {
        c = *++str;			/* 0之后的一个字符 */
        if (c >= 'a') c -= 0x20;		/* 转换为大写字母 */
        if ('X' == c)		/* 0x或0X为16进制 */
        {
            ibase = 16;
            c = *++str;
        }
        else if ('B' == c)	/* 0b或0B为2进制 */
        {
            ibase = 2;
            c = *++str;
        }
        else				/* 否则为8进制 */
            ibase = 8;
    }
    else					/* 10进制 */
        ibase = 10;

    // 转换为数值
    while (0 != (c = *str++))
    {
        // 转换数值
        if (c >= 'a') c -= 0x20;	/* 转换为大写字母 */
        if (c >= '0'&&c <= '9')		/* 数字字符 */
            c -= '0';
        else if (c >= 'A'&&c <= 'F')/* A~F转换位10~15 */
            c -= ('A' - 10);
        else
            return 0;

        //转换为数值
        if (c >= ibase) return 0;		/* 当前基数非法的字符 */
        val = val * ibase + c;
    }

    //负号处理
    if (flags) val = 0 - val;			/* apply sign if needed */
    *varl = val;
    return 1;
}
/******************************************************************************************
说明:			10进制字符转数值
参数:
// @s			字符串
返回值:	int32_t
******************************************************************************************/
int32_t eAtoi(const char* s)
{
    int value;
    char c;
    uint8_t sig = 0;

    //跳过空格
    while (' ' == *s)
        ++s;
    c = *s++;

    //判断符号位
    if ('-' == c)
    {
        sig = 1;
        c = *s++;
    }
    else if ('+' == c)
        c = *s++;

    //转换数值
    for (value = 0; c >= '0'&&c <= '9'; c = *s++)
        value = value * 10 + c - '0';

    if (sig)
        return -value;
    return value;
}
/******************************************************************************************
说明:字符串转换为浮点数
支持 1.0、1.1e1、 +1.1、0.1e5等格式
参数:
// @str			待转换的字符串
// @varl		待保存的数据指针
返回值:	int		转换结果 0失败 1成功
******************************************************************************************/
int estrtof(const char *str, float * varl)
{
    char c;							//字符临时变量
    int8_t digSign = 0, expSign = 0;//符号位和指数符号位
    int8_t pointPos = 0;				//小数系数
    int8_t digcount = 0;				//有效数字计数
    int8_t exp = 0;					//指数
    int value = 0;					//有效数字
    float ftemp = 0.0f;				//浮点数临时变量

                                    //浮点数乘除系数表
    static const float floatExpTable[] =
    {
        1.0f, 1.0E1f, 1.0E2f, 1.0E3f, 1.0E4f,
        1.0E5f, 1.0E6f, 1.0E7f, 1.0E8f, 1.0E9f,
        1.0E10f, 1.0E11f, 1.0E12f, 1.0E13f, 1.0E14f,
        1.0E15f, 1.0E16f, 1.0E17f, 1.0E18f, 1.0E19f,
        1.0E20f, 1.0E21f, 1.0E22f, 1.0E23f, 1.0E24f,
        1.0E25f, 1.0E26f, 1.0E27f, 1.0E28f, 1.0E29f,
        1.0E30f, 1.0E31f, 1.0E32f, 1.0E33f, 1.0E34f,
        1.0E35f, 1.0E36f, 1.0E37f, 1.0E38f
    };

    // 参数断言
    ESTR_ASSERT(str);
    ESTR_ASSERT(varl);

    //跳过所有前空格
    while (' ' == *str) ++str;
    //判断符号位 
    if ('-' == *str)
        digSign = *str++;
    else if ('+' == *str)
        str++;
    *varl = 0.0f;
    while ('0' == *str) ++str;		/* 跳过前导0 */
                                    //转换有效数字
    c = *str;
    for (;;)
    {
        if (c >= '0'&& c <= '9')
        {
            if (++digcount >= 8) return 0;  /* 最大有效数字为8位 */
            value = value * 10 + c - '0';	/* 转换数值 */
            if (pointPos) ++pointPos;       /* 记录小数位的系数 */
        }
        else if (!pointPos && '.' == c)	/* 检查小数点 */
            pointPos = 1;
        else
            break;

        c = *++str;
    }
    //转换指数部分
    if ('e' == c || 'E' == c)
    {
        if (!value) return 0;		//具有e符号时数字位不允许全为0
        if ('-' == (c = *++str))	//判断指数符号
            expSign = *str++;
        else if ('+' == c)
            str++;
        while (0 != (c = *str++))
        {
            if (c >= '0'&& c <= '9')
            {
                exp = exp * 10 + c - '0';
                if (exp > 38) return 0;	//最大指数限制为38
            }
            else
                return 0;
        }
        //指数符号
        if (expSign) exp = -exp;
    }
    else if ('\0' != c)
        return 0;

    // 计算浮点数结果数值
    if (pointPos)
        exp -= (pointPos - 1);
    if (exp > 38 || exp < -38)		/* 指数最大38 最小-38 */
        return 0;
    if (exp < 0)
        ftemp = value / floatExpTable[-exp];
    else
        ftemp = value * floatExpTable[exp];

    //浮点数符号
    if (digSign) ftemp = -ftemp;
    *varl = ftemp;
    return 1;
}
/******************************************************************************************
说明:			自定义函数接口的格式化打印字符
参数:
// @pfun		字符打印函数指针
// @fmt			格式化字符串
返回值:	void
******************************************************************************************/
void efprintf(void(*pfun)(char), const char*fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    evprintf(pfun, fmt, ap);
    va_end(ap);
}

/*----------------------------------------标准打印----------------------------------------------*/

static char* _buff_ptr;
static uint32_t _buff_size = 0;
void putchar(char chr);

/******************************************************************************************
说明:	字符串缓冲区字符打印函数

参数:
// @chr			打印字符
返回值:	void
******************************************************************************************/
static void __e_snputchar_handle(char chr)
{
    if (_buff_size > 1)
    {
        --_buff_size;		/* 最大计数 */
        *_buff_ptr++ = chr;	/* 保存字符 */
    }
    //字符串封尾
    *_buff_ptr = '\0';
}
/******************************************************************************************
说明:	格式打印到缓冲区
支持
%s	--打印字符串	%c打印字符
参数:
// @buff		缓冲区
// @size		缓冲区大小
// @fmt			格式化字符串
返回值:	void
******************************************************************************************/
void esnprintf(char* buff, uint32_t count, const char*fmt, ...)
{
    va_list ap;
    _buff_ptr = buff;
    _buff_size = count;

    //参数断言
    ESTR_ASSERT(buff);
    ESTR_ASSERT(fmt);
    ESTR_ASSERT(count);

    va_start(ap, fmt);
    evprintf(__e_snputchar_handle, fmt, ap);
    va_end(ap);
}
/******************************************************************************************
说明:	标准格式化打印输出函数

参数:
// @fmt		格式化输出字符串
// @...		可变参数表
返回值:	void
******************************************************************************************/
void eprintf(const char*fmt, ...)
{
    va_list ap;

    //参数断言
    ESTR_ASSERT(fmt);

    va_start(ap, fmt);
    evprintf(putchar, fmt, ap);
    va_end(ap);
}
/******************************************************************************************
说明:	格式打印输出函数
支持
%s--打印字符串	%c--打印字符 %x %X %b %B %d %
参数:
// @pfun		字符输出函数指针
// @fmt			格式化字符串
// @arp			可变参数列表
返回值:	void
******************************************************************************************/
void evprintf(void(*pfun)(char), const char*fmt, va_list arp)
{
    int32_t sv;
    uint32_t nv;
    char s[36], *p;
    register char c;
    register uint16_t i;
    uint16_t fg;		/* 标记 */
    uint16_t width;		/* 指定宽度 */
    const char* hcode;

#define FL_SIGN       (0x00001)   /* '+'标记 在输出前放置正或负号 */
#define FL_SIGNSP     (0x00002)   /* ' '标记 在输出前放置空格或负号*/
#define FL_LEFT       (0x00004)   /* '-'标记 左对齐标志 */
#define FL_LEADZERO   (0x00008)   /* '0'标记 放置前导零 */
#define FL_ALTERNATE  (0x00080)   /* '#'标记 变换 */

    //参数断言
    ESTR_ASSERT(pfun);
    ESTR_ASSERT(fmt);

    for (;;)
    {
        // 查找%标志
        while ('%' != (c = *fmt++))
        {	/* 非%字符 直接打印 */
            if (!c) return;
            pfun(c);
        }
        if ('%' == (c = *fmt++))
        {	/* %%直接打印%字符 */
            pfun(c);
            continue;
        }
        // 获取标志符号
        for (fg = 0;;)
        {
            switch (c)
            {
            case ' ':
                fg |= FL_SIGNSP;
                goto NEXT_FLAG;
            case '+':
                fg |= FL_SIGN;
                goto NEXT_FLAG;
            case '-':
                fg |= FL_LEFT;
                goto NEXT_FLAG;
            case '#':
                fg |= FL_ALTERNATE;
                goto NEXT_FLAG;
            case '0':
                fg |= FL_LEADZERO;
                goto NEXT_FLAG;
            }
            break;
        NEXT_FLAG:
            c = *fmt++;
        }

        //最小宽度
        if ('*' == c)
        {
            width = (uint16_t)va_arg(arp, int);
            c = *fmt++;
        }
        else
        {
            for (width = 0; c >= '0' && c <= '9'; c = *fmt++)	/* 最小宽度 */
                width = width * 10 + c - '0';
        }

        //判断结束
        if (!c) return;

        //转换标记
        if (fg & FL_LEFT) { fg &= ~FL_LEADZERO; }
        //处理标准标记
        hcode = UCode;
        p = &s[35]; s[35] = '\0'; i = 0;
        sv = va_arg(arp, int32_t);
        nv = (uint32_t)sv;
        switch (c)
        {
        case 'c':	/* %c 打印字符 */
            pfun((char)sv);
            continue;
        case 's':	/* %s 打印字符串 */
            p = (char*)sv;		    /* 字符串指针参数 */
            ESTR_ASSERT(p);
            for (; p[i]; ++i);		/* 计算字符串长度 */
            goto PRINT_STRING;
        case 'x':	/* %x 16进制显示 */
            hcode = LCode;
        case 'X':	/* %X 16进制大写显示 */
            if (fg&FL_ALTERNATE) { pfun('0'); pfun(hcode[16]); i += 2; }
            if (!nv) { *--p = '0'; ++i; }
            else for (; nv; ++i) { *--p = hcode[nv & 0x0f]; nv >>= 4; }
            goto PRINT_STRING;
        case 'b':	/* %b 2进制 */
            hcode = LCode;
        case 'B':
            if (fg&FL_ALTERNATE) { pfun('0'); pfun(hcode[0x0b]); i += 2; }
            if (!nv) { *--p = '0'; ; ++i; }
            else for (; nv; ++i) { *--p = hcode[nv & 0x01]; nv >>= 1; }
            goto PRINT_STRING;
        case 'o':	/* %o 8进制显示 */
            if (sv)
            {
                if (sv < 0) { nv = -sv; }
                for (; nv; ++i) { *--p = hcode[nv & 0x07]; nv >>= 3; }
            }
            if (fg&FL_ALTERNATE) { *--p = '0'; ++i; }
            if (sv < 0) { *--p = '-'; ++i; }
            goto PRINT_STRING;
        case 'd':
            if (!sv) { *--p = '0'; ++i; }
            else
            {
                if (sv < 0) { nv = -sv; }
                for (; nv; ++i) { *--p = hcode[nv % 10]; nv /= 10; }
                if (sv < 0) { *--p = '-'; ++i; }
            }
            goto PRINT_STRING;
        default:
            pfun(c); continue;
        }
        //对字符串进行打印操作
    PRINT_STRING:
        c = fg&FL_LEADZERO ? '0' : ' ';     /* 填充字符 */
        while (!(fg&FL_LEFT) && i++ < width) pfun(c);	/* 非左对齐填充左字符 */
        while (*p) pfun(*p++);			    /* 打印字符串 */
        while (i++ < width) pfun(' ');		/* 填充右空格 */
        continue;
    }
}
/******************************************************************************************
说明:	打印内存数据 分别以十六进制和字符模式打印
00000000: 01 02 03 04 05 06 07 08 01 02 03 04 05 06 07 08 ................
支持
%s	--打印字符串	%c打印字符
参数:
// @pbuff			内存指针
// @size			数量
// @addr			打印显示地址
// @linsize			每行打印数量
返回值:	void
******************************************************************************************/
void ePrintMem(uint8_t* pbuff, uint32_t size, uint32_t addr, uint8_t linsize)
{
    uint32_t cnum;
    register uint8_t c;
    register uint32_t i;

    //参数断言
    ESTR_ASSERT(linsize);

    for (; size; size -= cnum, addr += cnum)
    {
        //打印地址
        cnum = addr;
        for (i = 0; i < 8; ++i)
        {
            putchar(UCode[cnum >> 28]);	/* 先打印高HEX字符 */
            cnum <<= 4;				    /* 为下次打印做准备 */
        }
        putchar(':');
        putchar(' ');
        cnum = size < linsize ? size : linsize; /* 计算所需打印字符数量 */

        //按hex格式打印数据
        for (i = 0; i < cnum; ++i)
        {
            c = pbuff[i];
            putchar(UCode[c >> 4]);
            putchar(UCode[c & 0x0f]);
            putchar(' ');
        }
        //打印对齐空白
        for (i = linsize-cnum; i; --i)
        {
            putchar(' '); putchar(' '); putchar(' ');
        }

        //按ASCII码打印数据
        for (i = 0, c = 0; i < cnum; ++i)
        {
            c = *pbuff++;
            if (c<' ' || c>'\x7f') c = '.';
            putchar(c);	    /* 打印字符 */
        }
        putchar('\n');
    }
}

