#include "debug.h"
#include "common.h"
#include "console.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

void printch(char ch) //输出字符
{
    Console_WriteBytes(&ch,1);
}

void printint(int dec) //输出整数
{
    TraceInt4(dec,1);
}

void printstr(char* ptr) //输出字符串
{
   TraceMsg(ptr,1);
}

void printfloat(float flt) //输出浮点数，小数点第5位四舍五入
{
    int interpart = (int)flt;
    int fltpart = (int)(100000*(flt - interpart));

    if( fltpart % 10 >=5 )
    {
        fltpart = fltpart/10 + 1;
    }
    else
    {
        fltpart = fltpart/10;
    }

    printint(interpart);
    printch('.');
    printint(fltpart);
}

void QY_printf(char* format,...)
{
    va_list ap;
    va_start(ap,format);
    while(*format)
    {
        if(*format != '%')
        {
            printch(*format);
            format++;
        }
        else
        {
            format++;
            switch(*format)
            {
                case 'c':
                {
                    char valch = va_arg(ap,char);
                    printch(valch);
                    format++;
                    break;
                }
                case 'd':
                {
                    int valint = va_arg(ap,int);
                    printint(valint);
                    format++;
                    break;
                }
                case 's':
                {
                    char* valstr = va_arg(ap,char*);
                    printstr(valstr);
                    format++;
                    break;
                }
                case 'f':
                {
                    float valflt = va_arg(ap,double);
                    printfloat(valflt);
                    format++;
                    break;
                }
                default:
                {
                    printch(*format);
                    format++;
                    break;
                }
            }
        }
    }
    va_end(ap);
}

void print_stacktrace()
{
#if 0
    int size = 5;
    void * array[5];
    int stack_num = backtrace(array, size);
    char ** stacktrace = (char**)backtrace_symbols(array, stack_num);
    for (int i = 0; i < stack_num; ++i)
    {
        Console_WriteStringln(stacktrace[i]);
    }
    free(stacktrace);
#endif 
}