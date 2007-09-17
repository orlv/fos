#ifndef JAR_H_
#define JAR_H_

#include "hash.h"

typedef
struct ZipFile_s {
	int length;
	
	unsigned char *data;
	hash_table_t  *dir;
} ZipFile_t;

ZipFile_t * zip_process(unsigned char *data, unsigned int len);
char      * zip_find_dir_entry(char *pathname, ZipFile_t *zip);
char      * zip_find_entry(char *pathname, ZipFile_t *zip, int *uncomp_len);

#endif /*JAR_H_*/
