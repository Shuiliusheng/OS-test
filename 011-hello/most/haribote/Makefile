TOOLPATH = ../../../z_tools/
INCPATH = ../../../z_tools/haribote/


MAKE = $(TOOLPATH)make.exe -r
NASK = $(TOOLPATH)nask.exe 
EDIMG = $(TOOLPATH)edimg.exe
IMGTOL = $(TOOLPATH)imgtol.com
QEMU = $(TOOLPATH)qemu


CC1 = $(TOOLPATH)cc1.exe -I$(INCPATH) -Os -Wall -quiet
GAS2NASK = $(TOOLPATH)gas2nask.exe -a
OBJ2BIM = $(TOOLPATH)obj2bim.exe
BIM2HRB = $(TOOLPATH)bim2hrb.exe
BIN2OBJ = $(TOOLPATH)bin2obj.exe

RULEFILE = ../haribote.rul
MAKEFONT = $(TOOLPATH)makefont.exe

COPY     = copy
DEL      = del

OBJS_BOOTPACK = bootpack.obj naskfunc.obj hankaku.obj graphic.obj dsctbl.obj \
		int.obj fifo.obj keyboard.obj mouse.obj memory.obj sheet.obj timer.obj \
		mtask.obj console.obj file.obj filesystem.obj disk.obj


default :
	$(MAKE) ipl10.bin
	$(MAKE) haribote.sys

ipl10.bin : ipl10.nas Makefile
	$(NASK) ipl10.nas ipl10.bin ipl10.lst

asmhead.bin : asmhead.nas Makefile
	$(NASK) asmhead.nas asmhead.bin asmhead.lst
	
hankaku.bin : hankaku.txt Makefile
	$(MAKEFONT) hankaku.txt hankaku.bin

hankaku.obj : hankaku.bin Makefile
	$(BIN2OBJ) hankaku.bin hankaku.obj _hankaku

bootpack.bim : $(OBJS_BOOTPACK)  Makefile
	$(OBJ2BIM) @$(RULEFILE) out:bootpack.bim stack:3136k map:bootpack.map \
		$(OBJS_BOOTPACK)

bootpack.hrb : bootpack.bim Makefile
	$(BIM2HRB) bootpack.bim bootpack.hrb 0

haribote.sys : asmhead.bin bootpack.hrb Makefile
	copy /B asmhead.bin+bootpack.hrb haribote.sys


### ####
%.gas : %.c Makefile
	$(CC1) -o $*.gas $*.c
	
%.nas : %.gas Makefile 
	$(GAS2NASK) $*.gas $*.nas

%.obj : %.nas Makefile 
	$(NASK) $*.nas $*.obj $*.lst

clean :
	-$(DEL) *.lst
	-$(DEL) *.gas
	-$(DEL) *.obj
	-$(DEL) *.hrb
	-$(DEL) *.map
	-$(DEL) *.bim
	-$(DEL) hankaku.bin
	-$(DEL) asmhead.bin
	-$(DEL) bootpack.map
	-$(DEL) bootpack.bim
	-$(DEL) bootpack.hrb

src_only :
	$(MAKE) clean
	-$(DEL) ipl10.bin
	-$(DEL) haribote.sys
