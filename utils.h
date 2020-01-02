#ifndef _UTILS_H_
#define _UTILS_H_
#include <stdio.h>
#include <sys/types.h> 
#include <unistd.h> 
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include <time.h>
#include <pthread.h>

int safe_asprintf(char **strp, const char *fmt, ...);
int safe_vasprintf(char **strp, const char *fmt, va_list ap);
void plog(const char *format, ...) ;
void pinfo(const char *format, ...) ;
void pflog(const char *format, ...);
void pfinfo(const char *format, ...);

#define DEBUG

#ifdef DEBUG
void pflog(const char *format, ...);
void pfinfo(const char *format, ...);
void plog(const char *format, ...);
void pinfo(const char *format, ...);
#define debug(fmt, args...) plog(fmt, ##args) 
#else
#define debug(fmt, args...) do{}while(0)
#endif

static pthread_mutex_t fileMutex = PTHREAD_MUTEX_INITIALIZER;

#endif
