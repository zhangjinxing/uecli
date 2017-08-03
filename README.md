
## 特性

- [X] 命令补全：输入部分命令字符并按下`TAB`按键，将显示所有以输入开头的匹配命令列表。如果只有一项匹配，则自动完成输入命令。
- [X] 命令历史记录：任何时候按键盘`UP`和`DOWN`按键可浏览输入的命令行历史记录。
- [X] 子菜单支持：定义命令表时可以定义菜单类型的条目，该条目的指针指向另外一个子菜单命令列表。输入菜单项名称即可进入该子菜单中，同时提示符也显示菜单的分层结构。
- [X] 长帮助信息支持：使用`help + "指定命令"`可以浏览显示该命令的更详细的长帮助信息。
- [X] 可配置和和裁剪：可在配置文件中指定是否打开以上功能支持
- [ ] 增加命令行捕捉功能。捕捉后所有的输入的数据直接传入到捕捉的命令函数中，直到退出捕捉模式为止。

### 演示

- 命令补全：
![命令补全](http://i.imgur.com/lV17nyf.gif "命令补全")
- 历史记录：
![历史记录](http://i.imgur.com/0wd5NLP.gif "历史记录")
- 子菜单：
![子菜单](http://i.imgur.com/mL8XtzY.gif "子菜单")

## 移植

### 移植接口
移植`uecli`需要定义以下几个底层函数：
- uecli_port_init：初始化时会调用该函数用以初始化底层端口
- uecli_port_out:程序发送数据时会调用该函数将字符串数据发送出去
- uecli_port_in:程序通过该函数获取输入数据
> `win_uart_port.c`为win平台中的底层驱动示例

### 裁剪配置
在`uecli_cfg.h`文件中可以对uecli进行配置和裁剪：
```C
#define SHELL_CFG_COLOR         (1)     /* 支持颜色扩展 */
#define UECLI_CFG_LONG_HELP     (1)     /* 长帮助信息支持 */
#define UECLI_CFG_SUBMENU       (1)     /* 命令堆栈支持 */
#define UECLI_CFG_HISTORY       (1)     /* 命令历史支持 */
#define UECLI_CFG_COMPLETE      (1)     /* 自动补全支持 */
```

### 其他配置
在`uecli_cfg.h`文件中可以对提示符最大字符数、命令行最大字符数、最大子菜单层数等进行配置：
```C
/* 输入命令行字符串最大长度 */
#define UECLI_CFG_MAX_CMDLINE    (32)

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
```

### 不同平台的换行符
win平台和linux平台中换行符的定义是不同的：
- win平台中换行符为`\n`
- linux平台中换行符为`\r\n`
可在`uecli_cfg.h`文件中对换行符进行定义

## 使用

### 定义命令表
使用时首先需要定义一个命令表：
```C
const uecli_Handle handtalbe[]=
{
    UECLI_DECLARE_COMMAND(testfun,"test", "一个测试函数"),
    UECLI_DECLARE_COMMAND(testfun,"testprint", "一个测试函数"),
    UECLI_DECLARE_COMMAND(testfun,"printtest1", "测试函数1"),
    UECLI_DECLARE_COMMAND(testfun,"printtest", "测试函数"),
    UECLI_DECLARE_SUBMENU(handtalbe,"submenu", "子菜单测试"),
    UECLI_ITEM_END()
};
```
- UECLI_DECLARE_COMMAND：该项目为一个功能命令项，输入该命令将会调用相应的函数并执行。

> 示例：`UECLI_DECLARE_COMMAND(命令函数指针, 执行该命令输入的字符串, 描述信息, 可选的长帮助信息)`

- UECLI_DECLARE_SUBMENU：该项目为子菜单项，输入该命令将进入相应的子菜单。

> 示例：`UECLI_DECLARE_SUBMENU(子菜单命令指针, 进入子菜单输入的字符串, 描述信息)`

- UECLI_ITEM_END：命令表定义结束

### 初始化
之后应调用`uecli_Initialize`来初始化命令表和其他内部数据，同时内部将会调用`uecli_port_init`函数来初始化底层接口。

### 处理接收的字符
对于接收到的每个字符，都调用`uecli_ProcessRecChar`函数来处理接收到的字符数据，当需要显示信息时，会调用`uecli_port_out`函数将打印数据传输出去。

> ```C
uecli_Initialize(handtalbe);
for (;;)
{
    len = uecli_port_in(buff, 1024);	/* 读取数据 */
    uecli_ProcessRecChar(buff, len);
}
```