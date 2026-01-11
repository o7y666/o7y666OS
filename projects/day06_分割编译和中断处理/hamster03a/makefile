TOOLPATH = ../tools/
INCPATH  = ../tools/haribote/

MAKE     = $(TOOLPATH)make.exe -r
NASK     = $(TOOLPATH)nask.exe
EDIMG    = $(TOOLPATH)edimg.exe
IMGTOL   = $(TOOLPATH)imgtol.com
GAS2NASK = $(TOOLPATH)gas2nask.exe -a
BIN2OBJ  = $(TOOLPATH)bin2obj.exe
OBJ2BIM  = $(TOOLPATH)obj2bim.exe
MAKEFONT = $(TOOLPATH)makefont.exe
BIM2HRB  = $(TOOLPATH)bim2hrb.exe
RULEFILE = $(TOOLPATH)haribote/haribote.rul
CC1      = $(TOOLPATH)cc1.exe -I$(INCPATH) -Os -Wall -quiet
COPY     = copy
DEL      = del

default :
	$(MAKE) img
#文件生成规则
#启动区内由汇编转换成二进制
o7yipl.bin : o7yipl.nas makefile
	$(NASK) o7yipl.nas o7yipl.bin o7yipl.lst

asmhead.bin : asmhead.nas makefile
	$(NASK) asmhead.nas asmhead.bin asmhead.lst

bootpack.gas : bootpack.c makefile
	$(CC1) -o bootpack.gas bootpack.c

bootpack.nas : bootpack.gas makefile
	$(GAS2NASK) bootpack.gas bootpack.nas

bootpack.obj : bootpack.nas makefile
	$(NASK) bootpack.nas bootpack.obj bootpack.lst

graphic.gas : graphic.c makefile
	$(CC1) -o graphic.gas graphic.c

graphic.nas : graphic.gas makefile
	$(GAS2NASK) graphic.gas graphic.nas

graphic.obj : graphic.nas makefile
	$(NASK) graphic.nas graphic.obj graphic.lst

dsctbl.gas : dsctbl.c makefile
	$(CC1) -o dsctbl.gas dsctbl.c

dsctbl.nas : dsctbl.gas makefile
	$(GAS2NASK) dsctbl.gas dsctbl.nas

dsctbl.obj : dsctbl.nas makefile
	$(NASK) dsctbl.nas dsctbl.obj dsctbl.lst

naskfunc.obj : naskfunc.nas makefile
	$(NASK) naskfunc.nas naskfunc.obj naskfunc.lst

hankaku.bin : hankaku.txt makefile
	$(MAKEFONT) hankaku.txt hankaku.bin

hankaku.obj : hankaku.bin makefile
	$(BIN2OBJ) hankaku.bin hankaku.obj _hankaku

bootpack.bim : bootpack.obj naskfunc.obj hankaku.obj graphic.obj dsctbl.obj makefile
	$(OBJ2BIM) @$(RULEFILE) out:bootpack.bim stack:3136k map:bootpack.map \
		bootpack.obj naskfunc.obj hankaku.obj graphic.obj dsctbl.obj

bootpack.hrb : bootpack.bim makefile
	$(BIM2HRB) bootpack.bim bootpack.hrb 0

hamster.sys : asmhead.bin bootpack.hrb makefile
	copy /B asmhead.bin+bootpack.hrb hamster.sys

hamster.img : o7yipl.bin hamster.sys makefile
	$(EDIMG)   imgin:../tools/fdimg0at.tek \
		wbinimg src:o7yipl.bin len:512 from:0 to:0 \
		copy from:hamster.sys to:@: \
		imgout:hamster.img


#命令
img:
	$(MAKE) hamster.img
run:
	$(MAKE) img
	$(COPY) hamster.img ..\tools\qemu\fdimage0.bin
	$(MAKE) -C ../tools/qemu
install:
	$(MAKE) img
	$(IMGTOL) w a: hamster.img
clean :
	-$(DEL) *.bin
	-$(DEL) *.lst
	-$(DEL) *.gas
	-$(DEL) *.obj
	-$(DEL) bootpack.nas
	-$(DEL) graphic.nas
	-$(DEL) dsctbl.nas
	-$(DEL) bootpack.map
	-$(DEL) bootpack.bim
	-$(DEL) bootpack.hrb
	-$(DEL) hamster.sys
src_only :
	../tools/make.exe clean
	-del hamster.img


