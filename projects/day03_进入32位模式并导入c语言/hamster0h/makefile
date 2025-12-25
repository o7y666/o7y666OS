TOOLPATH = ../tools/
INCPATH  = ../tools/haribote/

MAKE     = $(TOOLPATH)make.exe -r
NASK     = $(TOOLPATH)nask.exe
EDIMG    = $(TOOLPATH)edimg.exe
IMGTOL   = $(TOOLPATH)imgtol.com
GAS2NASK = $(TOOLPATH)gas2nask.exe -a
OBJ2BIM  = $(TOOLPATH)obj2bim.exe
BIM2HRB  = $(TOOLPATH)bim2hrb.exe
RULEFILE = $(TOOLPATH)haribote/haribote.rul
CC1      = $(TOOLPATH)cc1.exe -I$(INCPATH) -Os -Wall -quiet
COPY     = copy
DEL      = del

default :
	../tools/make.exe img
#文件生成规则
#启动区内由汇编转换成二进制
o7yipl.bin : o7yipl.nas makefile
	../tools/nask.exe o7yipl.nas o7yipl.bin o7yipl.lst

hamster.sys : hamster.nas makefile
	$(NASK) hamster.nas hamster.sys hamster.lst

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
	-$(DEL) o7yipl.bin
	-$(DEL) o7yipl.lst
	-$(DEL) hamster.sys
	-$(DEL) hamster.lst

src_only :
	../tools/make.exe clean
	-del hamster.img


