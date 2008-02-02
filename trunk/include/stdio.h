/*
  kernel/include/stdio.h
  Copyright (C) 2006 Oleg Fedorov
*/

#ifndef _STDIO_H
#define _STDIO_H

#include <types.h>
#include <unistd.h>
typedef void FILE;

#ifdef __cplusplus
extern "C" {
#endif
int printf(const char *fmt, ...);
int sprintf(char *str, const char *fmt, ...);
void fgets(char *buf, size_t size, FILE* handle);
int snprintf(char *str, size_t size, const char *fmt, ...);
FILE *fopen(const char *path, const char *mode);
FILE *fdopen(int fd, const char *mode);
FILE *freopen(const char *path, const char *mode, FILE *stream);
int fclose(FILE *fp);
int fflush(FILE *stream);
size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);
int fseek(FILE *stream, long offset, int whence);
void rewind(FILE *stream);
long ftell(FILE *stream);
int fputc(int c, FILE *stream);
int fgetc(FILE *stream);
int unlocked_fputc(int c, FILE *stream);
size_t unlocked_fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);
int fprintf(FILE *stream, const char *fmt, ...);

#define getc(a) fgetc(a)
#ifdef __cplusplus
}
#endif
#ifndef iKERNEL
extern FILE *stdin, *stdout, *stderr; 
#define getchar() fgetc(stdin)
#endif
#endif
