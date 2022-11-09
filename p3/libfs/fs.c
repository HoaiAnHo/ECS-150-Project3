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
#define FAT_EOC 0xffff;

/* Global Variables */
struct disk_blocks cur_disk; // global var for fs_info

/* TODO: Phase 1 - VOLUME MOUNTING */

// very first block of the disk, contains information about filesystem
struct super_block{
	int64_t signature;  // 8 bytes, signature must be equal to "ECS150FS"
	int16_t total_blks; // 2 bytes
	int16_t root_dir_idx;  // 2 bytes
	int16_t data_blk_idx; // 2 bytes
	int16_t total_data_blks; // 2 bytes
	int8_t fat_blks; // 1 bytes
	int8_t padding[4079]; // 4079 bytes
}; 

// linked list structure for FAT blocks
struct fat_blocks{
	struct fat_blocks *prev;
	struct fat_blocks *next;
	int16_t entries[2048]; // 2 bytes
};

struct root_entry{
	int8_t filename[16];
	int64_t file_size;
	int16_t first_data_idx;
	int8_t padding[10];
};

// array structure for root entries
struct root_blocks{
	struct root_entry entries[128];
};

struct disk_blocks{
	struct super_block super;
	struct fat_blocks * fat_front; //starting block
	struct fat_blocks * fat_back; //starting block
	struct root_blocks root;
};

int fs_mount(const char *diskname)
{
	if (!diskname) return -1;

	/* Open Virtual Disk*/
	// added "#include <unistd.h>" to use O_RDWR
	int disk_check = block_disk_open(diskname);
	if (!disk_check) return -1;

	/* Read the metadata (superblock, fat, root directory)*/
	// 1) Superblock - 1st 8 bytes
	struct super_block obj;
	block_read(0, &obj);
	cur_disk.super = obj;
	// block_write(0, &buf);


	// 2) FAT blocks - each block is 2048 entries, each entry is 16 bits
	for (int8_t f = 0; f < cur_disk.super.fat_blks; f++){
		struct fat_blocks cur_block;

		// what will each entry in the current fat block have
		int16_t fat_entry;
		if (f < cur_disk.super.total_data_blks)
		{
			fat_entry = 0;
		}
		else if (f == cur_disk.super.total_data_blks - 1)
		{
			fat_entry = FAT_EOC;
		}
		else 
		{
			fat_entry = f + 1;
		}

		// assign value to each fat entry in the current block
		for (int t = 0; t < 2048; t++){
			cur_block.entries[t] = fat_entry;
		}

		// assign links
		if (f == 0){
			cur_block.prev = NULL; // if first FAT block
			cur_block.next = NULL;
			cur_disk.fat_front = &cur_block;
			cur_disk.fat_back = NULL;
		} 
		else if (f == cur_disk.super.total_data_blks - 1){
			cur_block.next = NULL; // if last FAT block
			cur_disk.fat_back = &cur_block;
		} 
		else{
			struct fat_blocks * copy_back = cur_disk.fat_back;
			cur_block.prev = copy_back;
			cur_block.next = NULL;
			cur_disk.fat_back = &cur_block;
			copy_back->next = cur_disk.fat_back;
		}
	}

	// 3) Root directory - 1 block, 32-byte entry per file
	/*struct root_entry r_blocks[128];
	block_read(cur_disk.super.root_dir_idx, &r_blocks);
	cur_disk.root = r_blocks; */

	struct root_blocks r_blocks;
	block_read(cur_disk.super.root_dir_idx, &r_blocks);
	cur_disk.root = r_blocks;

	return 0;
}

int fs_umount(void)
{
	/* Close Virtual Disk */
	if (!block_disk_count()) return -1;
	if (!block_disk_close()) return -1;
	return 0;
}

int fs_info(void)
{
	if (!block_disk_count()) return -1;
	/* Show Info about Volume */
	// there should be a global class that contains the current vd info
	// we would then read from it if available, and print the info

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

