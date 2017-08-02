
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <windows.h>
#include <tchar.h>

#include "uecli_cfg.h"

// ********************************************************************************************
// 内部变量

// 串口句柄对象
HANDLE _hCom = NULL;
// 初始化和打开
static bool _sopenBool = false;
// 读异步对象
static OVERLAPPED _ro;
// 写异步对象
static OVERLAPPED _wo;

// ********************************************************************************************
// 接口函数

// 初始化


/**
 * \brief      底层初始化函数
 * \details    上层应用调用改函数来完成底层接口的初始化
 * \return     操作完成返回true，否则返回false
 */
bool uecli_port_init(void)
{
    //执行结果
    bool result;
    //COM端口字符串
    const char* comName = "COM4";
    DWORD dwErrorFlags;

    ////////////////////////////////////////////////////////
    //打开串口
    _hCom = CreateFileA(comName,        /* 串口 */
        GENERIC_READ | GENERIC_WRITE,   /* 允许读和写  */
        0,                              /* 独占方式*/
        NULL,                           /* 安全特性 */
        OPEN_EXISTING,                  /* 打开而不是创建  */
        FILE_FLAG_OVERLAPPED,           /* 异步方式  */
        NULL);
    if (INVALID_HANDLE_VALUE == _hCom || NULL == _hCom)
    {
        _hCom = NULL;
        UECLI_LOG("打开串口设备错误%s出错,错误码%d.", comName, GetLastError());
        return false;
    }
    //配置串口
    DCB dcb;
    SecureZeroMemory(&dcb, sizeof(DCB));
    GetCommState(_hCom, &dcb);  // 读取当前配置值
    dcb.DCBlength = sizeof(DCB);
    dcb.BaudRate = CBR_115200;  // 波特率
    dcb.ByteSize = 8;           // 数据长度
    dcb.Parity = NOPARITY;      // 校验位
    dcb.StopBits = ONESTOPBIT;  // 停止位
    if (!SetCommState(_hCom, &dcb))
    {
        CloseHandle(_hCom);
        _hCom = NULL;
        UECLI_LOG("串口配置DCB错误,错误码%d.", GetLastError());
        return false;
    }
    SetupComm(_hCom, 2048, 2048);
    result = ClearCommError(_hCom, &dwErrorFlags, NULL);// 清除通讯错误

    // 设置超时结构
    COMMTIMEOUTS TimeOuts;
    GetCommTimeouts(_hCom, &TimeOuts);
    TimeOuts.ReadIntervalTimeout = 1;           /* 读间隔超时 */
    TimeOuts.ReadTotalTimeoutMultiplier = 0;    /* 读超时系数ms */
    TimeOuts.ReadTotalTimeoutConstant = 0;      /* 读超时常量ms */
    TimeOuts.WriteTotalTimeoutMultiplier = 0;   /* 写超时系数ms */
    TimeOuts.WriteTotalTimeoutConstant = 0;     /* 写超时常量ms */
    if (!SetCommTimeouts(_hCom, &TimeOuts))
    {
        CloseHandle(_hCom);
        _hCom = NULL;
        UECLI_LOG("串口配置超时参数出错,错误码%d.", GetLastError());
        return false;
    }

    // 创建异步操作对象
    _ro.Offset = 0;
    _ro.OffsetHigh = 0;
    _ro.hEvent = CreateEvent(
        NULL,       // 默认安全属性
        FALSE,      // 自动复位对象
        FALSE,      // 初始化状态-无信号
        NULL);      // 无事件名

    _wo.Offset = 0;
    _wo.OffsetHigh = 0;
    _wo.hEvent = CreateEvent(
        NULL,       // 默认安全属性
        FALSE,      // 自动复位对象
        FALSE,      // 初始化状态-无信号
        NULL);      // 无事件名

    //初始化临界区
    UECLI_LOG("打开端口 %s 成功", comName);
    return true;
}

/**
 * \brief      调用该函数以发送数据
 * \param[in] buff   待发送内存区
 * \param[in] num    发送数据数量
 */
void uecli_port_out(const void* buff, uint32_t num)
{
    // 清除通讯错误
    DWORD dwErrorFlags;
    ClearCommError(_hCom, &dwErrorFlags, NULL); /* 清除通讯错误 */
    //ResetEvent(_wo.hEvent);
    WriteFile(_hCom, buff, num, (LPDWORD)&num, &_wo);    /* 发送数据 */
    GetOverlappedResult(_hCom, &_wo, &dwErrorFlags, TRUE);
}

/**
* \brief      调用该函数以发送数据
* \param[in] buff 保存接收数据内存区
* \param[in] maxnum 最大接收数据数量
* \return     返回实际读取数据数量
*/
int uecli_port_in(void* buff, uint16_t maxnum)
{
    DWORD len = 0;
    DWORD dwErrorFlags;

    ClearCommError(_hCom, &dwErrorFlags, NULL); /* 清除通讯错误 */
    //ResetEvent(_ro.hEvent);
    ReadFile(_hCom, (LPVOID)buff, maxnum, &len, &_ro);
    //WaitForSingleObject(_ro.hEvent, INFINITE);
    GetOverlappedResult(_hCom, &_ro, &len, TRUE);
    return len;
}
