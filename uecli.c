/**
 * @file     uecli.c
 * @brief    精小嵌入式cli命令行接口模块核心文件
 * @author   张诗星
 * @par
 * (C) Copyright 杭州大仁科技有限公司
 * @version
 * 2017/07/28 张诗星 修订说明\n
 *
 */ 

// ********************************************************************************************
// 头文件

#include <assert.h>
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

/**
* @brief 基于索引的字符串类型.
*
* 可快速获取长度，快速附加、删减字符
*/
typedef struct
{
    /* 命令行输入字符串缓存 */
    char str[UECLI_CFG_MAX_CMDLINE];
    /* 字符串长度 */
    int length;
}IdxString;

/**
* @brief uecli精小命令行对象类型.
*
*/
typedef struct
{
    /* 用户输入字符串缓存 */
    IdxString instring;
    /* 内部临时字符串缓存 */
    char tmpString[TEMP_STRING_LEN];
    /* 提示符字符串缓存 */
    char prompt[UECLI_CFG_MAX_POMPTLINE];
    /* 命令列表入口指针 */
    const uecli_Handle* cmdtable;

    /* 子菜单支持 */
#if UECLI_CFG_SUBMENU>0
    /* 子菜单堆栈 */
    const uecli_Handle* stack[UECLI_CFG_STACK_COUNT];
    /* 堆栈索引 指向最后空闲单元 */
    int stackpos;
#endif

    /* 命令行历史纪录支持 */
#if UECLI_CFG_HISTORY>0
    /* 历史记录字符串缓存 */
    char history[UECLI_CFG_HISTORY_COUNT][UECLI_CFG_MAX_CMDLINE];
    /* 历史记录保存尾部索引 */
    int historyEnd;
    /* 历史记录查询索引 */
    int historyindex;
    /* 扩展命令状态支持 */
    char extSequenceState;
#endif
    /* 命令行锁定支持 */
    const uecli_Handle* lockHandle;  /* 命令锁定句柄 */
}uecli_Type;

// ********************************************************************************************
// 内部变量

// cli命令行接口对象
static uecli_Type uecli;

// ********************************************************************************************
// 引用声明
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
        uecli_snprintf(pistr->str, UECLI_CFG_MAX_CMDLINE, "\033[%dD\033[J",
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

    //设置指定的字符串
    if (str1 && '\0' != *str1)
    {
        pistr->length = uecli_strlen(str1);
        uecli_strncpy(pistr->str, str1, UECLI_CFG_MAX_CMDLINE);
        uecli_PrintString(pistr->str);
    }
}
// 附加字符
static inline char AppendCmdlinechar(IdxString* pistr, char c)
{
    if (pistr->length < UECLI_CFG_MAX_CMDLINE - 1)
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
static char* GetNextDelim(char* strline, char** pstr)
{
    assert(strline);

    // 跳过所有空白字符
    for (; IsBlankChar(*strline); ++strline);

    // 遇到字符串结尾直接返回NULL
    if ('\0' == *strline) return NULL;
    *pstr = strline;

    bool mask = false;
    // 结尾封0
    for (char c; ; ++strline)
    {
        if ('\0' == (c = *strline))
            return strline;
        if (c == '"')
            mask = !mask;
        else if (!mask && IsBlankChar(c))
            break;
    }

    *strline = '\0';
    return strline + 1;
}
// 分割字符串
static int SplitString(char* str, char* substr[], int size)
{
    // 参数断言
    assert(str);
    assert(substr);

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
// 子命令支持

#if UECLI_CFG_SUBMENU>0

//更新cli提示符字符串
static void UpdateCLIPrompt(void)
{
    char* pstr = uecli.prompt;

    uecli_strncpy(pstr, UECLI_NEWLINE, UECLI_CFG_MAX_POMPTLINE);
    /* 添加子菜单路径 */
    for (int i = 0; i < uecli.stackpos; ++i)
    {
        uecli_strncat(pstr, "\\", UECLI_CFG_MAX_POMPTLINE);
        uecli_strncat(pstr, uecli.stack[i]->exename, UECLI_CFG_MAX_POMPTLINE);
    }
    uecli_strncat(pstr, STRING_PROMPT, UECLI_CFG_MAX_POMPTLINE);
}
// 获取栈顶
static inline const uecli_Handle* GetMenuStackHandle(void)
{
    if (uecli.stackpos)
        return uecli.stack[uecli.stackpos - 1];
    return NULL;
}
// 入栈
static inline void PushMenuStack(const uecli_Handle* ptr)
{
    /* 超过最大数量则覆盖 */
    if (uecli.stackpos >= UECLI_CFG_STACK_COUNT)
        uecli.stackpos = UECLI_CFG_STACK_COUNT - 1;
    uecli.stack[uecli.stackpos++] = ptr;
    UpdateCLIPrompt();
}
// 出栈
static inline const uecli_Handle* PopMenuStack(void)
{
    const uecli_Handle* ptr = NULL;
    if (uecli.stackpos)
    {
        ptr = uecli.stack[--uecli.stackpos];
        UpdateCLIPrompt();
    }
    return ptr;
}

#else
static inline const uecli_Handle* PopMenuStack(void)
{
    return NULL;
}
static inline void PushMenuStack(const uecli_Handle* ptr)
{
}
//更新cli提示符字符串
static void UpdateCLIPrompt(void)
{
    char* pstr = uecli.prompt;

    uecli_strncpy(pstr, UECLI_NEWLINE, UECLI_CFG_MAX_POMPTLINE);
    uecli_strncat(pstr, STRING_PROMPT, UECLI_CFG_MAX_POMPTLINE);
}
#endif

// 获取用户命令
static inline const uecli_Handle* GetUserCmdHandle(void)
{
#if UECLI_CFG_SUBMENU>0
    const uecli_Handle* phand = GetMenuStackHandle();
    if (NULL != phand)
        return (const uecli_Handle*)phand->pdata;
#endif
    return uecli.cmdtable;
}

// ********************************************************************************************
// 历史记录支持

#if UECLI_CFG_HISTORY>0
// 保存命令历史
static void SaveHistory(const char* cmdline)
{
    /* 空白字符串直接退出 */
    if (IsBlankString(cmdline))
        return;

    /* 输入的命令行不重复则记录 */
    int i = uecli.historyEnd;
    if (uecli_strcasecmp(uecli.history[i], cmdline))
    {
        i = CYCLE_INCREMENT(i, UECLI_CFG_HISTORY_COUNT - 1);
        uecli_strncpy(uecli.history[i], cmdline, UECLI_CFG_MAX_CMDLINE);
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
    register int i = uecli.historyindex;
    int starti = GetStartHistoryPostion();

    /* 判断是否为空 */
    if ('\0' == uecli.history[uecli.historyEnd][0])
        return NULL;
    else if (INVALID_HISTORY_INDEX == i)
        uecli.historyindex = starti;
    else if (i != uecli.historyEnd)
    {
        i = CYCLE_INCREMENT(i, UECLI_CFG_HISTORY_COUNT - 1);
        uecli.historyindex = i;
    }
    assert(uecli.history[uecli.historyindex][0] != '\0');
    return uecli.history[uecli.historyindex];
}
// 获取上一个历史记录
static const char* GetPriveHistory(void)
{
    register int i = uecli.historyindex;
    int starti = GetStartHistoryPostion();

    /* 判断是否为空 */
    if ('\0' == uecli.history[uecli.historyEnd][0])
        return NULL;
    else if (INVALID_HISTORY_INDEX == i)
        uecli.historyindex = uecli.historyEnd;
    else if (i != starti)
    {
        i = CYCLE_DECREMENT(i, UECLI_CFG_HISTORY_COUNT - 1);
        uecli.historyindex = i;
    }
    assert(uecli.history[uecli.historyindex][0] != '\0');
    return uecli.history[uecli.historyindex];
}
// 处理扩展字符序列
static char ProcessESCSequence(char c)
{
    const char* tstr;
    switch (c)
    {
    case '\x1b':    /* 起始命令 */
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
    case 'A':       /* 上箭头 */
        if (2 == uecli.extSequenceState)
        {
            uecli.extSequenceState = 0;
            if (!IsBlankString(tstr = GetPriveHistory()))
                CopyStringToCmdline(&uecli.instring, tstr);
            return 0;
        }
        break;
    case 'B':       /* 下箭头 */
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

#define PROCESS_ESC_SEQUENCE(c) ProcessESCSequence(c)
#define SAVE_HISTORY(cmd)  SaveHistory(cmd)
#else
#define PROCESS_ESC_SEQUENCE(c) (1)
#define SAVE_HISTORY(cmd)
#endif

// ********************************************************************************************
// 命令补全支持

#if UECLI_CFG_COMPLETE>0
// 返回匹配字符串长度 1完全匹配 0部分匹配 -1不匹配
static int StringIsMatch(const char* str1, const char* str2)
{
    assert(str1);
    assert(str2);

    if (str1 == str2) return 1;

    for (char c; ; ++str1, ++str2)
    {
        if ('\0' == (c=*str1))
        {
            if ('\0' == *str2)
                return 1;
            return 0;
        }
        else if (c != *str2)
            break;
    }
    return -1;
}
// 打印匹配命令字符串表
static void PrintMatchCmdTable(const char** matchtable, int len)
{
    uecli_PrintString(UECLI_NEWLINE);
    const char* formatstr = NULL;
    /* 打印列表 */
    for (int i = 0; i < len; ++i)
    {
        if ((i + 1) % 4 == 0)
            formatstr = "%s" UECLI_NEWLINE;
        else
            formatstr = "%-16s";

        uecli_snprintf(uecli.tmpString, TEMP_STRING_LEN, formatstr, matchtable[i]);
        uecli_PrintString(uecli.tmpString);
    }
    /* 打印提示符 */
    uecli_PrintString(UECLI_NEWLINE);
    PrintCLIPrompt();
    uecli_PrintString(GetCmdlineString(&uecli.instring));
}
// 搜索匹配命令
static int SearchAutoCompleteCommand(const char** strtable, int idx, const uecli_Handle* phand)
{
    for (; phand->pdata; ++phand)
    {
        if (!StringIsMatch(GetCmdlineString(&uecli.instring), phand->exename))
        {
            if (idx >= UECLI_CFG_AUTOCOMP_NUM - 1)
            {
                strtable[UECLI_CFG_AUTOCOMP_NUM - 1] = "...";
                idx = UECLI_CFG_AUTOCOMP_NUM;
                break;
            }
            strtable[idx] = phand->exename;
            ++idx;
        }
    }
    return idx;
}
// 自动补全支持
static void AutoCompleteInstring(void)
{
    /* 无输入直接显示帮助信息 */
    if (!GetCmdlineLength(&uecli.instring))
    {
        uecli_PrintString(UECLI_NEWLINE);
        Cmd_HelpMain(0, 0);
        PrintCLIPrompt();
        return;
    }

    const char* matchStringTable[UECLI_CFG_AUTOCOMP_NUM];
    int i = 0;

    /* 匹配系统命令 */
    i = SearchAutoCompleteCommand(matchStringTable, i, GetSyscmdHandle());
    if (i < UECLI_CFG_AUTOCOMP_NUM)
        i = SearchAutoCompleteCommand(matchStringTable, i, GetUserCmdHandle());


    /* 只找到1个匹配 直接完成 */
    if (1 == i)
        CopyStringToCmdline(&uecli.instring, matchStringTable[0]);
    else if (i)
        PrintMatchCmdTable(matchStringTable, i);
}
#else
static void AutoCompleteInstring(void)
{
}
#endif

// 回显输入字符
static char cli_echo(char c)
{
    UECLI_LOG("接收到字符0X%02x:%c", c, c);

    //解析控制命令
    switch (c)
    {
    case '\r':
    case '\n':  /* 回车换行 */
        uecli_PrintString(UECLI_NEWLINE);
        return '\n';
        break;
    case '\b':  /* 退格字符 */
        if (DeleteCmdlineChar(&uecli.instring))
            uecli_PrintString("\b \b");
        break;
    case '\t':  /* 制表符 */
        AutoCompleteInstring();
        break;
    case '\x1b': /* ESC 扩展功能符 */
        PROCESS_ESC_SEQUENCE(c);
        break;
    default:    /* 其他可打印字符 */
        if (uecli_isprintfchar(c) && PROCESS_ESC_SEQUENCE(c) && AppendCmdlinechar(&uecli.instring,c))
            uecli_port_out((const uint8_t*)&c, 1);		/* 输出回显 */
    }

    return c;
}
// 搜索匹配的命令项
static const uecli_Handle* SearchMatchCommand(const char* cmdline)
{
    const uecli_Handle* ptr = NULL;
    /* 搜索系统命令列表 */
    for (ptr = GetSyscmdHandle(); ptr->pdata; ++ptr)
    {
        if (!uecli_strcasecmp(cmdline, ptr->exename))
            return ptr;
    }
    /* 搜索用户命令列表 */
    for (ptr = GetUserCmdHandle(); ptr->pdata; ++ptr)
    {
        if (!uecli_strcasecmp(cmdline, ptr->exename))
            return ptr;
    }
    return NULL;
}

// ********************************************************************************************
// 外部接口函数

/**
 * @brief   对输入的命令行字符串进行分割并查找命令执行
 *
 * @param[in]   cmdline   输入的命令字符串
 * @return  void
 */
void uecli_ExeCmdine(const char* cmdstring)
{
    char strbuff[UECLI_CFG_MAX_CMDLINE];
    char* argcbuff[UECLI_CFG_MAX_ARGNUM];

    /* 分割字符串会改变原数据，先拷贝一份出来 */
    uecli_strncpy(strbuff, cmdstring, UECLI_CFG_MAX_CMDLINE);
    int count = SplitString(strbuff, argcbuff, UECLI_CFG_MAX_ARGNUM);

    /* 搜索并执行命令 */
    if (count)
    {
        UECLI_LOG("接收到命令行:%s", cmdstring);

        const uecli_Handle* phand = SearchMatchCommand(argcbuff[0]);
        if (NULL != phand)
        {
            SAVE_HISTORY(cmdstring);
            if (UECLI_TYPE_FUN == phand->itemType)
                ((void(*)(int, char**))(uint32_t)phand->pdata)(count, argcbuff);
            else
            {
                PushMenuStack(phand);
            }
        }
        else
        {
            UECLI_LOG("\"%s\" 不是有效的命令", argcbuff[0]);
            uecli_snprintf(uecli.tmpString, TEMP_STRING_LEN,STRING_INVALID_COMMAND, argcbuff[0]);
            uecli_PrintString(uecli.tmpString);
        }
        
    }
    /* 清除命令行输入 打印提示符 */
    ClearCmdlineString(&uecli.instring);
    PrintCLIPrompt();
}
/**
 * @brief      处理接收到的字符数据
 *
 * @param[in]  recchar   接收到的字符数据数组
 * @param[in]  len       数据数量
 * @return     void
 */
void uecli_ProcessRecChar(const char recchar[], int len)
{
    //处理接收到的每个字符
    for (int i = 0; i < len; ++i)
    {
        /* 回显处理 */
        if ('\n' == cli_echo(recchar[i]))
        {
            /* 执行命令行 */
            uecli_ExeCmdine(GetCmdlineString(&uecli.instring));
            break;
        }
    }
}
/**
 * @brief      打印指定的字符串
 *
 * @param[in]  str      待打印的字符串
 * @return     void
 */
void uecli_PrintString(const char* str)
{
    if (str)
        uecli_port_out(str, uecli_strlen(str));
}
/**
 * @brief      初始化CLI命令行对象
 *
 * @param[in]  phand     所支持的命令表
 * @return     void
 */
void uecli_Initialize(const uecli_Handle* phand)
{
    memset(&uecli,0,sizeof(uecli));
    uecli.cmdtable = phand;

    uecli_port_init();      // 底层接口初始化
    uecli_PrintString(STRING_COPYRIGHT_INFO);
    UpdateCLIPrompt();      // 更新提示符字符串
    PrintCLIPrompt();       // 打印提示符
}

