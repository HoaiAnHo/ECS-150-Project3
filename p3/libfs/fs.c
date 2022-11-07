#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include "disk.h"
#include "fs.h"

#define ROOT_DIR_MAX 128;
#define FD_MAX 32;

/* Global Variables */
struct disk cur_disk; // global var for fs_info
int vd; // virtual disk file descriptor
int mounted = 0; // int check to see if vd is mounted

/* TODO: Phase 1 - VOLUME MOUNTING */

// very first block of the disk, contains information about filesystem
struct super_block{
	int64_t signature;  // 8 bytes
	int16_t total_blks; // 2 bytes
	int16_t root_dir_idx;  // 2 bytes
	int16_t data_blk_idx; // 2 bytes
	int16_t total_data_blks; // 2 bytes
	int8_t fat_blks; // 1 bytes
	int8_t padding[4079]; // 4079 bytes
}; 

// linked list structure for FAT entries
struct fat_blocks{
	struct fat_entry *front;
	struct fat_entry *back;
};

// singular fat block entry
struct fat_entry{
	int16_t entry; // 2 bytes
};

// array structure for FAT entries
struct root_blocks{
	struct root_entry entries[128];
};

struct root_entry{
	int8_t filename[16];
	int64_t file_size;
	int16_t first_data_idx;
	int8_t padding[10];
};

struct disk{
	struct super_block super;
	struct fat_blocks fat;
	struct root_blocks root;
};

int fs_mount(const char *diskname)
{
	if (!diskname) return -1;

	/* Open Virtual Disk*/
	vd = open(diskname, O_RDWR); // added "#include <unistd.h>" to use O_RDWR
	if (!vd) return -1;
	mounted = 1;

	/* Read the metadata (superblock, fat, root directory)*/
	// 1) Superblock - 1st 8 bytes
	struct super_block obj;
	read(vd, &obj, sizeof(obj));
	obj.signature = 0; // signature must be equal to "ECS150FS"
	obj.total_blks = 0;
	obj.root_dir_idx = 0;
	obj.data_blk_idx = 0;
	obj.fat_blks = 0;
	cur_disk.super = obj;
	// write(vd, &obj, sizeof(obj));


	// 2) FAT blocks - each block is 2048 entries, each entry is 16 bits
	// int fat_entry;
	// for (every FAT block){
	// 	if (corresponding data block is availible) fat_entry = 0;
	// 		else if (last data block of a file) fat_entry = FAT_EOC;
	// 		else fat_entry = index of next data block;
	// 	for (every blank entry in FAT block){
	// 		blank entry = fat_entry;
	// 	}
	// }

	// 3) Root directory - 1 block, 32-byte entry per file
	struct root_entry r_blocks[128];
	for (int i = 0; i < 128, i++){ // for every entry in root directory
		struct root_entry root_obj;
		read(vd, &root_obj, sizeof(root_obj));
		root_obj.filename = 0;
		root_obj.file_size = 0;
		root_obj.first_data_idx = 0;
		root_obj.padding = 0;
		r_blocks[i] = root_obj;
	}
	cur_disk.root = r_blocks;

	return 0;
}

int fs_umount(void)
{
	/* Close Virtual Disk */
	if (mounted == 0) return -1;
	close(vd);
	return 0;
}

int fs_info(void)
{
	if (mounted == 0) return -1;
	/* Show Info about Volume */
	// there should be a global class that contains the current vd info
	// we would then read from it if availible, and print the info

	return 0;
}

/* TODO: Phase 2 - FILE CREATION/DELETION */

int fs_create(const char *filename)
{
	/* TODO: Phase 2 */
	/* Create a new file */

	/* Initially, size is 0 and pointer to 1st block is FAT_EOC */
}

int fs_delete(const char *filename)
{
	/* TODO: Phase 2 */
	/* Delete an existing file */

	/* Free allocated data blocks, if any */
}

int fs_ls(void)
{
	/* TODO: Phase 2 */
	/* List all the existing files */
}

/* TODO: Phase 3 - FILE DESCRIPTOR OPERATIONS 
none of these functions should change the file system*/

int fs_open(const char *filename)
{
	/* TODO: Phase 3 */
	/* Initialize and return file descriptor */

	/* 32 file descriptors max 
	- Use FD_MAX macro */

	/* Can open same filee multiple times */

	/* Contains the file's offset (initially 0) */

}

int fs_close(int fd)
{
	/* TODO: Phase 3 */
	/* Close file descriptor */
}

int fs_stat(int fd)
{
	/* TODO: Phase 3 */
	/* return file's size */
}

int fs_lseek(int fd, size_t offset)
{
	/* TODO: Phase 3 */
	/* move file's offset */
}

/* TODO: Phase 4 - FILE READING/WRITING 
- THIS IS THE MOST COMPLICATED PHASE */

int fs_write(int fd, void *buf, size_t count)
{
	/* TODO: Phase 4 */
	/* Read a certain number of bytes from a file */
}

int fs_read(int fd, void *buf, size_t count)
{
	/* TODO: Phase 4 */
	/* Write a certain number of bytes to a file */

	/* Extend file if necessary */
}

