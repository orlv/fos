#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include "romfs.h"
romfs::romfs(char *filename) {
	load_fs(filename);
	check_superblock();
	printf("Loaded ROMFS image: '%s'\n", sb->volume);
}

int romfs::load_fs(char *filename) {
	struct stat st;
	int hndl = open(filename, 0);
	if(!hndl) 
		return 1;
	fstat(hndl, &st);
	fs = new char[st.st_size];
	if(!fs)
		return 1;
	::read(hndl, fs, st.st_size);
	close(hndl);
	return 0;
}

int romfs::check_superblock() {
	sb = (romfs_superblock_t *) fs;
	sb_size = ROMFS_ALIGN(sizeof(romfs_superblock_t) + strlen(sb->volume));
	if(!strncmp("-rom1fs-", sb->signature, 8)) {
		printf("Use patched (little-endian) ROMFS. Sorry.\n");
		return 1;
	}
	if(strncmp("mor--sf1", sb->signature, 8)) {
		printf("Invalid signature\n");
		return 1;
	}
	return 0;
}

unsigned int romfs::read(romfs_inode_t *in, char *ptr, char *buf, size_t size, off_t offset) {
	if(offset > in->size)
		return 0;
	if(size > in->size)
		size = in->size;
//	printf("read %x %x %x %u %u\n", in, ptr, buf, size, offset);
	memcpy(buf, ptr + offset, size);
	return size;
}
char * romfs::search_file(char *name, romfs_inode_t *in, romfs_inode_t *parent) {
	if(parent == NULL) parent = (romfs_inode_t *)(fs + sb_size);
	int i = 0;
	for(romfs_inode_t *ptr = parent;
			ptr != (romfs_inode_t *) fs;
			ptr = (romfs_inode_t *)(fs + ROMFS_NEXT(ptr->next)), i++) {
		if(!strcmp(ptr->name, name)) {

			memcpy(in, ptr, sizeof(*in));
			return (char *)ROMFS_ALIGN(ptr + sizeof(*in) + strlen(name));
		}
	}
	return NULL;
}

char * romfs::search_path(char *name, romfs_inode_t *inode) {
	char *part = new char[256];
	int i = 1;
	romfs_inode_t inl;
	romfs_inode_t *in = &inl;
	romfs_inode_t *parent = NULL;
	int type;
	char *ptr;
	while(1) {
		for(int j = 0; j < 256; j++, i++) {
			part[j] = name[i];
			if(part[j] == '/' || part[j] == 0) {
				i++;
				part[j] = 0;
				break;
			}
		}
		ptr = search_file(part, in, parent);
		if(ptr == NULL) {
			delete part;
			return NULL;
		}
scan_inode:
		type = ROMFS_TYPE(in->next);
		if(type == ROMFS_DIRECTORY) {
			parent = (romfs_inode_t *)(fs + in->info);
			continue;
		}
		if(type == ROMFS_FILE) {
			memcpy(inode, in, sizeof(*in));
			delete part;
			return ptr;
		}
		if(type == ROMFS_HARDLINK) {
			in = (romfs_inode_t *)(fs + inl.info);
			ptr = (char *)ROMFS_ALIGN(in + sizeof(*in) + strlen(in->name));
			printf("hardlink to inode %x\n", in);
			goto scan_inode;
		}
		printf("unknown type - %x\n", type);
		break;
	}
	delete part;
	return NULL;
}

void romfs::stat(romfs_inode_t *inode, struct stat* st) {
	st->st_dev     = 0;
	st->st_ino     = inode->checksum;
	st->st_mode    = 0777;
	st->st_nlink   = 1;
	st->st_uid     = 0;
	st->st_gid     = 0;
	st->st_rdev    = 0;
      
	st->st_size    = inode->size;
      
	st->st_blksize = 1;
	st->st_blocks  = inode->size;
	st->st_atime   = 0;
	st->st_mtime   = 0;
	st->st_ctime   = 0;
}

