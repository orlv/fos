#include <fcntl.h>
#include <stdio.h>
#include <types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "romfs.h"

static char *romfs;
static romfs_superblock_t *sb;
static int load_fs(char *filename);
static int check_superblock();
static int sb_size;
static char * search_path(char *name, romfs_inode_t *inode);
static char * search_file(char *name, romfs_inode_t *in, romfs_inode_t *parent);

int romfs_init() {
	if(load_fs("/mnt/modules/initrd.gz")) {
		printf("Loading FS image failed\n");
		return 1;
	}
	if(check_superblock())
		return 1;
	printf("Loaded ROMFS image: '%s'\n", sb->volume);

	printf("Trying to read /etc/RLY/test\n");
	char *buf = malloc(256);
	int readed = romfs_read("/etc/RLY/test", buf, 256, 0);
	printf("Readed %u bytes\n", readed);
	if(!readed) {
		printf("Reading failure.\n");
		return 1;
	}
	buf[readed] = 0;
	printf("contents:\n%s\n", buf);
	printf("completed.\n");
	return 0;
} 

static int check_superblock() {
	sb = (romfs_superblock_t *) romfs;
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

int romfs_read(char *path, char *buf, int size, int offset) {
	romfs_inode_t in;
	char *ptr = search_path(path, &in);
	if(ptr == NULL)
		return 0;
	if(offset > in.size)
		return 0;
	if(size > in.size)
		size = in.size;
	memcpy(buf, ptr + offset, size - offset);
	return size - offset;
}

static int load_fs(char *filename) {
	struct stat st;
	int hndl = open(filename, 0);
	if(!hndl) 
		return 1;
	fstat(hndl, &st);
	romfs = malloc(st.st_size);
	if(!romfs)
		return 1;
	int readed = read(hndl, romfs, st.st_size);
//	char *foo = (char *)  romfs;
//	for(int i=0; i<1024; i++)
//	  printf("%c",foo[i]);
	close(hndl);
	printf("Readed %d bytes vs %d\n", readed, st.st_size);
	return 0;
}

static char * search_path(char *name, romfs_inode_t *inode) {
	char *part = malloc(256);
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
		if(ptr == NULL)
			return NULL;
		printf("part %s found\n", part);
scan_inode:					// да, я знаю что goto - 3,14здец. но это короче, чем куча вложенных циклов.
		type = ROMFS_TYPE(in->next);
		if(type == ROMFS_DIRECTORY) {
			parent = (romfs_inode_t *)(romfs + in->info);
			continue;
		}
		if(type == ROMFS_FILE) {
			memcpy(inode, in, sizeof(*in));
			return ptr;
		}
		if(type == ROMFS_HARDLINK) {
			in = (romfs_inode_t *)(romfs + inl.info);
			ptr = (char *)ROMFS_ALIGN(in + sizeof(*in) + strlen(in->name));
			goto scan_inode;
		}
		printf("unknown type - %x\n", type);
		break;
	}
	return NULL;
}

static char * search_file(char *name, romfs_inode_t *in, romfs_inode_t *parent) {
	if(parent == NULL) parent = (romfs_inode_t *)(romfs + sb_size);
	int i = 0;
	for(romfs_inode_t *ptr = parent;
			ptr != (romfs_inode_t *) romfs;
			ptr = (romfs_inode_t *)(romfs + ROMFS_NEXT(ptr->next)), i++) {
		printf("%s:%s\n", ptr->name, name);
		if(!strcmp(ptr->name, name)) {
		  /*			printf("               00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F");
			char *p = ROMFS_ALIGN(ptr) - 16;
			for(int i = 0; i < 256; i++) {
				if(!(i % 16))
					printf("\n%06x: %03x: ", p + i, i);
				if(((char *) p + i) ==  ROMFS_ALIGN(ptr + sizeof(*in) + strlen(name))) 
					printf("\033[0;38;40m>\033[0m");
				else
					printf(" ");
				printf("%02x", *((char *) p + i) & 0xFF);
			}
			printf("\n ---- Data ptr: %x ----\n", ROMFS_ALIGN(ptr + sizeof(*in) + strlen(name)));*/
			memcpy(in, ptr, sizeof(*in));
			return (char *)ROMFS_ALIGN(ptr + sizeof(*in) + strlen(name));
		}
	}
	return NULL;
}
