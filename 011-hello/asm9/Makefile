TOOLPATH = ../../z_tools/
INCPATH = ../../z_tools/haribote/


MAKE = $(TOOLPATH)make.exe -r
NASK = $(TOOLPATH)nask.exe 
EDIMG = $(TOOLPATH)edimg.exe
IMGTOL = $(TOOLPATH)imgtol.com
QEMU = $(TOOLPATH)qemu
COPY     = copy
DEL      = del

CC1 = $(TOOLPATH)cc1.exe -I$(INCPATH) -Os -Wall -quiet
GAS2NASK = $(TOOLPATH)gas2nask.exe -a
OBJ2BIM = $(TOOLPATH)obj2bim.exe
BIM2HRB = $(TOOLPATH)bim2hrb.exe
RULEFILE = $(TOOLPATH)haribote/haribote.rul

MAKEFONT = $(TOOLPATH)makefont.exe
BIN2OBJ = $(TOOLPATH)bin2obj.exe


OBJS_BOOTPACK = bootpack.obj naskfunc.obj hankaku.obj graphic.obj dsctbl.obj \
		int.obj fifo.obj keyboard.obj mouse.obj memory.obj sheet.obj timer.obj \
		mtask.obj console.obj file.obj filesystem.obj disk.obj


default :
	$(MAKE) img

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

######nas
hello.bim : hello.obj Makefile
	$(OBJ2BIM) @$(RULEFILE) out:hello.bim stack:1k map:hello.map hello.obj

hello.hrb : hello.bim Makefile
	$(BIM2HRB) hello.bim hello.hrb 0


hello2.bim : hello2.obj Makefile
	$(OBJ2BIM) @$(RULEFILE) out:hello2.bim stack:1k map:hello2.map hello2.obj

hello2.hrb : hello2.bim Makefile
	$(BIM2HRB) hello2.bim hello2.hrb 0

a.bim : a.obj a_nask.obj Makefile
	$(OBJ2BIM) @$(RULEFILE) out:a.bim map:a.map a.obj a_nask.obj

a.hrb : a.bim Makefile
	$(BIM2HRB) a.bim a.hrb 0
	
testmem.bim : testmem.obj a_nask.obj Makefile
	$(OBJ2BIM) @$(RULEFILE) out:testmem.bim stack:1k map:testmem.map testmem.obj a_nask.obj

testmem.hrb : testmem.bim Makefile
	$(BIM2HRB) testmem.bim testmem.hrb 0
	
testfile.bim : testfile.obj a_nask.obj Makefile
	$(OBJ2BIM) @$(RULEFILE) out:testfile.bim stack:1k map:testfile.map testfile.obj a_nask.obj

testfile.hrb : testfile.bim Makefile
	$(BIM2HRB) testfile.bim testfile.hrb 0

crack1.bim : crack1.obj a_nask.obj Makefile
	$(OBJ2BIM) @$(RULEFILE) out:crack1.bim map:crack1.map crack1.obj a_nask.obj

crack1.hrb : crack1.bim Makefile
	$(BIM2HRB) crack1.bim crack1.hrb 0
	
crack2.hrb : crack2.nas Makefile
	$(NASK) crack2.nas crack2.hrb crack2.lst

bootpack.hrb : bootpack.bim Makefile
	$(BIM2HRB) bootpack.bim bootpack.hrb 0

haribote.sys : asmhead.bin bootpack.hrb Makefile
	copy /B asmhead.bin+bootpack.hrb haribote.sys

haribote.img : ipl10.bin haribote.sys hello.hrb hello2.hrb a.hrb crack1.hrb crack2.hrb testmem.hrb testfile.hrb Makefile
	$(EDIMG)   imgin:../../z_tools/fdimg0at.tek \
		wbinimg src:ipl10.bin len:512 from:0 to:0 \
		copy from:haribote.sys to:@: \
		copy from:console.c to:@: \
		copy from:make.bat to:@: \
		copy from:hello.hrb to:@: \
		copy from:hello2.hrb to:@: \
		copy from:a.hrb to:@: \
		copy from:crack1.hrb to:@: \
		copy from:crack2.hrb to:@: \
		copy from:testmem.hrb to:@: \
		copy from:testfile.hrb to:@: \
		imgout:haribote.img


### ####
%.gas : %.c Makefile
	$(CC1) -o $*.gas $*.c
	
%.nas : %.gas Makefile 
	$(GAS2NASK) $*.gas $*.nas

%.obj : %.nas Makefile 
	$(NASK) $*.nas $*.obj $*.lst



img :
	$(MAKE) haribote.img

run :
	$(MAKE) img
	$(COPY) haribote.img ..\..\z_tools\qemu\fdimage0.bin
	$(MAKE) -C $(QEMU)

install :
	$(MAKE) img
	$(IMGTOL) w a: haribote.img

clean :
	-$(DEL) *.bin
	-$(DEL) *.lst
	-$(DEL) *.gas
	-$(DEL) *.obj
	-$(DEL) *.hrb
	-$(DEL) *.map
	-$(DEL) *.bim
	-$(DEL) bootpack.map
	-$(DEL) bootpack.bim
	-$(DEL) bootpack.hrb
	-$(DEL) haribote.sys
	-$(DEL) haribote.img

src_only :
	$(MAKE) clean
	-$(DEL) haribote.img
