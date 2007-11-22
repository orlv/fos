#include <types.h>

#include "romfs.h"
int main(int argc, char *argv[]) {
	if(romfs_init())
		return 1;
	return 0;
}
