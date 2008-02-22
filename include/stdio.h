/*
  kernel/include/stdio.h
  Copyright (C) 2006 Oleg Fedorov
*/

#ifndef _STDIO_H
#define _STDIO_H

#include <types.h>
#include <unistd.h>
#include <stdarg.h>

typedef void FILE;

userlinkage int printf(const char *fmt, ...);
userlinkage int sprintf(char *str, const char *fmt, ...);
userlinkage void fgets(char *buf, size_t size, FILE* handle);
userlinkage int snprintf(char *str, size_t size, const char *fmt, ...);
userlinkage FILE *fopen(const char *path, const char *mode);
userlinkage FILE *fdopen(int fd, const char *mode);
userlinkage FILE *freopen(const char *path, const char *mode, FILE *stream);
userlinkage int fclose(FILE *fp);
userlinkage int fflush(FILE *stream);
userlinkage size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
userlinkage size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);
userlinkage int fseek(FILE *stream, long offset, int whence);
userlinkage void rewind(FILE *stream);
userlinkage long ftell(FILE *stream);
userlinkage int fputc(int c, FILE *stream);
userlinkage int fgetc(FILE *stream);
userlinkage int unlocked_fputc(int c, FILE *stream);
userlinkage size_t unlocked_fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);
userlinkage int fprintf(FILE *stream, const char *fmt, ...);

userlinkage int vsprintf(char *buf, const char *fmt, va_list args);
userlinkage int vfprintf(FILE *file, const char *fmt, va_list args);
userlinkage int sprintf(char *str, const char *fmt, ...);
userlinkage int vsnprintf(char *buf, size_t size, const char *fmt, va_list args);

  
#define getc(a) fgetc(a)

#ifndef iKERNEL
extern FILE *stdin, *stdout, *stderr; 
#define getchar() fgetc(stdin)
#endif

#endif
