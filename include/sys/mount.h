/*
 * Copyright (C) 2008 Sergey Gridassov
 */

#ifndef __SYS_MOUNT_H
#define __SYS_MOUNT_H

#ifdef __cplusplus
extern "C" {
#endif

int mount(const char *source, const char *target, const char *filesystemtype, unsigned long mountflags, const void *data);

int ping(const char *target);

#ifdef __cplusplus
}
#endif

#endif

