/**
 * @file     uecli_cmd.h
 * @brief    uecli内部系统命令实现
 * @author   张诗星
 * @par
 * (C) Copyright 杭州大仁科技有限公司
 * @version
 * 2017/07/24 张诗星 修订说明\n
 *
 */

#include "uecli_cfg.h"

// ********************************************************************************************
// 字符串表

// 命令提示符
const char* STRING_PROMPT = ">>";

// 版权描述信息
const char* STRING_COPYRIGHT_INFO =
"\r\n**********************************************************"
"\r\n*                 UECLI 嵌入式控制台界面                 *"
"\r\n**********************************************************\r\n";

// 错误命令提示
const char* STRING_INVALID_COMMAND = "\"%s\" 不是有效的命令！\r\n";

// 版本描述
const char* STRING_VERSIONS =
"Versions: V0.1 Copyright2016 \r\n"
"email: zhangjinxing_2006@163.com\r\n"
__DATE__ " -- " __TIME__"\r\n";

//清屏命令字符串
const char* STRING_CLEAR_SCREEN = "\033[2J\033[0;0H";

//命令列表字符串
const char* STRING_CMD_LIST = "%-16s %s\r\n";

// ********************************************************************************************
// 系统命令支持

extern const uecli_Handle uecli_syscmdList[];
static const uecli_Handle* GetUserCmdHandle(void);
static const uecli_Handle* PopMenuStack(void);
static const uecli_Handle* SearchMatchCommand(const char* cmdline);

// 打印支持的命令列表
static void PrintCommandList()
{
    const uecli_Handle* ptr = uecli_syscmdList;
    for (; ptr->pdata; ++ptr)
    {
        uecli_snprintf(uecli.tmpString, TEMP_STRING_LEN, STRING_CMD_LIST,
            ptr->exename, ptr->desc);
        uecli_PrintString(uecli.tmpString);
    }
    ptr = GetUserCmdHandle();
    for (; ptr->pdata; ++ptr)
    {
        uecli_snprintf(uecli.tmpString, TEMP_STRING_LEN, STRING_CMD_LIST,
            ptr->exename, ptr->desc);
        uecli_PrintString(uecli.tmpString);
    }
}
// help命令
static void Cmd_HelpMain(int argc, char* argv[])
{
#if UECLI_CFG_LONG_HELP>0
    if (argc >= 2)
    {
        const uecli_Handle* phand = SearchMatchCommand(argv[1]);
        if (phand && UECLI_TYPE_FUN == phand->itemType)
        {
            if (phand->helpstr)
                uecli_PrintString(phand->helpstr);
        }
        else
        {
            uecli_snprintf(uecli.tmpString, TEMP_STRING_LEN, STRING_INVALID_COMMAND, argv[1]);
            uecli_PrintString(uecli.tmpString); /* 输出信息 */
        }
        return;
    }
#endif

    //打印系统命令帮助信息
    uecli_PrintString(STRING_COPYRIGHT_INFO);
    PrintCommandList();
}
// ver版本信息命令住函数
static void Cmd_VerMain(int argc, char* argv[])
{
    uecli_PrintString(STRING_VERSIONS);

    argc = argc;
    argv = argv;
}
// CLEAR命令
static void Cmd_ClearMain(int argc, char* argv[])
{
    uecli_PrintString(STRING_CLEAR_SCREEN);
    argc = argc;
    argv = argv;
}
// ..返回上一级命令
static void Cmd_UpperMain(int argc, char* argv[])
{
    PopMenuStack();
    argc = argc;
    argv = argv;
}
// 系统命令表
const uecli_Handle uecli_syscmdList[] =
{
    UECLI_DECLARE_COMMAND(Cmd_UpperMain,   "..",       "返回上级子菜单"),
    UECLI_DECLARE_COMMAND(Cmd_HelpMain,    "help",     "显示支持的命令列表"),
    UECLI_DECLARE_COMMAND(Cmd_VerMain,     "ver",      "显示版本和帮助信息"),
    UECLI_DECLARE_COMMAND(Cmd_ClearMain,   "cls",      "清空屏幕"),
    UECLI_ITEM_END()
};