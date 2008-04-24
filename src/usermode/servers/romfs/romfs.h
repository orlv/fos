#ifndef ROMFS_H
#define ROMFS_H
#include <types.h>
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
#define NEED_FILE	0
#define NEED_DIR	1
#define NEED_DIR_OR_FILE	2
	typedef struct {
		u32_t next;
		u32_t info;
		u32_t size;
		u32_t checksum;
		char name[];
	// data follow
	} romfs_inode_t;

class romfs
{
private:
	typedef struct {
		char signature[8];	// -rom1fs-
		u32_t size;
		u32_t checksum;
		char volume[];
	} romfs_superblock_t;


	char *fs;
	romfs_superblock_t *sb;
	int sb_size;
	
	int load_fs(const char *filename);
	int check_superblock();
	char *search_file(char *name, romfs_inode_t *in, romfs_inode_t *parent);
public:
	romfs(const char *filename);
	unsigned int read(romfs_inode_t *in, char *ptr, char *buf, size_t size, off_t offset);
	char * search_path(char *name, romfs_inode_t *inode, int need_type);
	void stat(romfs_inode_t *inode, struct stat *st);
	int get_ent_count(romfs_inode_t *inode);
	romfs_inode_t *get_inode(romfs_inode_t *parent, int offset);
	~romfs();
};
#endif
