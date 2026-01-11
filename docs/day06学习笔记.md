# 《30天自制操作系统》第六天学习笔记

## 目录

- 1. 今日学习目标

- 2. 主要学习内容

  - 2.1 PIC初始化
  - 2.2 中断处理程序
  - 2.3 字符显示问题调试

- 3. 关键代码分析

- 3.1 PIC初始化代码

  - 3.2 中断处理程序代码
  - 3.3 字符显示函数

- 4. 遇到的问题与解决方案

- 5. 总结与感悟

## 1. 今日学习目标

- 理解8259A PIC的工作原理和初始化方法
- 编写键盘和鼠标的中断处理程序
- 解决字符显示中的问题
- 掌握中断处理的基本流程

## 2. 主要学习内容

### 2.1 PIC初始化

可编程中断控制器（PIC）是管理硬件中断的重要组件。x86架构使用两个8259A PIC芯片（主PIC和从PIC）来处理中断。

**关键概念**：

- IRQ（中断请求线）：硬件设备通过IRQ向CPU发送中断信号
- 中断向量号：CPU用于查找中断处理程序的索引
- IMR（中断屏蔽寄存器）：控制哪些中断被屏蔽
- ICW（初始化命令字）：初始化PIC的命令序列

### 2.2 中断处理程序

中断处理程序是响应硬件中断的代码。需要处理：

1. 保存上下文（寄存器状态）
2. 处理中断（如读取键盘数据）
3. 发送EOI（中断结束）命令
4. 恢复上下文并返回

### 2.3 字符显示问题调试

今天遇到了多个问题，包括IDT初始化错误、字符显示问题等，通过调试发现了以下问题：

1. IDT初始化循环边界错误
2. `putfonts8_asc`函数中颜色参数传递错误
3. `putfont8`函数中最后一个像素判断错误
4. 中断处理程序缺少EOI命令

## 3. 关键代码分析

### 3.1 PIC初始化代码

```
void init_pic(void)
{
    io_out8(PIC0_IMR, 0xff);  // 禁止所有中断
    io_out8(PIC1_IMR, 0xff);
    
    // 主PIC初始化
    io_out8(PIC0_ICW1, 0x11);  // 边沿触发模式
    io_out8(PIC0_ICW2, 0x20);  // IRQ0-7映射到中断0x20-0x27
    io_out8(PIC0_ICW3, 1<<2);  // IRQ2连接从PIC
    io_out8(PIC0_ICW4, 0x01);  // 非缓冲模式
    
    // 从PIC初始化
    io_out8(PIC1_ICW1, 0x11);
    io_out8(PIC1_ICW2, 0x28);  // IRQ8-15映射到中断0x28-0x2f
    io_out8(PIC1_ICW3, 2);     // 连接到主PIC的IRQ2
    io_out8(PIC1_ICW4, 0x01);
    
    // 允许键盘中断（IRQ1）和从PIC中断（IRQ2）
    io_out8(PIC0_IMR, 0xf9);  // 11111001b
    io_out8(PIC1_IMR, 0xef);  // 11101111b（允许鼠标IRQ12）
}
```

### 3.2 中断处理程序代码

```
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
```

### 3.3 字符显示函数

修正后的字符显示函数：

```
void putfont8(char *vram, int xsize, int x, int y, char c, char *font)
{
    int i;
    char *p, d;
    for (i = 0; i < 16; i++) {
        p = vram + (y + i) * xsize + x;
        d = font[i];
        if ((d & 0x80) != 0) { p[0] = c; }
        if ((d & 0x40) != 0) { p[1] = c; }
        if ((d & 0x20) != 0) { p[2] = c; }
        if ((d & 0x10) != 0) { p[3] = c; }
        if ((d & 0x08) != 0) { p[4] = c; }
        if ((d & 0x04) != 0) { p[5] = c; }
        if ((d & 0x02) != 0) { p[6] = c; }
        if ((d & 0x01) != 0) { p[7] = c; }  // 注意：这里是0x01，不是0x81
    }
    return;
}

void putfonts8_asc(char *vram, int xsize, int x, int y, char c, unsigned char *s)
{
    extern char hankaku[4096];
    for (; *s != 0x00; s++) {
        putfont8(vram, xsize, x, y, c, hankaku + *s * 16);  // 传递颜色参数c
        x += 8;
    }
    return;
}
```

## 4. 遇到的问题与解决方案

### 问题1：IDT初始化错误导致死机

**原因分析**：

在IDT初始化循环中，没有除以8导致初始化了过多的IDT条目：

```
// 错误代码
for(i=0; i<=LIMIT_IDT; i++) {  // 没有除以8
    set_gatedesc(idt+i,0,0,0);
}
```

**问题解释**：

- `LIMIT_IDT`是字节数限制（例如0x7ff，即2047字节）
- IDT每个描述符占8字节
- 错误循环了2048次，但实际上只需要256次（2048÷8）
- 这导致IDT后面的内存被错误地初始化为中断门描述符
- 当中断发生时，CPU可能会跳转到错误地址执行，导致死机

**解决方案**：

正确除以8，确保只初始化有效的IDT条目：

```
// 正确代码
for(i=0; i<=LIMIT_IDT/8; i++) {  // 除以8
    set_gatedesc(idt+i,0,0,0);
}
```

### 问题2：字符显示为黑色，在黑色背景上看不见

**原因分析**：

`putfonts8_asc`函数中将颜色参数写死为0（黑色）：

```
putfont8(vram, xsize, x, y, 0, hankaku + *s * 16);  // 错误：写死了0
```

**解决方案**：

传递颜色参数`c`：

```
putfont8(vram, xsize, x, y, c, hankaku + *s * 16);  // 正确：使用参数c
```

### 问题3：字符"b"右上角有黑点

**原因分析**：

`putfont8`函数中最后一个像素判断错误：

```
if ((d & 0x81) != 0) { p[7] = c; }  // 错误：0x81检查了最高位和最低位
```

**解决方案**：

改为只检查最低位：

```
if ((d & 0x01) != 0) { p[7] = c; }  // 正确：0x01只检查最低位
```

## 5. 总结与感悟

### 知识点总结

1. **PIC初始化**：需要按照特定顺序写入ICW1-ICW4，设置中断向量偏移和连接方式。
2. **IDT初始化**：必须正确计算IDT条目数量，每个描述符占8字节。
3. **中断处理流程**：保存上下文→处理中断→发送EOI→恢复上下文→返回。
4. **键盘中断**：IRQ1对应中断号0x21，需要从端口0x60读取扫描码。
5. **鼠标中断**：IRQ12对应中断号0x2c，需要给从PIC和主PIC都发送EOI。
6. **字符显示**：注意颜色参数传递和像素处理细节。

### 经验教训

1. **IDT初始化必须准确**：IDT描述符是8字节结构，循环边界必须除以8，否则会破坏内存结构。
2. 中断处理程序必须快速执行并返回，不能进入死循环。
3. 必须发送EOI通知PIC中断处理完成。
4. 函数参数传递要仔细检查，避免写死固定值。
5. 位操作要特别注意掩码的正确性。

### 学习收获

通过今天的学习，深入理解了x86架构的中断处理机制，掌握了PIC初始化和中断处理程序的编写方法。通过调试各种问题，特别是IDT初始化错误导致的死机问题，提高了排查和解决bug的能力。这些知识为后续实现更复杂的操作系统功能打下了坚实基础。

------

*第六天学习结束，继续加油！*