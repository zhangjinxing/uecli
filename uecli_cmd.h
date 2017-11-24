/// \file uecli_cmd.h
/// \brief 系统命令支持
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
 
#include "uecli.h"

// ********************************************************************************************
// 字符串表

// 命令提示符
const char* STRING_PROMPT = ">>";

// 版权描述信息
const char* STRING_COPYRIGHT_INFO =
"\r\n**********************************************************"
"\r\n*                 UECLI 嵌入式控制台界面                 *"
"\r\n**********************************************************" UECLI_NEWLINE;

// 错误命令提示
const char* STRING_INVALID_COMMAND = "\"%s\" 不是有效的命令！";

// 版本描述
const char* STRING_VERSIONS =
"Versions: V0.2 Copyright2016" UECLI_NEWLINE
"email: zhangjinxing_2006@163.com" UECLI_NEWLINE
__DATE__ " -- " __TIME__ UECLI_NEWLINE;

//清屏命令字符串
const char* STRING_CLEAR_SCREEN = "\033[2J\033[0;0H";

//命令列表字符串
const char* STRING_CMD_LIST = "%-16s";
//子菜单打印字符串
const char* STRING_SUBMENU_LIST = "/%-15s";

// ********************************************************************************************
// 系统命令支持

static inline const uecli_MenuItem* GetSyscmdHandle(void);
static const uecli_MenuItem* GetUserCmdHandle(void);
static const uecli_MenuItem* PopMenuStack(void);
static const uecli_MenuItem* SearchMatchCommand(const char* cmdline);

// 打印命令项列表
static void PrintHandleList(const uecli_MenuItem* ptr)
{
    const char* strformat;
    
    if (ptr)
    {
        for (; ptr->pdata; ++ptr)
        {
            strformat = (ptr->itemType==UECLI_TYPE_SUBMENU) ? 
                STRING_SUBMENU_LIST : STRING_CMD_LIST;
            uecli_snprintf(uecli.tmpString, UECLI_CFG_TEMP_MAXLEN, strformat,ptr->exename);
            uecli_PrintString(uecli.tmpString);

            // 打印帮助信息
            uecli_PrintString(ptr->desc);
            uecli_PrintString(UECLI_NEWLINE);
        }
    }
}

// help命令
static void Cmd_HelpMain(int argc, char* argv[])
{
    // 长帮助信息
#if UECLI_01_LONG_HELP>0
    if (argc >= 2)
    {
        const uecli_MenuItem* pitem = SearchMatchCommand(argv[1]);
        if (pitem && UECLI_TYPE_FUN == pitem->itemType)
        {
            if (pitem->helpstr)
                uecli_PrintString(pitem->helpstr);
        }
        else
        {
            uecli_snprintf(uecli.tmpString, UECLI_CFG_TEMP_MAXLEN, STRING_INVALID_COMMAND, argv[1]);
            uecli_PrintString(uecli.tmpString);
        }
        
        uecli_PrintString(UECLI_NEWLINE);
        return;
    }
#endif

    //打印系统命令帮助信息
    uecli_PrintString(STRING_COPYRIGHT_INFO);
    
    PrintHandleList(GetSyscmdHandle());     // 打印系统命令表
    PrintHandleList(GetUserCmdHandle());    // 打印用户命令表

    (void)argc;
    (void)argv;
}

// ver版本信息命令住函数
static void Cmd_VerMain(int argc, char* argv[])
{
    uecli_PrintString(STRING_VERSIONS);

    (void)argc;
    (void)argv;
}

// CLEAR命令
static void Cmd_ClearMain(int argc, char* argv[])
{
    uecli_PrintString(STRING_CLEAR_SCREEN);
    (void)argc;
    (void)argv;
}

// ..返回上一级命令
static void Cmd_UpperMain(int argc, char* argv[])
{
    PopMenuStack();
    (void)argc;
    (void)argv;
}

// 系统命令表
static const uecli_MenuItem uecli_syscmdList[] =
{
    UECLI_DECLARE_COMMAND(Cmd_UpperMain,   "..",       "返回上级子菜单"),
    UECLI_DECLARE_COMMAND(Cmd_HelpMain,    "help",     "显示支持的命令列表"),
    UECLI_DECLARE_COMMAND(Cmd_VerMain,     "ver",      "显示版本和帮助信息"),
    UECLI_DECLARE_COMMAND(Cmd_ClearMain,   "cls",      "清空屏幕"),
    UECLI_DECLARE_END()
};

static inline const uecli_MenuItem* GetSyscmdHandle(void)
{
    return uecli_syscmdList;
}


