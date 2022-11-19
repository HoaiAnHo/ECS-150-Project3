#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include "disk.h"
#include "fs.h"

#define FAT_ENTRIES 2048;
#define ROOT_DIR_MAX 128;
#define FD_MAX 32;
#define FAT_EOC 0xffff;

/* Global Variables */
int fat_blk_free;  // Keeps track of free fat blocks
int rdir_blk_free; // Keeps track of free root blocks
struct disk_blocks cur_disk; // global var for fs_info
int fd_count; // count the current number of file descriptors?

/* Structs */

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
	int16_t entries[2048]; // 2 bytes
};

struct fat_entry{
	int16_t entry;
};

struct root_entry{
	int8_t filename[16];
	int32_t file_size;
	int16_t first_data_idx;
	int8_t padding[10];
};

// array structure for root entries
struct root_blocks{
	struct root_entry entries[128];
};

struct data_blocks{
	int16_t data[2048]; // 2 bytes
};

struct disk_blocks{
	struct super_block super;
	struct root_blocks root;
	struct fat_blocks *fat_blks;
	struct data_blocks *data_blks;
	struct fat_entry *fat_entries;
};

/* Helper Functions */

// helper functions for phase 1
// return how many fat entries are still free
int free_fats()
{
	int free_count = 0;
	// read through fat blocks, look at fat entries
	for (int i = 0; i < cur_disk.super.fat_blks*2048; i++)
	{
			if (cur_disk.fat_entries[i].entry == 0) //entry is assigned value
			{
				free_count++;
			}
	}
	return free_count;
}

// Function to help find the number of free spots in the root block
int free_roots()
{
	int free_count = ROOT_DIR_MAX;
	for (int i = 0; i < 128; i++)
	{
		if (cur_disk.root.entries[i].filename[0] != '\0')
		{
			free_count--;
		}
		else if (cur_disk.root.entries[i].filename[0] == '\0')
		{
			return free_count;
		}
	}
	return 0;
}

// Function to help find a free spot in the fat blocks
int find_free_fat_spot()
{
	for(int i = 0; i < cur_disk.super.fat_blks*2048; i++)
	{
		if(cur_disk.fat_entries[i].entry == 0)
			return i;
	}
	//If cannot find a free spot return -1
	return -1;
}

// Function to help find a free spot in the root block
int find_free_root_spot()
{
	int free_spot = 0;
	for (int i = 0; i < 128; i++)
	{
		if (cur_disk.root.entries[i].filename[0] == '\0')
		{
			free_spot = i;
			return free_spot;
		}
	}
	// If -1 return then there are no more free spots in the root
	return -1;
}

// helper functions for phase 2
int file_exist(const char *filename)
{
	for (int i = 0; i < 128; i++)
	{
		if (strcmp(filename, cur_disk.root.entries[i].filename) == 0) return 0;
		else if (cur_disk.root.entries[i].filename[0] == '\0') return -1;
	}
	return -1;
}

// helper functions for phase 4
int data_blk_index()
{
	// returns the index of the data block corresponding to the file's offset
	return 0;
}

void alloc_data_blk()
{
	printf("you accessed the helper function!");
	// allocate new data block and link it at the end of the data's block chain
	// allocation must follow first-fit strategy (first block availible from the beginning of the FAT)
}

/* TODO: Phase 1 - VOLUME MOUNTING */

int fs_mount(const char *diskname)
{
	if (!diskname) 
	{
		printf("Disk name was NULL\n");
		return -1;
	}
	/* Open Virtual Disk*/
	// added "#include <unistd.h>" to use O_RDWR
	int disk_check = block_disk_open(diskname);
	if (disk_check != 0)
		{
			printf("Unsuccessful disk open\n");
			return -1;
		}

	/* Read the metadata (superblock, fat, root directory)*/
	// 1) Superblock - 1st 8 bytes
	struct super_block obj;
	block_read(0, &obj);
	cur_disk.super = obj;

	// 2.2 FAT blocks - each block is 2048 entries, each entry is 16 bits
	cur_disk.fat_entries = malloc(sizeof(struct fat_entry) * cur_disk.super.fat_blks * 2048);
	for(int i = 0; i < cur_disk.super.fat_blks; i++)
	{
		block_read(1+i, &cur_disk.fat_entries[i*2048]);
	}

	// 3) Root directory - 1 block, 32-byte entry per file

	struct root_blocks r_blocks;
	block_read(cur_disk.super.root_dir_idx, &r_blocks);
	cur_disk.root = r_blocks;

	// 4) Data Blocks - from 1st data block index to the end
	cur_disk.data_blks = malloc(sizeof(struct data_blocks) * cur_disk.super.total_data_blks);
	struct data_blocks one_block_of_data;

	for(int i = cur_disk.super.data_blk_idx; i < cur_disk.super.total_data_blks; i++)
	{
		block_read(i, &cur_disk.data_blks[i]);
	}

	return 0;
}

int fs_umount(void)
{
	/* Chack if virtual disk os open */
	if (block_disk_count() == -1) return -1;

	/* May not have to do all the block writing here, prof said we could do it in parts
	when we make changes in the other functions which may be faster than doing it all
	at once at the end. However, we can do this if we, for the life of us, can't get it
	right in smaller parts */

	/*
	// Writing in current FAT blocks
	for(int i = 0; i < cur_disk.super.fat_blks; i++)
	{
		block_write(1+i, &cur_disk.fat_blks[i]);
	}

	// Writing in current Root block
	block_write(cur_disk.super.root_dir_idx, &cur_disk.root);

	//Writing in current data blocks
	for(int i = cur_disk.super.data_blk_idx; i < cur_disk.super.total_data_blks; i++)
	{
		block_write(i, &cur_disk.data_blks[i]);
	}
	*/

	//free allocated space and close disk
	free(cur_disk.fat_blks);
	free(cur_disk.data_blks);
	block_disk_close();
	return 0;
}

int fs_info(void)
{
	if (!block_disk_count()) return -1;
	/* Show Info about Volume */
	// there should be a global class that contains the current vd info
	// we would then read from it if available, and print the info
	printf("FS Info:\n");
	printf("total_blk_count=%i\n", cur_disk.super.total_blks);
	printf("fat_blk_count=%i\n", cur_disk.super.fat_blks);
	printf("rdir_blk=%i\n", cur_disk.super.root_dir_idx);
	printf("data_blk=%i\n", cur_disk.super.data_blk_idx);
	printf("data_blk_count=%i\n", cur_disk.super.total_data_blks);
	fat_blk_free = free_fats();
	rdir_blk_free = free_roots();
	printf("fat_free_ratio=%i/%i\n", fat_blk_free, cur_disk.super.total_data_blks);
	printf("rdir_free_ratio=%i/%i\n", rdir_blk_free, 128);                          

	return 0;
}

/* TODO: Phase 2 - FILE CREATION/DELETION */

int fs_create(const char *filename)
{
	/* TODO: Phase 2 */
	if (block_disk_count() == -1 || !filename) return -1;
	if (strlen(filename) > FS_FILENAME_LEN) return -1;
	// check in root directory if the filename already exists, if so return -1
	if (file_exist(filename) < 0) return -1;

	// Create New File

	//Looking for a free spot in the root
	int free_root_location = find_free_root_spot();
	if(free_root_location == -1)
	{
		printf("No More Free spots in Root");
		return 0;
	}

	//Looking for a free spot in any of the fat blocks
	int free_fat_blk; //Need to keep track of the block we found the spot in
	int free_fat_location; // The spot itself
	for(int i = 0; i < cur_disk.super.fat_blks; i++)
	{
		free_fat_location = find_free_fat_spot(cur_disk.fat_blks[i]);

		if(free_fat_location != -1) //We found a free spot
		{
			free_fat_blk = i; //Save the block we found it in
			break; //Stop the search
		}
	}

	//Calculate data block index
	int data_block_index = 2048*free_fat_blk + free_fat_location;

	//Set all information to current root entry
	*cur_disk.root.entries[free_root_location].filename = *filename;
	cur_disk.root.entries[free_root_location].file_size = 0;
	cur_disk.root.entries[free_root_location].first_data_idx = data_block_index;

	// Now we have to write this altered root block onto the virtual disk
	block_write(cur_disk.super.root_dir_idx, &cur_disk.root);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//Old Code for fs_create()

	/* Create a new file 
	// if this is the first time we're writing in a file:
	if (cur_disk.root.entries[0].filename[0] == '\0')
	{
		// specify filename
		*cur_disk.root.entries[0].filename = *filename;
		// size = 0, first index on data blocks = FAT_EOC
		cur_disk.root.entries[0].file_size = 0;
		cur_disk.root.entries[0].first_data_idx = FAT_EOC;
		// reset the other info because there's no content at this point
		/* Initially, size is 0 and pointer to 1st block is FAT_EOC 
	}
	else{
		// Find an empty entry in the root directory
		int empty = ROOT_DIR_MAX - free_roots();
		// Fill entry in root directory with proper information
		*cur_disk.root.entries[empty].filename = *filename;
		cur_disk.root.entries[empty].file_size = 0;
		cur_disk.root.entries[empty].first_data_idx = FAT_EOC;

		// QUESTION: HOW DO WE MODIFY DATA BLOCKS???
		// QUESTION: WHAT INDEX VALUES DO WE ASSIGN???
	}
	*/
	return 0;
}

int fs_delete(const char *filename)
{
	/* Delete an existing file */
	// file's entry must be emptied
	// all data blocks containing the file's contents must be freed in the FAT
	// 1) Go to root directory, find FAT entry first index from root entry
	int index = 1;
	for (int i = 0; i < 128; i++)
	{
		if (strcmp(cur_disk.root.entries[i].filename, filename) == 0)
		{
			index = cur_disk.root.entries[i].first_data_idx;
			break;
		}
	}
	// 2) for each data block in the file, free the FAT entry/data blocks
	int fat_idx = 0;
	while (index != 0xffff)
	{
		// fat_idx = index % FAT_ENTRIES;
		// index = index % cur_disk.super.fat_blks;
		// cur_disk.data_blks[cur_disk.super.data_blk_idx + index];
		// cur_disk.fat_blks->entries[index];
	}	
	// 3) free that file's root entry

	/* Free allocated data blocks, if any */
	return 0;
}

int fs_ls(void)
{
	if (block_disk_count() == -1) return -1;
	/* List all the existing files */
	printf("FS Ls:\n");
	// iterate through root directory and pull values
	for (int i = 0; i < 128; i++)
	{
		if (cur_disk.root.entries[i].filename[0] != '\0')
		{
			printf("file: %s, size: %i, data_blk: %i\n", 
			cur_disk.root.entries[i].filename, 
			cur_disk.root.entries[i].file_size, 
			cur_disk.root.entries[i].first_data_idx);
		}
		else
		{
			return 0;
		}
	}
	return 0;
}

/* TODO: Phase 3 - FILE DESCRIPTOR OPERATIONS 
none of these functions should change the file system*/

int fs_open(const char *filename)
{
	/* TODO: Phase 3 */
	/* Initialize and return file descriptor */
	// this will be used for reading/writing operations, changing the file offset, etc.

	/* 32 file descriptors max 
	- Use FD_MAX macro */

	/* Can open same filee multiple times */

	/* Contains the file's offset (initially 0) */

	return 0;
}

int fs_close(int fd)
{
	/* TODO: Phase 3 */
	/* Close file descriptor */
	// close(fd);
	return 0;
}

int fs_stat(int fd)
{
	/* TODO: Phase 3 */
	/* return file's size */
	// corresponding to the specified file descriptor
		// ex: to append to a file, call fs_lseek(fd, fs_stat(fd));
	return 0;
}

// offset = current reading/writing position in the file
int fs_lseek(int fd, size_t offset)
{
	/* TODO: Phase 3 */
	/* move file's offset */
	return 0;
}

/* TODO: Phase 4 - FILE READING/WRITING 
- THIS IS THE MOST COMPLICATED PHASE */

int fs_write(int fd, void *buf, size_t count)
{
	/* TODO: Phase 4 */
	/* Read a certain number of bytes from a file */
	return 0;
}

int fs_read(int fd, void *buf, size_t count)
{
	/* TODO: Phase 4 */
	/* Write a certain number of bytes to a file */

	/* Extend file if necessary */
	return 0;
}
