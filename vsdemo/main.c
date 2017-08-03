
#include <stdio.h>
#include "uecli.h"

int uecli_port_in(void* buff, uint16_t maxnum);

void estrtoitest(int argc, char** argv)
{
    int32_t temp;
    int res;

    if (argc > 1)
    {
        res = estrtoi(argv[1], &temp);
        char tcharbuff[128];
        esnprintf(tcharbuff, 128, "执行结果：%d %o %x, 返回值：%d\r\n", temp, temp, temp, res);
        uecli_PrintString(tcharbuff);
    }

    argc = argc;
    argv = argv;
}
void testprintf(int argc, char** argv)
{
    char tstr[128];
    tstr[0] = '\0';

    if (argc > 2)
    {
        int valu = 0;
        if (estrtoi(argv[2], &valu))
            esnprintf(tstr,128,argv[1], valu);
        else
            esnprintf(tstr, 128, "%s", "请输入正确的参数\r\n");
    }
    else if (argc > 1)
        esnprintf(tstr, 128, argv[1]);
    else
        esnprintf(tstr, 128, "%s", "请输入正确的参数\r\n");

    uecli_PrintString(tstr);
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
    UECLI_DECLARE_COMMAND(estrtoitest,"estrtoi", "测试estrtoi"),
    UECLI_DECLARE_COMMAND(testprintf,"eprintf", "测试eprintf"),
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


