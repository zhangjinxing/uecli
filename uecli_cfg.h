/// \file uecli_cfg.h
/// \brief uecli配置头文件
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

#ifndef _U_E_CLI_CFG_H_
#define _U_E_CLI_CFG_H_

// ********************************************************************************************
// 配置

/// \defgroup  UECLI_CFG相关配置定义
/// \{
#define UECLI_CFG_STR_MAXLEN     (32)    ///< 输入命令行字符串最大长度
#define UECLI_CFG_TEMP_MAXLEN    (64)    ///< 内部处理临时字符串最大长度
#define UECLI_CFG_MAX_ARGNUM     (6)     ///< 输入命令行中最大分割参数数量
#define UECLI_CFG_MAX_POMPTLINE  (32)    ///< 提示符字符串最大长度
#define UECLI_CFG_HISTORY_COUNT  (8)     ///< 输入命令行历史最大数量
#define UECLI_CFG_STACK_COUNT    (8)     ///< 子菜单堆栈最大数量
#define UECLI_CFG_AUTOCOMP_NUM   (16)    ///< 自动补全最大显示数量
/// \}

/// \defgroup  UECLI_01功能支持开关
/// \{
#define UECLI_01_COLOR          (0)     ///< 支持颜色扩展
#define UECLI_01_IN_HOOK        (1)     ///< 输入钩子支持
#define UECLI_01_LONG_HELP      (1)     ///< 长帮助信息支持
#define UECLI_01_SUBMENU        (1)     ///< 子菜单支持
#define UECLI_01_HISTORY        (1)     ///< 命令历史支持
#define UECLI_01_COMPLETE       (1)     ///< 自动补全支持
/// \}

///< 换行符
#define UECLI_NEWLINE   "\r\n"

///< log支持
#define UECLI_LOG(FORMAT, ...) /*eprintf(FORMAT UECLI_NEWLINE, ##__VA_ARGS__)*/

//字符串操作函数定义
#include "estring.h"
#define uecli_snprintf(sd,num,ss,...) esnprintf(sd,num,ss,##__VA_ARGS__)
#define uecli_isprintfchar(ch) ((ch)>=0x20&&(ch)<=0x7f)

///< 复制字符串
#define uecli_strncpy(sd,ss,num) estrncpy(sd,ss,num)

///< 命令字符比较
#define uecli_strcasecmp(sd,ss) estrcmpNocase(sd,ss)

///< 追加字符
#define uecli_strncat(sd, ss, len)  estrncat(sd,ss,len)

///< 获取字符串长度
#define uecli_strlen(str)   estrlen(str)

#include <assert.h>
#define uecli_assert(x) assert(x)

#endif
