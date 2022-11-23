#include <assert.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <fs.h>

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#define test_fs_error(fmt, ...) \
	fprintf(stderr, "%s: "fmt"\n", __func__, ##__VA_ARGS__)

#define die(...)				\
do {							\
	test_fs_error(__VA_ARGS__);	\
	exit(1);					\
} while (0)

#define die_perror(msg)			\
do {							\
	perror(msg);				\
	exit(1);					\
} while (0)

int main()
{
	if(fs_mount("disk.fs") != 0)
    {
        printf("Could not open virtual disk \n");
    }
    
	fs_ls();

	int fd1 = fs_open("mount_test.c");
	int fd2 = fs_open("info_test.c");
	int fd3 = fs_open("test_fs.c");

	printf("Size of fd1: %d \n", fs_stat(fd1));
	printf("Size of fd2: %d \n", fs_stat(fd2));
	printf("Size of fd3: %d \n", fs_stat(fd3));

	fs_close(fd1);
	fs_close(fd2);
	fs_close(fd3);

	fs_umount();

	return 0;
}
