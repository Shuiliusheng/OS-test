16-console-win

tranform char
shift+key
A->a
enter 遇到边界页面滚动
mem cls命令 dir命令 type1, 支持FAT阅读

add hlt nas 拆分函数

api 简单的尝试，asmfuction call far retf
farcall retf 程序调用后返回

利用中断进行api调用，不用动态更改地址
查找文件名称进行执行

中断进入，edx传参，再决定api函数

console_task->cmd_app->hrb->int40->naskfunc->hrb_api

cs_base 0x0fe8

c语言使用api，更改c语言编译的二进制文件的开头，call retf
console_task->cmd_app->c.hrb->api_nas->int40->naskfunc->hrb_api
