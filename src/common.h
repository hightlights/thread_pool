#ifndef __COMMOM_H__
#define __COMMOM_H__

#include <stdio.h>
#include <stdarg.h>


#define DEBUG
#ifdef DEBUG
    #define LOG(...) \
    do { \
        printf("[ %s:%d ] Massage : ", __FILE__, __LINE__); \
        printf(__VA_ARGS__); \
        printf("\n"); \
    } while (0)
#else
    #define LOG(info, ...)
#endif

#endif