#include "bootpack.h"

void init_pic(void)
//pic 的初始化 
{
    io_out8(PIC0_IMR, 0xff); //禁止所有中断 IMR interrupt mask register
    io_out8(PIC1_IMR, 0xff); //禁止所有中断 PIC1=从PIC
    //PIC 0 主pic初始化
    io_out8(PIC0_ICW1, 0x11);/*边沿触发模式 ICW 初始化命令字*/ 
    io_out8(PIC0_ICW2, 0x20);// 决定IRQ以哪一号中断通知CPU
    io_out8(PIC0_ICW3, 1<<2);//主从相关的连接设定，IRQ2与从PIC相连，因此设定为00000100
    io_out8(PIC0_ICW4, 0x01);
    //PIC 1 从pic初始化 
    io_out8(PIC1_ICW1, 0x11);
    io_out8(PIC1_ICW2, 0x28);
    io_out8(PIC1_ICW3, 2);
    io_out8(PIC1_ICW4, 0x01);
    //imr
    io_out8(PIC0_IMR, 0xfb); //PIC1以外全部禁止
    io_out8(PIC1_IMR, 0xff); //禁止所有中断 PIC1=从PIC
}

void inthandler21(int *esp)
//来自PS/2键盘的中断
{
    struct BOOTINFO *binfo = (struct BOOTINFO *)ADR_BOOTINFO;
    boxfill8(binfo->vram, binfo->scrnx, COL8_840084,0,0,32*8-1,15);
    putfonts8_asc(binfo->vram,binfo->scrnx,0,0,COL8_FFFFFF,"INT 21 (IRQ-1) : PS/2 keyboard");
    for(;;){
        io_hlt();
    }
}

void inthandler2c(int *esp)
//鼠标中断
{
    struct BOOTINFO *binfo = (struct BOOTINFO *)ADR_BOOTINFO;
    boxfill8(binfo->vram, binfo->scrnx, COL8_000000,0,0,32*8-1,15);
    putfonts8_asc(binfo->vram,binfo->scrnx,0,0,COL8_FFFFFF,"INT 2c (IRQ-12) : PS/2 mouse");
    for(;;){
        io_hlt();
    }
}
//从PIC中断处理
void inthandler27(int *esp){
    io_out8(PIC0_OCW2, 0x67);// 向IRQ7发送0x60 0x67 = 0x60 + 0x07（IRQ7） 处理假中断
    return;
}