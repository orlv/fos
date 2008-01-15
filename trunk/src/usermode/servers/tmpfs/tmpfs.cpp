#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "tmpfs.h"

tmpfs::tmpfs() {
	list = NULL;
}

void tmpfs::unlink(char *filename) {
	for(tmpfs_file *ptr = list, *prev = NULL; ptr; prev = ptr, ptr = ptr->next) {
		if(!strcmp(ptr->name, filename)) {
			free(ptr->name);
			if(ptr->data)
				free(ptr->data);
			if(prev)
				prev->next = ptr->next;
			else
				list = ptr->next;
			free(ptr);
			break;
		}
	}
}

void tmpfs::create_file(char *filename) {
	filename++;
	unlink(filename);
	tmpfs_file *file = new tmpfs_file;
	file->next = list;
	file->size = 0;
	file->data = NULL;
	file->name = new char [strlen(filename) + 1];
	strcpy(file->name, filename);
	list = file;
}

int tmpfs::locate_file(char *filename, tmpfs_file **target) {
	filename++;
	for(tmpfs_file *ptr = list, *prev = NULL; ptr; prev = ptr, ptr = ptr->next) {
		if(!strcmp(ptr->name, filename)) {
			*target = ptr;
			return 0;
		}
	}
	return -1;
}

void tmpfs::stat(tmpfs_file *file, struct stat *target) {
	target->st_dev     = 0;
	target->st_ino     = (u32_t)file;
	target->st_mode    = 0777;
	target->st_nlink   = 1;
	target->st_uid     = 0;
	target->st_gid     = 0;
	target->st_rdev    = 0;
      
	target->st_size    = file->size;
      
	target->st_blksize = 1;
	target->st_blocks  = file->size;
	target->st_atime   = 0;
	target->st_mtime   = 0;
	target->st_ctime   = 0;
}

int tmpfs::read(tmpfs_file *file, char *to, int size, int offset) {
	if(size > file->size)
		size = file->size;
	if(offset > file->size)
		return 0;

	memcpy(to, file->data + offset, size);
	return size;
}

int tmpfs::write(tmpfs_file *file, char *from, int size, int offset) {
	if(size + offset > file->size) {
		file->size = size + offset;
		file->data = (char *)realloc(file->data, file->size);
	}
	memcpy(file->data + offset, from, size);
	return size;
}

int tmpfs::files_count() {
	int count = 0;
	for(tmpfs_file *ptr = list, *prev = NULL; ptr; prev = ptr, ptr = ptr->next)
		count++;
	return count;
}

void tmpfs::first_file(tmpfs_file **target) {
	*target = list;
}

int tmpfs::get_file_by_offset(tmpfs_file **target, int offset) {
	int i = 0;
	for(tmpfs_file *ptr = list, *prev = NULL; ptr; prev = ptr, ptr = ptr->next) {
		if(i == offset) {
			*target = ptr;
			return 0;
		}
		i++;
	}
	return -1;
}
