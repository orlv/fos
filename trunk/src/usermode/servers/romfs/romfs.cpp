#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
//#include <dprintf.h>
#include "romfs.h"

romfs::romfs(const char *filename) {
	load_fs(filename);
	check_superblock();
	printf("Loaded ROMFS image: '%s'\n", sb->volume);
}

int romfs::load_fs(const char *filename) {
//	struct stat st;
	int hndl = open(filename, 0);
	if(!hndl) 
		return 1;
	int size = lseek(hndl, 0, SEEK_END);
	lseek(hndl, 0, SEEK_SET);
//	fstat(hndl, &st);
	fs = new char[size];
	if(!fs)
		return 1;
	::read(hndl, fs, size);
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
	if(size + offset > in->size)
		size = in->size - offset;
//	dprintf("read %x %x %x %u %u (total size %u) %u%% from start, %u%% of total\n", in, ptr, buf, size, offset, in->size, offset * 100 / in->size, size * 100 / in->size);
	memcpy(buf, ptr + offset, size);
	return size;
}
char * romfs::search_file(char *name, romfs_inode_t *in, romfs_inode_t *parent) {
	if(parent == NULL) parent = (romfs_inode_t *)(fs + sb_size);
	if(!strlen(name)) {
		memcpy(in, parent, sizeof(*in));
		return (char *)in;
	}
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

char * romfs::search_path(char *name, romfs_inode_t *inode, int need_type) {
	char *part = new char[256];
	int i = 1;
	romfs_inode_t inl;
	romfs_inode_t *in = &inl;
	romfs_inode_t *parent = NULL;
	int type;
	char *ptr;
	int last_part = 0;
	while(1) {
		for(int j = 0; j < 256; j++, i++) {
			part[j] = name[i];
			if(part[j] == '/' || part[j] == 0) {
				if(part[j] == '/')
					last_part = 0;
				else
					last_part = 1;
				i++;
				part[j] = 0;
				break;
			}
		}
		if(!strcmp(part, ".") && !last_part) continue;
		ptr = search_file(part, in, parent);
		if(ptr == NULL) {
			delete part;
			return NULL;
		}
scan_inode:
		type = ROMFS_TYPE(in->next);
		if(type == ROMFS_DIRECTORY) {
			if(!last_part) {
				parent = (romfs_inode_t *)(fs + in->info);
				continue;
			} else if(need_type == NEED_DIR || need_type == NEED_DIR_OR_FILE) {
				memcpy(inode, in, sizeof(*inode));
				delete part;
				return (fs + in->info);
			} else {
				delete part;
				return NULL;
			}
		}
		if(type == ROMFS_FILE) {
			if(last_part && (need_type == NEED_FILE || need_type == NEED_DIR_OR_FILE)) {
				memcpy(inode, in, sizeof(*in));
				delete part;
				return ptr;
			} else {
				delete part;
				return NULL;
			}
		}
		if(type == ROMFS_HARDLINK) {
			if((romfs_inode_t *)(fs + inl.info) == in) {
				delete part;
				return NULL;
			}
			in = (romfs_inode_t *)(fs + inl.info);
			ptr = (char *)ROMFS_ALIGN(in + sizeof(*in) + strlen(in->name));
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
        if(ROMFS_TYPE(inode->next) == ROMFS_DIRECTORY)
		st->st_mode |= S_IFDIR;

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

int romfs::get_ent_count(romfs_inode_t *inode) {
	int i = 0;
	for(romfs_inode_t *ptr = inode;
			ptr != (romfs_inode_t *) fs;
			ptr = (romfs_inode_t *)(fs + ROMFS_NEXT(ptr->next)), i++);
	return i;
}
romfs_inode_t *romfs::get_inode(romfs_inode_t *parent, int offset) {
	int i = 0;
	for(romfs_inode_t *ptr = parent;
		ptr != (romfs_inode_t *) fs;
		ptr = (romfs_inode_t *)(fs + ROMFS_NEXT(ptr->next)), i++) {
		if(i == offset)
			return ptr;
	}
	return NULL;
}
