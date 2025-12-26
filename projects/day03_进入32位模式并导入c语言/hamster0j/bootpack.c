/*告诉c 编译器,有一个函数在别的文件里面*/
void io_hlt(void);

void HariMain(void)
{

fin:
    io_hlt(); //执行naskfunc.nas里的_io_hlt
    goto fin;

}
