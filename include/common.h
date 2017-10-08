#ifndef __COMMON_H__
#define __COMMON_H__
#include <stdint.h>
#include <stdio.h>

#if (defined (LINUX) || defined (__linux__))
#define _GNU_SOURCE
#include <unistd.h>
#include <sys/syscall.h>
#endif	//LINUX

#ifndef U32
  typedef unsigned int U32;
#endif
#ifndef U16
  typedef unsigned short U16;
#endif
#ifndef U8
  typedef unsigned char U8;
#endif
#ifndef S32
  typedef signed int S32;
#endif
#ifndef S16
  typedef signed short S16;
#endif
#ifndef S8
  typedef signed char S8;
#endif

#define LOG_LEVEL_BUG		0
#define LOG_LEVEL_ERROR		1
#define LOG_LEVEL_WARNING	2
#define LOG_LEVEL_MESSAGE	3
#define LOG_LEVEL_DEBUG		4
/*********************************************/
#if defined(DEBUG) || defined(_DEBUG)
  #ifndef HX_DEBUG
    #define HX_DEBUG 1
  #endif
#endif	
/*********************************************/
#if defined(__KERNEL__) || defined(MODULE)
  #include <linux/kernel.h>
  #define HX_PRINT_D(x...)	printk(x)
  #define HX_PRINT_E(x...)	printk(x)
  #define HX_STD_ERR		KERN_INFO
#else
  #include <stdio.h>
  #define HX_PRINT_D(x...)	printf(x)
  #define HX_PRINT_E(x...)	fprintf(x)
  #define HX_STD_ERR		stderr	
#endif
/*********************************************/
#if defined(HX_DEBUG)
  #define DBG(fmt, args...)   HX_PRINT_D("[DBG@%s:%d]: "fmt"\n", __FUNCTION__, __LINE__, ##args)
#else
  #define DBG(fmt, args...)
#endif

#ifndef MSG
  #define MSG(fmt, args...)   HX_PRINT_D("[MSG@%s:%d]: "fmt"\n", __FUNCTION__, __LINE__, ##args)
#endif

#ifndef WRN
  #define WRN(fmt, args...)   HX_PRINT_D("[WRN@%s:%d]: "fmt"\n", __FUNCTION__, __LINE__, ##args)
#endif

#ifndef ERR
  #define ERR(fmt, args...)  HX_PRINT_E(HX_STD_ERR, "[ERR@%s:%d]: " fmt "\n", __FUNCTION__, __LINE__, ##args)
#endif

#ifndef BUG
  #define BUG(fmt, args...)		\
	do{				\
		HX_PRINT_E(HX_STD_ERR, "[BUGR@%s:%d]: " fmt "\n", __FUNCTION__, __LINE__, ##args);\
		while(1)		\
			sleep(1);	\
	}while(0)
#endif
/*********************************************/
#ifndef likely
  #define likely(x)	__builtin_expect(!!(x), 1)
#endif
#ifndef unlikely
  #define unlikely(x)	__builtin_expect(!!(x), 0)
#endif
/*********************************************/
#if defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && (__GNUC_MINOR__ > 4)))
#ifndef GCC_PUSH_OPTION
  #define GCC_PUSH_OPTION		_Pragma("GCC push_options")
#endif
#ifndef GCC_POP_OPTION
  #define GCC_POP_OPTION		_Pragma("GCC pop_options")
#endif
#ifndef GCC_OPTIMIZE_O0
  #define GCC_OPTIMIZE_O0		_Pragma("GCC optimize (\"O0\")")
#endif
#else   //__GNUC__
#ifndef GCC_PUSH_OPTION
  #define GCC_PUSH_OPTION
#endif
#ifndef GCC_POP_OPTION
  #define GCC_POP_OPTION
#endif
#ifndef GCC_OPTIMIZE_O0
  #define GCC_OPTIMIZE_O0
#endif
#endif  //__GNUC__
/*********************************************/
#ifndef __STRING
  #define __STRING(x)	#X
#endif
#ifndef STRING
  #define STRING(x)	__STRING(x)
#endif
#ifndef __PASTE
  #define __PASTE(x,y)	x ## y
#endif
#ifndef PASTE
  #define PASTE(x,y)	__PASTE(x,y)
#endif
#ifndef UNIQUE
  #ifdef __GNUC__
     #define UNIQUE(x)	PASTE(x, PASTE(__COUNTER__,__LINE__))
  #else
     #define UNIQUE(x)	PASTE(x, __LINE__)
  #endif
#endif
/*********************************************/
#ifndef ARRAY_SIZE
	#define ARRAY_SIZE(x)	(sizeof(x)/sizeof(x[0]))
#endif
/*********************************************/
#ifndef GETTID
	#define GETTID()	syscall(SYS_gettid)
#endif
/*********************************************/
#endif
