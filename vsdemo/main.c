
#include <stdio.h>
#include "uecli.h"

int uecli_port_in(void* buff, uint16_t maxnum);

void testfun(int argc, char** argv)
{
    uecli_PrintString("hellptest\n");

    argc = argc;
    argv = argv;
}
void testprtd(int argc, char** argv)
{
    if (argc > 2)
    {
        int valu = 0;
        if (estrtoi(argv[2], &valu))
            eprintf(argv[1], valu);
        else
            eprintf("请输入正确的参数\r\n");
    }
    argc = argc;
    argv = argv;
}

void menufun(int argc, char** argv)
{
    uecli_PrintString("testmenu\n");
}

const uecli_Handle submenu[] =
{
    UECLI_DECLARE_COMMAND(menufun,"testmenu", "子菜单函数",""),
    UECLI_ITEM_END()
};


const uecli_Handle handtalbe[]=
{
    UECLI_DECLARE_COMMAND(testfun,"test", "一个测试函数"),
    UECLI_DECLARE_COMMAND(testfun,"testprint", "一个测试函数"),
    UECLI_DECLARE_SUBMENU(submenu,"menu1", "子菜单测试"),
    UECLI_ITEM_END()
};

int main(void)
{
    char buff[1024];
    int len;

    uecli_Initialize(handtalbe);

    for (;;)
    {
        len = uecli_port_in(buff, 1024);	/* 读取数据 */
        uecli_ProcessRecChar(buff, len);
    }
}


