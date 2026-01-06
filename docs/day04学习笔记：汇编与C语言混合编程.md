# Day4 学习笔记：汇编与C语言混合编程

## 1. 第四天主要内容

第四天学习了如何用汇编语言编写底层函数，然后由C语言调用这些函数来实现硬件操作。具体包括：

- 编写汇编函数实现基本的I/O操作（如`io_hlt`, `io_cli`, `io_sti`, `io_in8`, `io_out8`等）
- 理解C语言调用汇编函数的约定（调用约定）
- 使用这些汇编函数在C语言中设置VGA调色板
- 理解调色板的工作原理和端口I/O

## 2. 汇编函数与C语言调用约定

### 2.1 寄存器使用规则

在x86架构中，有一些寄存器的使用是受到限制的，特别是在进行I/O操作时。下表总结了常用寄存器的用途：

| 寄存器 | 用途                                        | 是否可随意使用？        |
| ------ | ------------------------------------------- | ----------------------- |
| EAX    | 通用寄存器，通常用于函数返回值              | 是，但返回值必须放在EAX |
| EBX    | 通用寄存器                                  | 是                      |
| ECX    | 通用寄存器，常用于计数                      | 是                      |
| EDX    | 通用寄存器，但在I/O指令中必须用于存放端口号 | 否，I/O指令中固定用途   |
| ESP    | 栈指针，指向当前栈顶                        | 绝对不能乱动            |
| EBP    | 帧指针，用于访问函数参数和局部变量          | 通常保留                |

### 2.2 C语言调用约定（cdecl）

在32位x86架构上，C语言使用cdecl调用约定，其规则如下：

- 参数从右向左依次压栈
- 调用者负责清理栈（在函数调用后调整栈指针）
- 返回值存放在EAX寄存器中

例如，对于函数调用`io_out8(0x3f8, 'A')`，其汇编代码大致如下：

```assembly
push 'A'        ; 第二个参数（数据）
push 0x3f8      ; 第一个参数（端口号）
call _io_out8   ; 调用函数，同时将返回地址压栈
add esp, 8      ; 调用者清理栈（两个参数共8字节）
```

在函数`_io_out8`内部，栈的布局如下：

```markdown
高地址
+------------------+
|    'A'           |  [ESP+8]  第二个参数（数据）
+------------------+
|    0x3f8         |  [ESP+4]  第一个参数（端口号）
+------------------+
|    返回地址       |  [ESP]    由call指令压入
+------------------+  <- ESP（当前栈顶）
```

因此，在汇编函数中，我们可以通过`[ESP+4]`访问第一个参数，通过`[ESP+8]`访问第二个参数。

### 2.3 I/O指令的固定寄存器

x86架构的I/O指令（IN和OUT）对寄存器有硬性要求：

- 端口号必须由DX寄存器指定（如果端口号大于255，必须使用DX）
- 数据必须使用AL/AX/EAX（根据数据宽度选择）

例如：

- `IN AL, DX`：从DX指定的端口读取一个字节到AL
- `OUT DX, AL`：将AL中的字节输出到DX指定的端口

## 3. 汇编函数详解

### 3.1 简单封装函数

这些函数直接对应一条汇编指令，用于控制CPU的行为。

```assembly
_io_hlt:    ; void io_hlt(void);
    HLT     ; 使CPU进入休眠状态，直到中断发生
    RET

_io_cli:    ; void io_cli(void);
    CLI     ; 清除中断标志（禁止可屏蔽中断）
    RET

_io_sti:    ; void io_sti(void);
    STI     ; 设置中断标志（允许可屏蔽中断）
    RET

_io_stihlt: ; void io_stihlt(void);
    STI     ; 允许中断
    HLT     ; 然后休眠
    RET
```

### 3.2 I/O端口读写函数

这些函数用于与硬件设备通信，通过I/O端口读写数据。

#### 读取端口数据

```assembly
_io_in8:    ; int io_in8(int port);
    MOV EDX, [ESP+4]   ; 端口号 -> EDX
    MOV EAX, 0         ; 清空EAX，避免返回时高24位有垃圾数据
    IN AL, DX          ; 从端口读一个字节到AL
    RET                ; 返回值在EAX（低8位有效）

_io_in16:   ; int io_in16(int port);
    MOV EDX, [ESP+4]
    MOV EAX, 0
    IN AX, DX          ; 从端口读两个字节到AX
    RET

_io_in32:   ; int io_in32(int port);
    MOV EDX, [ESP+4]
    IN EAX, DX         ; 从端口读四个字节到EAX
    RET                ; 返回值就是EAX
```

#### 向端口写入数据

```assembly
_io_out8:   ; void io_out8(int port, int data);
    MOV EDX, [ESP+4]   ; 端口号 -> EDX
    MOV AL, [ESP+8]    ; 数据（8位）-> AL
    OUT DX, AL         ; 将AL中的数据输出到端口
    RET

_io_out16:  ; void io_out16(int port, int data);
    MOV EDX, [ESP+4]
    MOV AX, [ESP+8]    ; 数据（16位）-> AX
    OUT DX, AX
    RET

_io_out32:  ; void io_out32(int port, int data);
    MOV EDX, [ESP+4]
    MOV EAX, [ESP+8]   ; 数据（32位）-> EAX
    OUT DX, EAX
    RET
```

### 3.3 标志寄存器操作

```assembly
_io_load_eflags:    ; int io_load_eflags(void);
    PUSHFD          ; 将EFLAGS压栈
    POP EAX         ; 弹出到EAX
    RET             ; 返回值就是EFLAGS

_io_store_eflags:   ; void io_store_eflags(int eflags);
    MOV EAX, [ESP+4] ; 参数eflags -> EAX
    PUSH EAX         ; 将eflags压栈
    POPFD            ; 弹出到EFLAGS寄存器
    RET
```

## 4. 调色板设置代码分析

### 4.1 调色板原理

VGA显卡在256色模式下，每个像素的值（0-255）实际上是一个索引，指向调色板中的一种颜色。调色板由256个颜色项组成，每个颜色项由3个字节（R、G、B）表示，每个颜色分量的范围是0-63（6位）。

### 4.2 设置调色板的步骤

1. 禁止中断（避免设置过程中被中断打断）
2. 向端口0x03c8写入要设置的起始颜色索引
3. 依次向端口0x03c9写入R、G、B值（每个分量除以4，将8位转换为6位）
4. 恢复中断标志

### 4.3 C语言代码

```c
void set_palette(int start, int end, unsigned char *rgb) {
    int i, eflags;
    eflags = io_load_eflags(); // 保存当前中断标志
    io_cli();                   // 禁止中断
    io_out8(0x03c8, start);     // 设置起始索引
    for (i = start; i <= end; i++) {
        io_out8(0x03c9, rgb[0] / 4); // R
        io_out8(0x03c9, rgb[1] / 4); // G
        io_out8(0x03c9, rgb[2] / 4); // B
        rgb += 3;
    }
    io_store_eflags(eflags);    // 恢复中断标志
    return;
}
```

## 5. 总结

第四天的学习让我们掌握了：

1. **汇编与C语言混合编程**：学会了如何用汇编编写底层函数，并由C语言调用
2. **调用约定**：理解了cdecl调用约定，以及参数传递和返回值处理的规则
3. **I/O端口操作**：学会了通过IN和OUT指令与硬件设备通信
4. **调色板设置**：理解了VGA调色板的工作原理，并实现了设置调色板的函数
5. **中断控制**：学会了在关键操作期间禁止和恢复中断

这些知识是操作系统开发的基础，后续的设备驱动开发都会建立在这样的底层I/O操作之上。

## 6. 常见问题

### 6.1 为什么在I/O操作中要使用特定的寄存器？

这是x86架构的设计决定的，IN和OUT指令硬性要求使用DX指定端口，AL/AX/EAX传输数据。

### 6.2 为什么设置调色板时要禁止中断？

为了防止设置过程被中断打断，导致调色板数据写入不完整，造成显示异常。

### 6.3 为什么调色板的RGB值要除以4？

因为VGA硬件只支持6位颜色分量（0-63），而我们的RGB值是8位（0-255），除以4相当于右移2位，将8位值转换为6位。

------

*笔记生成时间：2025-12-26*

*作者：o7y666*