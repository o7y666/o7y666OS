OBJS_BOOTPACK = bootpack.obj naskfunc.obj hankaku.obj graphic.obj dsctbl.obj int.obj fifo.obj keyboard.obj mouse.obj\
memory.obj sheet.obj

TOOLPATH = ../tools/
INCPATH  = ../tools/haribote/

MAKE     = $(TOOLPATH)make.exe -r
NASK     = $(TOOLPATH)nask.exe
EDIMG    = $(TOOLPATH)edimg.exe
IMGTOL   = $(TOOLPATH)imgtol.com
GAS2NASK = $(TOOLPATH)gas2nask.exe -a
OBJ2BIM  = $(TOOLPATH)obj2bim.exe
BIM2HRB  = $(TOOLPATH)bim2hrb.exe
BIN2OBJ  = $(TOOLPATH)bin2obj.exe
MAKEFONT  = $(TOOLPATH)makefont.exe
RULEFILE = $(TOOLPATH)haribote/haribote.rul
CC1      = $(TOOLPATH)cc1.exe -I$(INCPATH) -Os -Wall -quiet
COPY     = copy
DEL      = del
default :
	$(MAKE) img

#普通规则

o7yipl.bin : o7yipl.nas makefile
	$(NASK) o7yipl.nas o7yipl.bin o7yipl.lst

asmhead.bin : asmhead.nas makefile
	$(NASK) asmhead.nas asmhead.bin asmhead.lst

hankaku.bin: hankaku.txt makefile
	$(MAKEFONT) hankaku.txt hankaku.bin

naskfunc.obj : naskfunc.nas makefile
	$(NASK) naskfunc.nas naskfunc.obj naskfunc.lst

hankaku.obj : hankaku.bin makefile
	$(BIN2OBJ) hankaku.bin hankaku.obj _hankaku

bootpack.bim : $(OBJS_BOOTPACK) makefile
	$(OBJ2BIM) @$(RULEFILE) out:bootpack.bim stack:3136k map:bootpack.map \
	$(OBJS_BOOTPACK) 


bootpack.hrb : bootpack.bim makefile
	$(BIM2HRB) bootpack.bim bootpack.hrb 0


cat.sys : asmhead.bin bootpack.hrb makefile
	copy /B asmhead.bin+bootpack.hrb cat.sys


cat.img: o7yipl.bin cat.sys makefile
	$(EDIMG) imgin:../tools/fdimg0at.tek \
	wbinimg src:o7yipl.bin len:512 from:0 to:0  \
	copy from:cat.sys to:@: \
	imgout:cat.img

#一般规则

%.gas : %.c makefile
	$(CC1) -o $*.gas $*.c

%.nas : %.gas makefile
	$(GAS2NASK) $*.gas $*.nas

%.obj : %.nas makefile
	$(NASK) $*.nas $*.obj $*.lst

img:
	 $(MAKE) cat.img
run:
	$(MAKE) img
	$(COPY) cat.img  ..\tools\qemu\fdimage0.bin
	$(MAKE) -C  ../tools/qemu
install:
	$(MAKE) img
	$(IMGTOL) w a : cat.img
clean :
	-$(DEL) *.bin
	-$(DEL) *.lst
	-$(DEL) *.obj
	-$(DEL) bootpack.map
	-$(DEL) bootpack.bim
	-$(DEL) bootpack.hrb
	-$(DEL) cat.sys
src_only :
	$(MAKE) clean
	-$(DEL) cat.img
