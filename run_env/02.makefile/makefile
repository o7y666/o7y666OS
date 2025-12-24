default :
	../tools/make.exe img
#文件生成规则
#启动区内由汇编转换成二进制
o7yipl.bin : o7yipl.nas makefile
	../tools/nask.exe o7yipl.nas o7yipl.bin o7yipl.lst

#由启动区二进制文件转换成img文件
o7yos.img: o7yipl.bin makefile
	../tools/edimg.exe imgin:../tools/fdimg0at.tek   wbinimg src:o7yipl.bin len:512 from:0 to:0   imgout:o7yos.img

#命令
img:
	 ../tools/make.exe -r o7yos.img
asm:
	../tools/nask.exe -r o7yipl.bin
run:
	../tools/make.exe img
	copy o7yos.img  ..\tools\qemu\fdimage0.bin
	../tools/make.exe -C  ../tools/qemu
install:
	../tools/make.exe img
	..\tools\imgtol.com w a : o7yos.img
clean :
	-del o7yipl.bin
	-del o7yipl.lst

src_only :
	../tools/make.exe clean
	-del o7yos.img
