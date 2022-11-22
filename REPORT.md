# ECS-150-Project3

# Phase 1: Volume Mounting
For this phase, we worked on three main functions, implemented _ helper
functions, and created _ data structures to keep track of the volume structures.

The data structures kept track of the superblock, FAT blocks with their entries,
the root directory with their entries, and the current state of the data blocks.
When we first run fs_umount, we use the block reading function given to us by
disk.h to read those blocks into the data structures. We made sure to specify in
those data structures what size the members were, so the whole block that was 
being read into the structure fit perfectly in size.

The helper functions also helped to keep track of the amount of free entries for
FAT and the root directory were availible. While this is helpful for later phase
functions in terms of storage checking, we originally wrote those functions for
volume info listing in fs_info.

# Phase 2: File Creation/Deletion
File creation involves logging the new information into the root directory, then
allocating space in the FAT entries and the data blocks.

File deletion was the most complicated for us, with having to free up all the
space involved.

Listing all the files availible was the most simple of these operations, as we 
simply had to iterate through the availible root directory entries and print
out a particular file's information in said entry.

# Phase 3: File Descriptor Operations
For file descriptors, we created a data structure for open files that would
contain members like filename and offset. Since we can open multiple file 
descriptors of the same file ___.

We initialized an array of the file descriptor data structures, with 32 slots
specifically as the limit to the amount of possible open fds. Upon the first
initialization, every index in the array contains NULL, which designates the
fd entry as empty and unopened. The value in fd that we return in fs_open, is 
the index number where a particular file descriptor we opened is located in
the array. If the user returns a certain fd in later functions like fs_seek, 
we use fd to index into the assigned location in the array, and grab whatever
member values are necessary for the operation. Running fs_close will replace
the given fd index with NULL, which deems it "closed"

# Phase 4: Reading/Writing Files
We used a bounce buffer as described in class.

# Testing and Issues Found
