/*
 * Copyright (C) 2008 Sergey Gridassov
 */

#include <c++/list.h>
#include <string.h>
#include <stdio.h>
#include "tmpfs.h"


tmpfs::tmpfs() {
	root = new tmpfs_file_t;
	root->type = TYPE_DIR;
	root->name = "/";
	root->data = NULL;
	root->size = 2;
	root->subitems = new List <tmpfs_file_t *>();

	List <tmpfs_file_t *> *root_list = new List<tmpfs_file_t *>(root);
	root_list->add(NULL);

	tmpfs_file_t *root_this = new tmpfs_file_t;
	root_this->type = TYPE_HARDLINK;
	root_this->name = ".";
	root_this->data = root_list;
	root_this->size = 0;
	root_this->subitems = new List <tmpfs_file_t *>();

	tmpfs_file_t *root_parent = new tmpfs_file_t;
	root_parent->type = TYPE_HARDLINK;
	root_parent->name = "..";
	root_parent->data = root_list;
	root_parent->size = 0;
	root_parent->subitems = new List <tmpfs_file_t *>();


	root->subitems->add(root_this);
	root->subitems->add(root_parent);
}

int tmpfs::create_file(char *filename) {
	return -1;
}

void tmpfs::unlink(char *filename) {

}

int tmpfs::get_file_by_offset(tmpfs_file_t *dir, tmpfs_file_t **target, int offset) {
	return -1;
}

int tmpfs::open_dir(const char *filename, tmpfs_file_t **target) {
	return -1;
}

void tmpfs::stat(tmpfs_file_t *file, struct stat *target) {

}

int tmpfs::read(tmpfs_file_t *file, char *to, int size, int offset) {
	return 0;
}

int tmpfs::write(tmpfs_file_t *file, char *from, int size, int offset) {
	return 0;
}

int tmpfs::locate_file(char *filename, tmpfs_file_t **target) {
	return -1;
}

int tmpfs::locate_object(const char *filename, List <struct __file *> **target, int allowed_types, tmpfs_file_t *parent) {
//	printf("Looking for %s in %x with type %d, storing in %x\n", filename, parent, allowed_types, target);
	if(parent->type != TYPE_DIR) return -1;
	List <struct __file *> *ptr = NULL;

	if(filename[0] == '/') filename++;

	char *end = strchr(filename, '/') ;
	bool last = false;
	size_t len;

	if(end) {
		last = false;
		len = end - filename;
	} else {
		last = true;
		len = strlen(filename);
	}

//	printf("compare length %d last %d\n", len, last);

	list_for_each(ptr, parent->subitems) {
//		printf("found %s, need %d bytes from %s\n", ptr->item->name, len, filename);
		if(strlen(ptr->item->name) >= (len - 1) && memcmp(ptr->item->name, filename, len) == 0) {
scan:
			if(!last && ptr->item->type != TYPE_DIR) {
//				printf("not last and not a dir\n");
				return -1;
			} else if(last && allowed_types & ptr->item->type) {	// конец пути и поддерживаемый тип
		//		printf("found!\n");
				*target = ptr;
				return 0;
			} else if(ptr->item->type == TYPE_HARDLINK) {
//				printf("hardlink\n");
				ptr = (List <struct __file *> *)ptr->item->data;
				goto scan;
			} else if(last) {					// последний элемент пути, не поддерживаемый тип
//				printf("last item, unsupported type\n");
				return -1;
			} else {
//				printf("recursion\n");
				return locate_object(end, target, allowed_types, ptr->item);	// продолжаем поиск
			}
		}
	}

	return -1;
}

int tmpfs::posix_access(const char *buffer) {
	printf("tmpfs: accessing %s\n", buffer);
	List	<struct __file *> *file;

	if(locate_object(buffer, &file, TYPE_DIR | TYPE_FILE, root) < 0)
		return -1;

	printf("tmpfs: found\n");

	return 0;
}

int tmpfs::mkdir(const char *filename) {
	printf("tmpfs: creating %s\n");
	char *parent = strdup(filename);
	char *last = strrchr(parent, '/');
	if(last == NULL || last == parent) {
		delete parent;
		return -1;
	}

	parent[last - parent] = 0;
	printf("tmpfs: parent: %s\n", parent);
	delete parent;

	return -1;
}
