#ifndef TMPFS_H
#define TMPFS_H
typedef struct file {
	struct file *next;
	int size;
	char *data;
	char *name;
} tmpfs_file;

class tmpfs {
	tmpfs_file *list;
public:
	tmpfs();
	void create_file(char *filename);
	void unlink(char *filename);
	int locate_file(char *filename, tmpfs_file **target);
	void stat(tmpfs_file *file, struct stat *target);
	int read(tmpfs_file *file, char *to, int size, int offset);
	int write(tmpfs_file *file, char *from, int size, int offset);
	int files_count();
	void first_file(tmpfs_file **target);
	int get_file_by_offset(tmpfs_file **target, int offset);
};

#endif 
