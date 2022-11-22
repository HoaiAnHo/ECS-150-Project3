#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include "disk.h"
#include "fs.h"

/* Useful macros*/
#define FAT_ENTRIES 2048;
//#define FAT_EOC 0xffff;

/* Structs */

// very first block of the disk, contains information about filesystem
struct super_block{
	uint64_t signature;  // 8 bytes, signature must be equal to "ECS150FS"
	uint16_t total_blks; // 2 bytes
	uint16_t root_dir_idx;  // 2 bytes
	uint16_t data_blk_idx; // 2 bytes
	uint16_t total_data_blks; // 2 bytes
	uint8_t fat_blks; // 1 bytes
	uint8_t padding[4079]; // 4079 bytes
}; 

// linked list structure for FAT blocks
struct fat_blocks{
	uint16_t entries[2048]; // 2 bytes
};

struct fat_entry{
	uint16_t entry;
};

struct root_entry{
	char filename[16];
	uint32_t file_size;
	uint16_t first_data_idx;
	int8_t padding[10];
};

// array structure for root entries
struct root_blocks{
	struct root_entry entries[FS_FILE_MAX_COUNT];
};

struct data_blocks{
	int8_t data[4048]; // 1 byte
};

struct disk_blocks{
	struct super_block super;
	struct root_blocks root;
	struct fat_blocks *fat_blks;
	struct data_blocks *data_blks;
	struct fat_entry *fat_entries;
};

struct file_descriptor{
	int offset;
	int status; //0 is open, 1 is closed
	char *filename;
};

// Global Variables
int fat_blk_free;  // Keeps track of free fat blocks
int rdir_blk_free; // Keeps track of free root blocks
struct disk_blocks cur_disk; // global var for fs_info
int fd_count; // count the current number of file descriptors?
struct file_descriptor file_desc[FS_OPEN_MAX_COUNT]; // keep all fds here

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
	int free_count = FS_FILE_MAX_COUNT;
	for (int i = 0; i < FS_FILE_MAX_COUNT; i++)
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
	for (int i = 0; i < FS_FILE_MAX_COUNT; i++)
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
		if (strcmp(filename, cur_disk.root.entries[i].filename) == 0) return 1; //found
		// else if (cur_disk.root.entries[i].filename[0] == '\0') return -1;
	}
	return 0; //not found
}

// helper functions for phase 3

// helper functions for phase 4
int data_blk_index()
{
	// returns the index of the data block corresponding to the file's offset
	return 0;
}

int alloc_data_blk()
{
	printf("you accessed the helper function!");
	// allocate new data block and link it at the end of the data's block chain
	// allocation must follow first-fit strategy (first block availible from the beginning of the FAT)
	return 0;
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
	if (block_disk_count() == -1 || !filename)
	{
		printf("No disk mounted \n");
		return -1;
	}
	if (strlen(filename) > FS_FILENAME_LEN)
	{
		printf("Name too long \n");
		return -1;
	}
	// check in root directory if the filename already exists, if so return -1
	if (file_exist(filename) == 1)
	{
		printf("File already exists \n");
		return -1;
	}

	// Create New File

	//Looking for a free spot in the root
	int free_root_location = find_free_root_spot();
	if(free_root_location == -1)
	{
		printf("No More Free spots in Root");
		return 0;
	}

	//Looking for a free spot in any of the fat blocks
	int free_fat_location = find_free_fat_spot();

	if(free_fat_location == -1)
	{
		printf("No more free locations in FAT");
		return 0;
	}

	//Calculate data block index
	int data_block_index = free_fat_location;

	//Set all information to current root entry
	strcpy(cur_disk.root.entries[free_root_location].filename, filename);
	cur_disk.root.entries[free_root_location].file_size = 0;
	cur_disk.root.entries[free_root_location].first_data_idx = data_block_index;

	// Now we have to write this altered root block onto the virtual disk
	block_write(cur_disk.super.root_dir_idx, &cur_disk.root);

	return 0;
}

int fs_delete(const char *filename)
{
	printf("fs_delete called \n");
	/* Delete an existing file */
	// file's entry must be emptied
	// all data blocks containing the file's contents must be freed in the FAT
	// 1) Go to root directory, find FAT entry first index from root entry
	int first_FAT = 1;
	for (int i = 0; i < 128; i++)
	{
		if (strcmp(cur_disk.root.entries[i].filename, filename) == 0)
		{
			first_FAT = cur_disk.root.entries[i].first_data_idx;

			// 2) free that file's root entry
			strcpy(cur_disk.root.entries[i].filename, "\0");
			cur_disk.root.entries[i].file_size = 0;
			cur_disk.root.entries[i].first_data_idx = 0;
			// not sure what else to do about padding
			break;
		}
	}
	printf("Got passed emptying root entry \n");
	// 3) for each data block in the file, free the FAT entry/data blocks
	int current_FAT = first_FAT;

	int next_idx = 1;
	while (cur_disk.fat_entries[current_FAT].entry != 0xffff)
	{
		//free(cur_disk.data_blks[index]); ???
		next_idx = cur_disk.fat_entries[current_FAT].entry;
		cur_disk.fat_entries[current_FAT].entry = 0;
		current_FAT = next_idx;
		// cur_disk.data_blks[cur_disk.super.data_blk_idx + index];
		// cur_disk.fat_blks->entries[index];
	}	
	printf("Got passed emptying fat \n");
	/* Free allocated data blocks, if any */
	block_write(cur_disk.super.root_dir_idx, &cur_disk.root);
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
	}
	return 0;
}

/* TODO: Phase 3 - FILE DESCRIPTOR OPERATIONS 
none of these functions should change the file system*/

int fs_open(const char *filename)
{
	/* Initialize and return file descriptor */
	// this will be used for reading/writing operations, changing the file offset, etc.
	/* Can open same file multiple times */
	/* Contains the file's offset (initially 0) */

	if (block_disk_count() == -1 || fd_count == FS_OPEN_MAX_COUNT) return -1;
	if (!filename || file_exist(filename) == -1) return -1;

	for (int i = 0; i < FS_OPEN_MAX_COUNT; i++)
	{
		if (!file_desc[i].status)
		{
			for (int j = 0; j < 128; j++)
			{
				if (strcmp(cur_disk.root.entries[j].filename, filename) == 0)
				{
					strcpy(file_desc[i].filename, filename);
					file_desc[i].offset = 0;
					fd_count++;
					return i;
				}
			}
		}
	}
	printf("something's wrong with the fs open implementation");
	return 0;
}

int fs_close(int fd)
{
	/* Close file descriptor */
	file_desc[fd].status = 0;
	fd_count--;
	return 0;
}

int fs_stat(int fd)
{
	/* return file's size */
	int offset = file_desc[fd].offset;
	// corresponding to the specified file descriptor
		// ex: to append to a file, call fs_lseek(fd, fs_stat(fd));
	return offset;
}

// offset = current reading/writing position in the file
int fs_lseek(int fd, size_t offset)
{
	/* move file's offset */
	file_desc[fd].offset = offset;
	return file_desc[fd].offset;
}

/* TODO: Phase 4 - FILE READING/WRITING 
- THIS IS THE MOST COMPLICATED PHASE */
// reading from a file contained in the data blocks, write from those data blocks into the file

// buf contains data, write onto data blocks (depending on where offset is)
int fs_write(int fd, void *buf, size_t count)
{
	/* TODO: Phase 4 */
	/* Read a certain number of bytes from a file */
	return 0;
}

// buffer gets data here
int fs_read(int fd, void *buf, size_t count)
{
	/* TODO: Phase 4 */
	/* Write a certain number of bytes to a file */

	/* Extend file if necessary */
	return 0;
}
