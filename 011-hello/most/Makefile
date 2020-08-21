TOOLPATH = ../../z_tools/
INCPATH = ../../z_tools/haribote/


MAKE = $(TOOLPATH)make.exe -r
EDIMG = $(TOOLPATH)edimg.exe
IMGTOL = $(TOOLPATH)imgtol.com
QEMU = $(TOOLPATH)qemu
COPY     = copy
DEL      = del


default :
	$(MAKE) haribote.img

haribote.img :  haribote/ipl10.bin  haribote/haribote.sys crack1/crack1.hrb crack2/crack2.hrb Makefile \
				testmem/testmem.hrb testfile/testfile.hrb testget/testget.hrb testput/testput.hrb \
				readfile/readfile.hrb wfile/wfile.hrb test/test.hrb te_timer/te_timer.hrb
	$(EDIMG)   imgin:../../z_tools/fdimg0at.tek \
		wbinimg src:haribote/ipl10.bin len:512 from:0 to:0 \
		copy from:haribote/haribote.sys to:@: \
		copy from:crack1/crack1.hrb to:@: \
		copy from:crack2/crack2.hrb to:@: \
		copy from:testmem/testmem.hrb to:@: \
		copy from:testfile/testfile.hrb to:@: \
		copy from:testget/testget.hrb to:@: \
		copy from:testput/testput.hrb to:@: \
		copy from:readfile/readfile.hrb to:@: \
		copy from:wfile/wfile.hrb to:@: \
		copy from:te_timer/te_timer.hrb to:@: \
		copy from:test/test.hrb to:@: \
		imgout:haribote.img

run :
	$(MAKE) haribote.img
	$(COPY) haribote.img ..\..\z_tools\qemu\fdimage0.bin
	$(MAKE) -C $(QEMU)

install :
	$(MAKE) haribote.img
	$(IMGTOL) w a: haribote.img

full :
	$(MAKE) -C haribote
	$(MAKE) -C apilib
	$(MAKE) -C crack1
	$(MAKE) -C crack2
	$(MAKE) -C testmem
	$(MAKE) -C testfile
	$(MAKE) -C testget
	$(MAKE) -C testput
	$(MAKE) -C wfile
	$(MAKE) -C te_timer
	$(MAKE) -C readfile
	$(MAKE) -C test
	$(MAKE) haribote.img

run_full :
	$(MAKE) full
	$(COPY) haribote.img ..\..\z_tools\qemu\fdimage0.bin
	$(MAKE) -C $(QEMU)

install_full :
	$(MAKE) full
	$(IMGTOL) w a: haribote.img

runOS :
	$(MAKE) -C haribote
	$(MAKE) run

clean :

src_only :
	$(MAKE) clean
	-$(DEL) haribote.img

clean_full :
	$(MAKE) -C haribote		clean
	$(MAKE) -C apilib		clean
	$(MAKE) -C crack1		clean
	$(MAKE) -C crack2		clean
	$(MAKE) -C testmem		clean
	$(MAKE) -C testfile		clean
	$(MAKE) -C testget	clean
	$(MAKE) -C testput	clean
	$(MAKE) -C readfile	clean
	$(MAKE) -C wfile	clean
	$(MAKE) -C te_timer	clean
	$(MAKE) -C test	clean
	
