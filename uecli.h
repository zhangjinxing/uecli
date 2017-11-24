/// \file uecli.h
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

#ifndef _U_E_CLI_H_
#define _U_E_CLI_H_

// ********************************************************************************************
// 头文件

#include <stdint.h>
#include <stdbool.h>
#include "uecli_cfg.h"

// ********************************************************************************************
// 类型定义

/// \brief 命令菜单项条目类型
///
typedef enum
{
    UECLI_TYPE_FUN = 0,     ///< 该条目为一个功能性的命令
    UECLI_TYPE_SUBMENU      ///< 该条目为子菜单
}uecli_ItemType;

/// \brief 命令列表类型定义
///
/// 用户必须定义命令列表数组，并在初始化时引用该数组
/// 数组为包含支持的各种命令的列表
typedef struct
{
    const void* pdata;          ///< 指向命令函数或子菜单命令表的数据指针
    uecli_ItemType itemType;    ///< 表示当前条目类型，命令or子菜单
    const char* exename;        ///< 用于匹配执行命令输入的字符串
    const char* desc;           ///< 显示在help命令中的说明字符串
    
#if UECLI_01_LONG_HELP>0
    const char* helpstr;        ///< 使用help+命令时显示的长帮助信息
#endif
}uecli_MenuItem;

/// \defgroup  UECLI_DECLARE命令接口声明定义
/// \{
#if UECLI_01_LONG_HELP>0     
#define UECLI_DECLARE_COMMAND(PDATA, EXENAME, HELPSTR, ...) \
    {(void*)PDATA, UECLI_TYPE_FUN, EXENAME, HELPSTR, ##__VA_ARGS__}

#define UECLI_DECLARE_SUBMENU(PDATA, MENUNAME, HELPSTR) \
    {(void*)PDATA, UECLI_TYPE_SUBMENU, MENUNAME, HELPSTR, 0}
    
#else

///< 定义命令菜单项
#define UECLI_DECLARE_COMMAND(PDATA, EXENAME, HELPSTR, ...) \
    {(void*)PDATA, UECLI_TYPE_FUN, EXENAME, HELPSTR}

///< 定义子菜单项
#define UECLI_DECLARE_SUBMENU(PDATA, MENUNAME, HELPSTR) \
    {(void*)PDATA, UECLI_TYPE_SUBMENU, MENUNAME, HELPSTR}
#endif

///< 命令列表定义结尾
#define UECLI_DECLARE_END() {0}
/// \}

// ********************************************************************************************
// 接口函数

//兼容C C++混合编程
#ifdef __cplusplus
extern "C" {
#endif

void uecli_Initialize(const uecli_MenuItem* cmdlist);
void uecli_ExeCmdine(const char* cmdstring);
void uecli_ProcessRecChar(const char recchar[], int len);
void uecli_PrintString(const char* str);

#if UECLI_01_IN_HOOK>0
void* uecli_SetHook(void(*pfun)(int,char**));
bool uecli_IsValidHook(void);
#endif

//兼容C C++混合编程
#ifdef __cplusplus
}
#endif

#endif
