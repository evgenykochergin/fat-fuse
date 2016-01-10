/*
   gcc -Wall fatfuse.c `pkg-config fuse --cflags --libs` -o fatfuse && ./fatfuse mounted-fs/

*/

#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

static const char *fatfuse_str = "fatfuse World!\n";
static const char *fatfuse_path = "/fatfuse";

static int fatfuse_getattr(const char *path, struct stat *stbuf)
{
	int res = 0;

	memset(stbuf, 0, sizeof(struct stat));
	if (strcmp(path, "/") == 0) {
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
	} else if (strcmp(path, fatfuse_path) == 0) {
		stbuf->st_mode = S_IFREG | 0444;
		stbuf->st_nlink = 1;
		stbuf->st_size = strlen(fatfuse_str);
	} else
		res = -ENOENT;

	return res;
}

static int fatfuse_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
			 off_t offset, struct fuse_file_info *fi)
{
	(void) offset;
	(void) fi;

	if (strcmp(path, "/") != 0)
		return -ENOENT;

	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);
	filler(buf, fatfuse_path + 1, NULL, 0);

	return 0;
}

static int fatfuse_open(const char *path, struct fuse_file_info *fi)
{
	if (strcmp(path, fatfuse_path) != 0)
		return -ENOENT;

	if ((fi->flags & 3) != O_RDONLY)
		return -EACCES;

	return 0;
}

static int fatfuse_read(const char *path, char *buf, size_t size, off_t offset,
		      struct fuse_file_info *fi)
{
	size_t len;
	(void) fi;
	if(strcmp(path, fatfuse_path) != 0)
		return -ENOENT;

	len = strlen(fatfuse_str);
	if (offset < len) {
		if (offset + size > len)
			size = len - offset;
		memcpy(buf, fatfuse_str + offset, size);
	} else
		size = 0;

	return size;
}

static struct fuse_operations fatfuse_oper = {
	.getattr	= fatfuse_getattr,
	.readdir	= fatfuse_readdir,
	.open		= fatfuse_open,
	.read		= fatfuse_read,
};

int main(int argc, char *argv[])
{
	return fuse_main(argc, argv, &fatfuse_oper, NULL);
}
