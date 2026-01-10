# 《30天自制操作系统》第五天学习笔记

## 目录

- 主要内容概述
- 具体知识点
  - 显示字符串和字体
  - 显示鼠标光标
  - GDT和IDT的初始化
  - 段描述符和门描述符的结构与设置
  - 汇编与C语言混合编程
- 代码分析
- 常见问题与解决方案
- 总结

------

## 主要内容概述

第五天的学习重点是操作系统内核的进一步开发，主要包括：

1. **显示字符串和字体**：通过字库文件显示英文字符
2. **显示鼠标光标**：实现鼠标光标的显示功能
3. **内存管理基础**：初始化GDT（全局描述符表）和IDT（中断描述符表）
4. **保护模式基础**：理解段描述符和门描述符的结构
5. **混合编程**：C语言与汇编语言的交互

------

## 具体知识点

### 显示字符串和字体

#### 1. 字体数据格式

- 使用8×16像素的点阵字体
- 每个字符占用16字节（8像素×16行，每行1字节）
- 整个字库包含256个字符，共4096字节
- 字体文件通过`makefont.exe`工具从文本文件转换

#### 2. 显示字符的原理

```c
void putfont8(char *vram, int xsize, int x, int y, char c, char *font)
```

- 逐行逐像素绘制字符
- 每个字节的8位对应一行的8个像素
- 通过位运算判断每个像素是否应该绘制

#### 3. 显示字符串函数

```c
void putfonts8_asc(char *vram, int xsize, int x, int y, char c, unsigned char *s)
```

- 循环显示字符串中的每个字符
- 每个字符占8像素宽度，显示后x坐标增加8

### 显示鼠标光标

#### 1. 鼠标光标设计

- 使用16×16像素的图案
- 三种状态：`*`（黑色轮廓）、`O`（白色内部）、`.`（背景色）
- 通过二维字符数组定义光标形状

#### 2. 光标缓冲区

```c
char mcursor[256];  // 16×16=256字节
```

- 存储处理后的光标像素数据
- 每个像素对应一个颜色索引

#### 3. 显示光标函数

```c
void putblock8_8(char *vram, int vxsize, int pxsize, int pysize, 
                 int px0, int py0, char *buf, int bxsize)
```

- 将光标缓冲区数据复制到显存
- 注意：传递的是缓冲区地址，不是缓冲区内容

### GDT和IDT的初始化

#### 1. GDT（全局描述符表）

- 用于内存分段管理
- 每个描述符8字节，最多8192个
- 必须包含一个空描述符（索引0）

#### 2. IDT（中断描述符表）

- 用于中断处理
- 每个门描述符8字节，共256个
- 包含中断处理程序的入口信息

#### 3. 初始化过程

```c
void init_gdtidt(void)
{
    // 1. 初始化GDT
    for(i=0; i<8192; i++) set_segmdesc(gdt+i, 0, 0, 0);
    set_segmdesc(gdt+1, 0xffffffff, 0x00000000, 0x4092);
    set_segmdesc(gdt+2, 0x0007ffff, 0x00280000, 0x409a);
    load_gdtr(0xffff, 0x00270000);
    
    // 2. 初始化IDT
    for(i=0; i<256; i++) set_gatedesc(idt+i, 0, 0, 0);
    load_idtr(0x7ff, 0x0026f800);
}
```

### 段描述符和门描述符的结构与设置

#### 1. 段描述符结构

```c
struct SEGMENT_DESCRIPTOR {
    short limit_low, base_low;
    char base_mid, access_right;
    char limit_high, base_high;
};
```

- **limit**：20位段界限
- **base**：32位段基址
- **access_right**：访问权限和标志位

#### 2. 设置段描述符

```c
void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, 
                  unsigned int limit, int base, int ar)
{
    if(limit > 0xfffff){
        ar |= 0x8000;  // 设置G位（粒度位）
        limit /= 0x1000;  // 转换为4KB页
    }
    // ... 位操作设置各个字段
}
```

#### 3. 门描述符结构

```c
struct GATE_DESCRIPTOR {
    short offset_low, selector;
    char dw_count, access_right;
    short offset_high;
};
```

- **offset**：32位中断处理程序偏移量
- **selector**：16位段选择子
- **dw_count**：调用门参数计数，中断门为0
- **access_right**：门的类型和权限

### 汇编与C语言混合编程

#### 1. 函数调用约定

- 参数从右向左压栈
- 调用者负责清理栈
- 返回值在EAX寄存器中

#### 2. 加载GDTR/IDTR的汇编实现

```assembly
_load_gdtr:  ; void load_gdtr(int limit, int addr)
    MOV AX, [ESP+4]    ; 读取limit参数
    MOV [ESP+6], AX    ; 构造6字节数据结构
    LGDT [ESP+6]       ; 加载GDTR
    RET
```

#### 3. 栈布局

```markdown
调用 load_gdtr(0xffff, 0x00270000) 时的栈布局：
ESP+0: 返回地址
ESP+4: limit参数 (0x0000ffff)
ESP+8: addr参数 (0x00270000)
```

------

## 代码分析

### 关键代码片段

#### 1. 鼠标光标初始化

```c
void init_mouse_cursor8(char *mouse, char bc)
{
    static char cursor[16][16] = {
        "**************..",
        "*OOOOOOOOOOO*...",
        // ... 其他行
    };
    // 将字符图案转换为颜色索引
    for(y=0; y<16; y++){
        for(x=0; x<16; x++){
            if(cursor[y][x]=='O') *(mouse+y*16+x)=7;      // 白色
            else if(cursor[y][x]=='.') *(mouse+y*16+x)=bc; // 背景色
            else if(cursor[y][x]=='*') *(mouse+y*16+x)=0;  // 黑色
        }
    }
}
```

#### 2. 错误用法与正确用法

```c
// 错误：传递数组第一个元素的值
init_mouse_cursor8(*mcursor, 14);
putblock8_8(..., *mcursor, ...);

// 正确：传递数组地址
init_mouse_cursor8(mcursor, 14);
putblock8_8(..., mcursor, ...);
```

#### 3. 段描述符的位操作

```c
sd->limit_high = ((limit >> 16) & 0x0f) | ((ar >> 8) & 0xf0);
// 高4位：flags (来自ar的高8位)
// 低4位：limit[19:16]
```

------

## 常见问题与解决方案

### 1. 编译警告：makes pointer from integer without a cast

**问题**：

```c
init_mouse_cursor8(*mcursor, 14);  // 警告
```

**原因**：传递了字符值（整数）而不是指针

**解决**：传递数组地址

```c
init_mouse_cursor8(mcursor, 14);   // 正确
```

### 2. 鼠标光标显示异常

**可能原因**：

- 字符比较错误：使用`'O'`（大写字母O）而不是`'0'`（数字0）
- 指针传递错误：传递`*mcursor`而不是`mcursor`

### 3. 段描述符设置错误

**检查点**：

- G位设置：当段界限超过1MB时需要设置G位
- 权限位：确保访问权限正确
- 地址对齐：确保段基址和界限正确

### 4. 内存对齐问题

**注意**：

- 结构体字段可能不连续
- 需要使用位操作组合字段
- 确保与硬件要求的格式一致

------

## 总结

第五天的学习内容涵盖了操作系统开发的多个核心概念：

1. **图形显示**：实现了字符和鼠标光标的显示
2. **内存管理**：初步了解了GDT和IDT的作用
3. **保护模式**：学习了段描述符和门描述符的结构
4. **混合编程**：掌握了C语言与汇编的交互方式

**关键收获**：

- 理解了指针和数组的区别
- 掌握了位操作在系统编程中的应用
- 了解了x86架构的保护模式基础
- 学会了如何调试和解决编译警告

**下一步学习重点**：

- 中断处理程序的实现
- 键盘和鼠标输入处理
- 多任务调度基础

------

*注：本笔记根据《30天自制操作系统》第五天内容整理，结合了实际操作中的问题和解决方案。*