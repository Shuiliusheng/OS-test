c语言使用api，更改c语言编译的二进制文件的开头，call retf
console_task->cmd_app->c.hrb->api_nas->int40->naskfunc->hrb_api

为代码分配了数据段
在汇编代码中对os进入应用进行代码段数据段等的切换
在应用产生中断时也需要进行切换，包括鼠标键盘中断
(目的：避免用户直接操作所有数据)

增加了中断0d的处理，该中断用于处理异常，并会给出异常号，qemu中没有反馈，virtualbox可以
此时crack1会报异常，终止

通过在x86的段标记中增加0x60来利用x86本身的段转换，即在进入api中断时，自动转换为内核态
此时crack2会报异常，使得用户无法通过修改cs和ds来修改内核数据

增加了栈异常的捕获和处理

ctrl+c 结束任务
获取c编译之后的hrb文件信息，包括段大小，数据段位置内容

测试了expand_segmentsize的可用性，利用malloc系统调用，程序可以自己扩大段大小，注意数据拷贝和程序运行后的释放

支持malloc api，该函数初始为数据段大小中去除栈空间外的大小。同时该函数支持分配超过freesize的内存块时，
通过扩大数据段大小来增加可供使用的大小。会进行内存cpy，需要注意mem数据结构中的实地址的变换

增加的文件系统(暂时使用一段内存作为硬盘使用，因为没有找到合适的保护模式访问硬盘的方法)
- 支持在空硬盘上构建文件系统，初始化硬盘的数据
- 支持open,close,read,write,seek,tell,eof文件api
- 支持创建目录，切换工作路径cd
- 支持文件删除，目录删除

多个console，多任务切换有一些问题
多任务切换的问题在于没有开中断。。。。两天

实现了鼠标点击窗口，左键按住移动窗口

实现了console的关闭，利用exit指令关闭，可以实现资源的完全回收
鼠标也可以关闭

键盘输入的api：获取单个字符的两种模式，获取字符串

start app命令，用于新开一个窗口运行命令

拆分了api文件，进一步完善了输入输出的api，支持\r

支持定时器api

支持多线程的api，包括create， run，sleep，delete，这一版未提供参数传入
支持多线程的传参，一个地址参数

支持窗口创建api，包括创建，输出字符，绘制方块，refresh等

支持用户捕获鼠标事件的api，app通过设置event函数来响应鼠标的各种事件，event函数由操作系统触发执行
（使用thread实现）

利用int类型优化了sheet的绘制

修改了initwindow api的问题
增加了ncstart命令， no cons start

增加了使用按键的计算器和绘画板

自带的命令
cd, mkdir, rm, type, mem, cls, exit
start: 新的console中启动一个命令
ncstart: 不开窗口启动一个命令
cons w h: 启动一个w*h的console 

支持type显示文件内容的多少
2: 显示下半屏
3：显示下一屏
4: 显示完
else：显示下一行

console command
1. cls: clear screen
2. type filename: show file content
   - enter 2: show half screen new content every times
   - enter 3: show a whole screen new content every times
   - enter 4: show content without stop
   - enter else: show a line new content every times
3. mem: show system memory information
4. exit: close console
5. mkdir dirname: create a directory in current directory
6. rm filename/dirname: remove file or dir
	- if file or dir is using(read/write), remove failed
7. cd :show current directory
   cd ..:back to last directory level
   cd dirname: into dir
8. start cmd/app: open a new console and run cmd or app
9. ncstart cmd/app: open a new console buf not show and run cmd/app
10. cons width height: open a new console with width*height


file information
1. hrb: application, input the filename to run directly
2. c: source code, can use type command to show the detail

testmem	: test the memory api correctness
testfile: test the file operator api correctness
testget	: test the input api correctness
testput	: test the output api correctness
te_timer: test the timer api correctness
test	: test the multithread api correctness
testdraw: test the draw api and eventlisten api correctness

readfile: function for reading a file
wfile	: function for writing a file
cal		: a calculator with interface(can push button)
painter	: a painter application, can choose color and draw with different line
