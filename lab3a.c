#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "ext2_fs.h"
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdint.h>

//hello mr.vertigo hope u are feeling better <3
//it is thursday 10pm
//some silly notes about my naming convention feel free to change to what u feel is best
//using underscores for sizes/values and camelCase for structures

char reportBuf[1024];
struct ext2_super_block sb;
struct ext2_group_desc groupDesc;

/* SUPER BLOCK VARIABLES */

int i;
int ext2_fd;
int block_size, inode_size;
int blocks_per_group, inodes_per_group;
int num_blocks, num_inodes;
int first_nonres_inode;

/* GROUP VARIABLES */

int free_blocks, free_inodes;
int block_bitmap_blk_num, inode_bitmap_blk_num, first_inode_blk_num;
int num_group;


void superblockSummary() {
	block_size = EXT2_MIN_BLOCK_SIZE << sb.s_log_block_size; 
	inode_size = sb.s_inode_size;
	num_blocks = sb.s_blocks_count;
	num_inodes = sb.s_inodes_count;
	blocks_per_group = sb.s_blocks_per_group;
	inodes_per_group = sb.s_inodes_per_group;
	first_nonres_inode = sb.s_first_ino;
	
	pread(ext2_fd, &sb, sizeof(sb), 1024);
	sprintf(reportBuf, "%s,%d,%d,%d,%d,%d,%d,%d", "SUPERBLOCK", num_blocks, num_inodes, 
			block_size, inode_size, blocks_per_group, inodes_per_group, first_nonres_inode);
	
	printf("%s\n", reportBuf);
}

void groupSummary() {
	num_group = 0;
	//blocks_per_group also included here
	//inodes_per_group also included here
	free_blocks = groupDesc.bg_free_blocks_count;
	free_inodes = groupDesc.bg_free_inodes_count;	
	block_bitmap_blk_num = groupDesc.bg_block_bitmap;
	inode_bitmap_blk_num = groupDesc.bg_inode_bitmap;	
        first_inode_blk_num = groupDesc.bg_inode_table;	

	pread(ext2_fd, &groupDesc, sizeof(groupDesc), 1024+block_size);
	sprintf(reportBuf, "%s,%d,%d,%d,%d,%d,%d,%d,%d", "GROUP", num_group, blocks_per_group, 
			inodes_per_group, free_blocks, free_inodes, block_bitmap_blk_num, inode_bitmap_blk_num,
			first_inode_blk_num);

	if (sb.s_magic != EXT2_SUPER_MAGIC)
	    exit(1); 	

	printf("%s\n", reportBuf);
}

int main(int argc, char** argv) {
	//-------------handle input argument----------------------
	if (argc < 2 || strstr(argv[1], ".img")==NULL) {
		printf("Not a valid filesystem\n");
		exit(1);
	}
	
	ext2_fd = open("EXT2_test.img", O_RDONLY);   

	//------------get summaries------------------------------
	superblockSummary();
	groupSummary();
}
