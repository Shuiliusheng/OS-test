; hello-os
; TAB=4
 
		ORG		0x7c00			 ;  ORG指令指明了程序的装填地址，即在内存中所在的地址，
								;其中地址0X7c00为当时IBM和英特尔设计人员规定的
 
; 以下的记述用于标准FAT12格式的软盘
		JMP		entry			; 0xeb, 0x4e
		DB		0x90
		DB		"HELLOIPL"		 ; 可以自由写扇区的名字（8字节）
		DW		512				 ; 1扇区的大小（必需要512）
		DB		1				 ; 集群的大小（必需要做一个扇区）
		DW		1			  	 ; FATA从这里开始（一般是从第一扇区开始）
		DB		2				 ; FATA的个数（必须是2）
		DW		224				 ; 根目录区域的大小（通常为224项）
		DW		2880			 ; 该驱动器的大小（必须要为2880扇区）
		DB		0xf0			 ; 媒体类型（必须要做0 xf0）
		DW		9				 ; FATO区域的长度（必须要做9扇区）
		DW		18				 ; 1卡车上有几个扇区
		DW		2				 ; 头数（必须要2）
		DD		0				 ; 因为没有使用分区，所以这里一定是0
		DD		2880			 ; 再写一次一个寄存器大小
		DB		0,0,0x29		 ; 虽然不太明白，但是放在?个价?上就好了
		DD		0xffffffff		 ; 大概卷序列号
		DB		"HELLO-OS   "	 ; 磁盘名称（11字节）
		DB		"FAT12   "		 ; 格式名称（8字节）
		RESB	18				 ; 先放18个字节
 
; 程序核心
 
entry:
		MOV		AX,0			 ; 初始化寄存器
		MOV		SS,AX			 ;;SS-stack segment,堆栈段
		MOV		SP,0x7c00		 ;;SP-stack pointer,栈指针寄存器
		MOV		DS,AX			 ;;DS-data segment,数据段
		MOV		ES,AX			 ;;ES-extra segment,附加段
 
		MOV		SI,msg			 ;;SI-source index,源变址寄存器
putloop:
		MOV		AL,[SI]
		ADD		SI,1			 ;让SI加1
		CMP		AL,0
		JE		fin
		MOV		AH,0x0e			 ; 指明一个文字
		MOV		BX,15			 ; 指定字符颜色
		INT		0x10			 ; 调用显卡BIOS,16号中断
		JMP		putloop
fin:
		HLT						 ; 让CPU停止，等待指令
		JMP		fin				 ; 无限循环
 
msg:
		DB		0x0a, 0x0a		 ; 换行两次
		DB		"hello, world"
		DB		0x0a			 ; 换行
		DB		0
 
		RESB	0x7dfe-$		 ; 到0x7dfe在0x00中填入的命令
 
		DB		0x55, 0xaa