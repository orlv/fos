/*
 * Copyright (C) 2008 Sergey Gridassov
 */

#ifndef PIC	/* не включать сообщения о ошибках в статические библиотеки */
	#define SAVE_SPACE
#endif

const char *sys_errlist[] = {
	"Success",			/* ESUCCESS, 0 */	
#ifndef SAVE_SPACE
	"Operation not permitted",	/* EPERM, 1  */
	"No such file or directory",	/* ENOENT, 2 */
	"No such process",		/* ESRCH, 3 */
	"Interrupted system call",	/* EINTR, 4 */
#endif
};

int sys_nerr = sizeof(sys_errlist) / sizeof(*sys_errlist);
