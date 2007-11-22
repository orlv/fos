#ifndef ROMFS_H
#define ROMFS_H
typedef struct {
	char signature[8];	// -rom1fs-
	u32_t size;
	u32_t checksum;
	char volume[];
} romfs_superblock_t;

typedef struct {
	u32_t next;
	u32_t info;
	u32_t size;
	u32_t checksum;
	char name[];
// data follow
} romfs_inode_t;

#define ROMFS_TYPE(a)	(a & 7)
#define ROMFS_EXECUTABLE(a)	(a & 8)
#define ROMFS_NEXT(a)	(a & 0xFFFFFFF0)

#define ROMFS_HARDLINK	0
#define ROMFS_DIRECTORY	1
#define ROMFS_FILE	2
#define ROMFS_SYMLINK	3
#define ROMFS_BLOCKDEV	4
#define ROMFS_CHARDEV	5
#define ROMFS_SOCKET	6
#define ROMFS_FIFO	7

#define ROMFS_ALIGN(a)	(((u32_t)a + 15) & ~15)

int romfs_init(void);
int romfs_read(char *path, char *buf, int size, int offset);
#endif
