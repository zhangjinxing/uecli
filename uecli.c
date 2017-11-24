/// \file uecli.c
/// \brief 核心文件
///
/// \details
/// uecli 是嵌入式精简的命令行接口支持模块
/// 其支持命令补全、历史记录、子菜单、长帮助信息等特性
///
/// \author 张诗星
/// \par
/// (C) Copyright 杭州大仁科技有限公司
///
/// \version
/// 2017/07/24 张诗星 初始版本\n

// ********************************************************************************************
// 头文件

#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "uecli.h"

// ********************************************************************************************
// 内部宏定义

// 循环递减
#define CYCLE_DECREMENT(X,MAXVALU) ((X)>0 ? ((X)-1) : (MAXVALU))

// 循环递增
#define CYCLE_INCREMENT(X,MAXVALU) ((X)<(MAXVALU) ? ((X)+1) : 0)

// 无效的历史记录索引
#define INVALID_HISTORY_INDEX   (-1)

// ********************************************************************************************
// 内部类型定义


// 基于索引的字符串类型.
//
// 可快速获取长度，快速附加、删减字符
typedef struct
{
    char str[UECLI_CFG_STR_MAXLEN];    // 命令行输入字符串缓存
    int length;     // 字符串长度
}IdxString;

// cli命令行接口对象
static struct uecli_Type
{
    
    IdxString instring;                     // 用户输入字符串缓存
    char tmpString[UECLI_CFG_TEMP_MAXLEN];   // 内部临时字符串缓存
    char prompt[UECLI_CFG_MAX_POMPTLINE];   // 提示符字符串缓存
    const uecli_MenuItem* cmdlist;            // 命令列表入口指针

    // 输入钩子支持
#if UECLI_01_IN_HOOK>0
    void(*hookfun)(int,const char**) ;  // 命令钩子句柄 
#endif

    // 子菜单支持
#if UECLI_01_SUBMENU>0
    const uecli_MenuItem* stack[UECLI_CFG_STACK_COUNT];   // 子菜单堆栈
    int stackpos;    // 堆栈索引 指向最后空闲单元
#endif

    // 命令行历史纪录支持
#if UECLI_01_HISTORY>0
    char history[UECLI_CFG_HISTORY_COUNT][UECLI_CFG_STR_MAXLEN];   // 历史记录字符串缓存
    int historyEnd;             // 历史记录保存尾部索引
    int historyindex;           // 历史记录查询索引
    char extSequenceState;      // 扩展命令状态支持
#endif

}uecli;

// ********************************************************************************************
// 内部变量

#include "uecli_cmd.h"

void uecli_port_out(const void* buff, uint32_t num);
bool uecli_port_init(void);

// ********************************************************************************************
// 内部函数

// 获取命令行输入字符串
static inline const char* GetCmdlineString(IdxString* pistr)
{
    return pistr->str;
}

// 获取命令行字符串长度
static inline int GetCmdlineLength(IdxString* pistr)
{
    return pistr->length;
}

// 清空输入命令行字符串
static inline void ClearCmdlineString(IdxString* pistr)
{
    pistr->length = 0;
    pistr->str[0] = '\0';
}

// 清空输入命令行显示
static inline void ClearCmdlineInput(IdxString* pistr)
{
    if (pistr->length)
    {
        uecli_snprintf(pistr->str, UECLI_CFG_STR_MAXLEN, "\033[%dD\033[J",
            (uint8_t)pistr->length);
        uecli_PrintString(pistr->str);
    }
    ClearCmdlineString(pistr);
}

// 拷贝字符串到命令行显示
static inline void CopyStringToCmdline(IdxString* pistr, const char* str1)
{
    // 清空之前的输入
    ClearCmdlineInput(pistr);

    // 设置指定的字符串
    if (str1 && '\0' != *str1)
    {
        uecli_strncpy(pistr->str, str1, UECLI_CFG_STR_MAXLEN);
        pistr->length = uecli_strlen(pistr->str);
        uecli_PrintString(pistr->str);
    }
}

// 附加字符
static inline char AppendCmdlinechar(IdxString* pistr, char c)
{
    if (pistr->length < UECLI_CFG_STR_MAXLEN - 1)
    {
        pistr->str[pistr->length++] = c;
        pistr->str[pistr->length] = '\0';
        return c;
    }
    return '\0';
}

// 删除最后一个字符
static inline char DeleteCmdlineChar(IdxString* pistr)
{
    char c = '\0';
    if (pistr->length)
    {
        c = pistr->str[--pistr->length];
        pistr->str[pistr->length] = '\0';
    }
    return c;
}

// 检查字符串是否空白
static inline bool IsBlankString(const char* str)
{
    return (NULL == str || '\0' == *str);
}

// 是否为空白字符
static inline bool IsBlankChar(char c)
{
    return (' ' == c || '\t' == c || '\r' == c);
}

// 获取下一个分割
static char* GetNextDelim(char* instring, char** pstr)
{
    uecli_assert(instring);

    // 跳过所有空白字符
    for (; IsBlankChar(*instring); ++instring);

    register char c = *instring;
    bool quotemask = false;     // 双引号标记

    if ('"' == c)
    {
        c = *(++instring);
        quotemask = true;
    }
    if ('\0' == c)
        return NULL;
            
    // 检测空白字符
    for (*pstr = instring; ; ++instring)
    {
        if ('\0' == (c=*instring))
            return instring;
        else if (quotemask && '"' == c)
            break;
        else if (!quotemask && IsBlankChar(c))
            break;
    }

    *instring = '\0';
    return instring + 1;
}

// 分割字符串
static int SplitString(char* str, char* substr[], int size)
{
    // 参数断言
    uecli_assert(str);
    uecli_assert(substr);

    if (IsBlankString(str) || !substr)
        return 0;

    int i = 0;
    for (; i < size; ++substr)
    {
        if (NULL == (str=GetNextDelim(str, substr)))
            break;
        ++i;
    }
    return i;
}

// 打印提示符
static inline void PrintCLIPrompt(void)
{
    uecli_PrintString(uecli.prompt);
}

// ********************************************************************************************
// 子菜单支持

#if UECLI_01_SUBMENU>0

// 更新cli提示符字符串
static void UpdateCLIPrompt(void)
{
    char* pstr = uecli.prompt;

    uecli_strncpy(pstr, UECLI_NEWLINE, UECLI_CFG_MAX_POMPTLINE);
    
    // 添加子菜单路径
    for (int i = 0; i < uecli.stackpos; ++i)
    {
        uecli_strncat(pstr, "\\", UECLI_CFG_MAX_POMPTLINE);
        uecli_strncat(pstr, uecli.stack[i]->exename, UECLI_CFG_MAX_POMPTLINE);
    }
    uecli_strncat(pstr, STRING_PROMPT, UECLI_CFG_MAX_POMPTLINE);
}

// 栈顶
static inline const uecli_MenuItem* GetMenuStackHandle(void)
{
    if (uecli.stackpos)
        return uecli.stack[uecli.stackpos - 1];
    return NULL;
}

// 菜单堆栈入栈
static inline void PushMenuStack(const uecli_MenuItem* ptr)
{
    // 超过最大数量则覆盖
    if (uecli.stackpos >= UECLI_CFG_STACK_COUNT)
        uecli.stackpos = UECLI_CFG_STACK_COUNT - 1;
    uecli.stack[uecli.stackpos++] = ptr;
    
    UpdateCLIPrompt();
}

// 菜单堆栈出栈
static inline const uecli_MenuItem* PopMenuStack(void)
{
    const uecli_MenuItem* ptr = NULL;
    if (uecli.stackpos)
    {
        ptr = uecli.stack[--uecli.stackpos];
        UpdateCLIPrompt();
    }
    return ptr;
}

#else
//更新cli提示符字符串
static void UpdateCLIPrompt(void)
{
    char* pstr = uecli.prompt;

    uecli_strncpy(pstr, UECLI_NEWLINE, UECLI_CFG_MAX_POMPTLINE);
    uecli_strncat(pstr, STRING_PROMPT, UECLI_CFG_MAX_POMPTLINE);
}
static inline const uecli_MenuItem* PopMenuStack(void)
{
    return NULL;
}
#endif

// 获取用户命令
static inline const uecli_MenuItem* GetUserCmdHandle(void)
{
#if UECLI_01_SUBMENU>0
    const uecli_MenuItem* phand = GetMenuStackHandle();
    if (NULL != phand)
        return (const uecli_MenuItem*)phand->pdata;
#endif
    return uecli.cmdlist;
}

// ********************************************************************************************
// 历史记录支持

#if UECLI_01_HISTORY>0
// 保存命令历史
static void SaveHistory(const char* cmdline)
{
    // 空白字符串直接退出
    if (IsBlankString(cmdline))
        return;

    // 输入的命令行不重复则记录
    int i = uecli.historyEnd;
    if (uecli_strcasecmp(uecli.history[i], cmdline))
    {
        i = CYCLE_INCREMENT(i, UECLI_CFG_HISTORY_COUNT - 1);
        uecli_strncpy(uecli.history[i], cmdline, UECLI_CFG_STR_MAXLEN);
        uecli.historyEnd = i;
    }
    uecli.historyindex = INVALID_HISTORY_INDEX;
}

// 获取历史记录环形列表起始位置
static inline int GetStartHistoryPostion(void)
{
    return **uecli.history == '\0' ? 1 :
        CYCLE_INCREMENT(uecli.historyEnd, UECLI_CFG_HISTORY_COUNT-1);
}

// 获取下一个历史记录
static const char* GetNextHistory(void)
{
    int i = uecli.historyindex;
    int starti = GetStartHistoryPostion();

    // 判断是否为空
    if ('\0' == uecli.history[uecli.historyEnd][0])
        return NULL;
    else if (INVALID_HISTORY_INDEX == i)
        uecli.historyindex = starti;
    else if (i != uecli.historyEnd)
    {
        i = CYCLE_INCREMENT(i, UECLI_CFG_HISTORY_COUNT - 1);
        uecli.historyindex = i;
    }
    
    uecli_assert(uecli.history[uecli.historyindex][0] != '\0');
    return uecli.history[uecli.historyindex];
}

// 获取上一个历史记录
static const char* GetPriveHistory(void)
{
    int i = uecli.historyindex;
    int starti = GetStartHistoryPostion();

    // 判断是否为空
    if ('\0' == uecli.history[uecli.historyEnd][0])
        return NULL;
    else if (INVALID_HISTORY_INDEX == i)
        uecli.historyindex = uecli.historyEnd;
    else if (i != starti)
    {
        i = CYCLE_DECREMENT(i, UECLI_CFG_HISTORY_COUNT - 1);
        uecli.historyindex = i;
    }
    
    uecli_assert(uecli.history[uecli.historyindex][0] != '\0');
    return uecli.history[uecli.historyindex];
}

// 处理扩展字符序列 返回值表示是否回显
static char ProcessESCSequence(char c)
{
    const char* tstr;
    switch (c)
    {
    case '\x1b':    // 起始命令
        uecli.extSequenceState = 1;
        return 0;
        break;
    case '[':
        if (1 == uecli.extSequenceState)
        {
            uecli.extSequenceState = 2;
            return 0;
        }
        break;
    case 'A':       // 上箭头
        if (2 == uecli.extSequenceState)
        {
            uecli.extSequenceState = 0;
            if (!IsBlankString(tstr = GetPriveHistory()))
                CopyStringToCmdline(&uecli.instring, tstr);
            return 0;
        }
        break;
    case 'B':       // 下箭头
        if (2 == uecli.extSequenceState)
        {
            uecli.extSequenceState = 0;
            if (!IsBlankString(tstr = GetNextHistory()))
                CopyStringToCmdline(&uecli.instring, tstr);
            return 0;
        }
        break;
    default:
        break;
    }
    return c;
}
#endif

// ********************************************************************************************
// 命令补全支持

#if UECLI_01_COMPLETE>0

// 按指定长度比较字符串
static int strncmpNocase(const char* src, const char* dst, unsigned int len)
{
    // 参数断言
    uecli_assert(src);
    uecli_assert(dst);

    char c1, c2;
    int ret = 0;

    // 判断指针是否相等
    if (src == dst) return 0;

    for (; c2 = *src, '\0' != (c1 = *dst); ++src, ++dst)
    {
        if (c1 >= 'A' && c1 <= 'Z')
            c1 += 32;
        if (c2 >= 'A' && c2 <= 'Z')
            c2 += 32;

        if (c2 != c1)
            break;
        else if (!--len)
            return 0;
    }

    // 格式化结果数值
    ret = c2 - c1;
    if (ret < 0)
        ret = -1;
    else if (ret > 0)
        ret = 1;
    return(ret);
}

// 打印匹配命令字符串表
static void PrintMatchCmdTable(const char** matchtable, int len)
{
    uecli_PrintString(UECLI_NEWLINE);
    const char* formatstr = NULL;
    
    // 打印列表
    for (int i = 0; i < len; ++i)
    {
        if ((i + 1) % 4 == 0)
            formatstr = "%s" UECLI_NEWLINE;
        else
            formatstr = "%-16s";

        uecli_snprintf(uecli.tmpString, UECLI_CFG_TEMP_MAXLEN, formatstr, matchtable[i]);
        uecli_PrintString(uecli.tmpString);
    }
    
    // 打印提示符
    uecli_PrintString(UECLI_NEWLINE);
    PrintCLIPrompt();
    uecli_PrintString(GetCmdlineString(&uecli.instring));
}

// 搜索匹配命令
static int SearchCompleteCmd(const char** strtable, int startindex, const uecli_MenuItem* list)
{
    int strlength = GetCmdlineLength(&uecli.instring);
    for (; list->pdata; ++list)
    {
        if (!strncmpNocase(GetCmdlineString(&uecli.instring), list->exename, strlength))
        {
            if (startindex >= UECLI_CFG_AUTOCOMP_NUM - 1)
            {
                strtable[UECLI_CFG_AUTOCOMP_NUM - 1] = "...";
                startindex = UECLI_CFG_AUTOCOMP_NUM;
                break;
            }
            strtable[startindex] = list->exename;
            ++startindex;
        }
    }
    return startindex;
}

// 自动补全支持
static void AutoCompleteInstring(void)
{
    // 无输入直接显示帮助信息
    if (!GetCmdlineLength(&uecli.instring))
    {
        uecli_PrintString(UECLI_NEWLINE);
        Cmd_HelpMain(0, 0);
        PrintCLIPrompt();
        return;
    }

    const char* matchStringTable[UECLI_CFG_AUTOCOMP_NUM];
    int i = 0;

    // 匹配系统命令
    i = SearchCompleteCmd(matchStringTable, i, GetSyscmdHandle());
    if (i < UECLI_CFG_AUTOCOMP_NUM)
        i = SearchCompleteCmd(matchStringTable, i, GetUserCmdHandle());

    // 只找到1个匹配 直接完成
    if (1 == i)
        CopyStringToCmdline(&uecli.instring, matchStringTable[0]);
    else if (i)
        PrintMatchCmdTable(matchStringTable, i);
}
#endif

// 回显输入字符
static char cli_echo(char c)
{
    UECLI_LOG("接收到字符0X%02x:%c", c, c);

    // 解析控制命令
    switch (c)
    {
    case '\r':
    case '\n':  // 回车换行
        uecli_PrintString(UECLI_NEWLINE);
        return '\n';
        break;
    case '\b':  // 退格字符
        if (DeleteCmdlineChar(&uecli.instring))
            uecli_PrintString("\b \b");
        break;
        
    #if UECLI_01_COMPLETE>0
    case '\t':  // 制表符
        AutoCompleteInstring();
        break;
    #endif  
    
    #if UECLI_01_HISTORY>0
    case '\x1b': // ESC 扩展功能符
        ProcessESCSequence(c);
        break;
    #endif
    
    default:    // 其他可打印字符
    #if UECLI_01_HISTORY>0
        if (!ProcessESCSequence(c))
            break;
    #endif
        if (uecli_isprintfchar(c) && AppendCmdlinechar(&uecli.instring,c))
            uecli_port_out((const uint8_t*)&c, 1);		/* 输出回显 */
    }

    return c;
}

// 搜索匹配的命令项
static const uecli_MenuItem* SearchMatchCommand(const char* cmdstring)
{
    const uecli_MenuItem* ptr = NULL;
    
    // 搜索系统命令列表
    for (ptr = GetSyscmdHandle(); ptr->pdata; ++ptr)
    {
        if (!uecli_strcasecmp(cmdstring, ptr->exename))
            return ptr;
    }
    
    // 搜索用户命令列表
    for (ptr = GetUserCmdHandle(); ptr->pdata; ++ptr)
    {
        if (!uecli_strcasecmp(cmdstring, ptr->exename))
            return ptr;
    }
    return NULL;
}

// ********************************************************************************************
// 外部接口函数

/// \brief 初始化CLI命令行对象
///
/// \param cmdlist 所支持的命令表
/// \return void
void uecli_Initialize(const uecli_MenuItem* cmdlist)
{
    memset(&uecli,0,sizeof(uecli));
    uecli.cmdlist = cmdlist;

    uecli_port_init();      // 底层接口初始化
    uecli_PrintString(STRING_COPYRIGHT_INFO);
    UpdateCLIPrompt();      // 更新提示符字符串
    PrintCLIPrompt();       // 打印提示符
}

#if UECLI_01_IN_HOOK>0

/// \brief 输入钩子支持 设置后所有的输入数据都传向指定的函数
///
/// \param pfun 钩子处理函数
/// \return void* 之前的钩子指针
void* uecli_SetHook(void(*pfun)(int,char**))
{
    void* temp = (void*)(uecli.hookfun);
    uecli.hookfun = (void(*)(int,const char**))pfun;

    return temp;
}

/// \brief 检查是否设置钩子
///
/// \return bool
bool uecli_IsValidHook(void)
{
    return uecli.hookfun != NULL;
}
#endif

/// \brief 对输入的命令行字符串进行分割并查找命令执行
///
/// \param cmdline 输入的命令字符串
/// \return void
void uecli_ExeCmdine(const char* cmdstring)
{
    char strbuff[UECLI_CFG_STR_MAXLEN];
    char* argcbuff[UECLI_CFG_MAX_ARGNUM];

    // 分割字符串会改变原数据，先拷贝一份出来
    uecli_strncpy(strbuff, cmdstring, UECLI_CFG_STR_MAXLEN);
    int count = SplitString(strbuff, argcbuff, UECLI_CFG_MAX_ARGNUM);

    // 搜索并执行命令
    if (count)
    {
        UECLI_LOG("接收到命令行:%s", cmdstring);

        const uecli_MenuItem* phand = SearchMatchCommand(argcbuff[0]);
        if (NULL != phand)
        {
        #if UECLI_01_HISTORY>0
            SaveHistory(cmdstring);
        #endif
            if (UECLI_TYPE_FUN == phand->itemType)
                ((void(*)(int, char**))(uint32_t)phand->pdata)(count, argcbuff);
            else
            {
            #if UECLI_01_SUBMENU>0
                PushMenuStack(phand);
            #endif
            }
        }
        else
        {
            UECLI_LOG("\"%s\" 不是有效的命令", argcbuff[0]);
            uecli_snprintf(uecli.tmpString, UECLI_CFG_TEMP_MAXLEN,STRING_INVALID_COMMAND, argcbuff[0]);
            uecli_PrintString(uecli.tmpString);
            uecli_PrintString(UECLI_NEWLINE);
        }
    }
    
    // 清除命令行输入 打印提示符
    ClearCmdlineString(&uecli.instring);
    PrintCLIPrompt();
}

/// \brief 处理接收到的字符数据
///
/// \param recchar 接收到的字符数据数组
/// \param len 数据数量
/// \return void
void uecli_ProcessRecChar(const char recchar[], int len)
{
    // 检查是否被锁定
    if (uecli.hookfun)
    {
        uecli.hookfun(len,&recchar);
        return;
    }
    
    // 处理接收到的每个字符
    for (int i = 0; i < len; ++i)
    {
        if ('\n' == cli_echo(recchar[i]))
        {
            /* 执行命令行 */
            uecli_ExeCmdine(GetCmdlineString(&uecli.instring));
            break;
        }
    }
}

/// \brief 打印指定的字符串
///
/// \param str 待打印的字符串
/// \return void
void uecli_PrintString(const char* str)
{
    if (str)
        uecli_port_out(str, uecli_strlen(str));
}

