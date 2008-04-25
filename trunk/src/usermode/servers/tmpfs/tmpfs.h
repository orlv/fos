#ifndef TMPFS_H
#define TMPFS_H

#include <c++/list.h>

#define TYPE_DIR	(1 << 0)
#define TYPE_FILE	(1 << 1)
#define TYPE_HARDLINK	(1 << 2)

typedef struct __file {
	int	type;
	const	char *name;
	void	*data;
	int	size;	
	List	<struct __file *> *subitems;
} tmpfs_file_t;

class tmpfs {
	tmpfs_file_t	*root;
	int locate_object(const char *filename, List <struct __file *> **target, int allowed_types, tmpfs_file_t *parent);
public:
	tmpfs();
	int create_file(char *filename);
	void unlink(char *filename);
	int locate_file(char *filename, tmpfs_file_t **target);
	void stat(tmpfs_file_t *file, struct stat *target);
	int read(tmpfs_file_t *file, char *to, int size, int offset);
	int write(tmpfs_file_t *file, char *from, int size, int offset);
	int get_file_by_offset(tmpfs_file_t *dir, tmpfs_file_t **target, int offset);
	int open_dir(const char *filename, tmpfs_file_t **target);
	int posix_access(const char *buffer);
	int mkdir(const char *filename);
};

#endif 
