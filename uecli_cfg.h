/**
 * @file     uecli_cfg.h
 * @brief    uecli配置头文件
 * @author   张诗星
 * @par
 * (C) Copyright 杭州大仁科技有限公司
 * @version
 * 2017/07/31 张诗星 修订说明\n
 *
 */ 

#ifndef _U_E_CLI_CFG_H_
#define _U_E_CLI_CFG_H_

// ********************************************************************************************
// 配置

/* 输入命令行字符串最大长度 */
#define UECLI_CFG_MAX_CMDLINE    (32)

// 内部临时字符串缓存大小
#define TEMP_STRING_LEN     (64)

/* 输入命令行中最大分割参数数量 */
#define UECLI_CFG_MAX_ARGNUM     (6)

/* 提示符字符串最大长度 */
#define UECLI_CFG_MAX_POMPTLINE  (32)

/* 输入命令行历史最大数量 */
#define UECLI_CFG_HISTORY_COUNT (16)

/* 子菜单堆栈最大数量 */
#define UECLI_CFG_STACK_COUNT   (4)

/* 自动补全最大显示数量 */
#define UECLI_CFG_AUTOCOMP_NUM  (16)

// 功能支持开关
//#define SHELL_CFG_COLOR         (1)     /* 支持颜色扩展 */
#define UECLI_CFG_LONG_HELP     (1)     /* 长帮助信息支持 */
#define UECLI_CFG_SUBMENU       (1)     /* 命令堆栈支持 */
#define UECLI_CFG_HISTORY       (1)     /* 命令历史支持 */
#define UECLI_CFG_COMPLETE      (1)     /* 自动补全支持 */

/* 换行符 */
#define UECLI_NEWLINE   "\r\n"

/* log支持 */
#define UECLI_LOG(FORMAT, ...) /*eprintf(FORMAT UECLI_NEWLINE, ##__VA_ARGS__)*/

//字符串操作函数定义
#include "estring.h"
#define uecli_snprintf(sd,num,ss,...) esnprintf(sd,num,ss,##__VA_ARGS__)
#define uecli_isprintfchar(ch) ((ch)>=0x20&&(ch)<=0x7f)
#define uecli_strncpy(sd,ss,num) estrncpy(sd,ss,num)
#define uecli_strcasecmp(sd,ss) estrcmpNocase(sd,ss)
#define uecli_strncat(sd, ss, len)  estrncat(sd,ss,len)
#define uecli_strlen(str)   estrlen(str)

#endif
