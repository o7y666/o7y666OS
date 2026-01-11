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