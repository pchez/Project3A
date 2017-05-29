#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "ext2_fs.h"
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdint.h>
#include <time.h>

//hello mr.vertigo hope u are feeling better <3
//it is thursday 10pm
//some silly notes about my naming convention feel free to change to what u feel is best
//using underscores for sizes/values and camelCase for structures

char reportBuf[1024];
char output[30];  // output buffer for time! 
struct ext2_super_block sb;
struct ext2_group_desc groupDesc;
struct ext2_inode inode;

/* SUPER BLOCK VARIABLES */

int i;
int ext2_fd;
__u32 block_size; 
__u16 inode_size;
__u32 blocks_per_group, inodes_per_group;
__u32 num_blocks, num_inodes;
__u32 first_nonres_inode;

/* GROUP VARIABLES */

__u16 free_blocks, free_inodes;
__u32 block_bitmap_blk_num, inode_bitmap_blk_num, first_inode_blk_num;
int num_group;

/* BLOCK AND INODE BITMAP VARIABLES */

char byte_buffer;
int bit_mask, j;

/* INODE VARIABLES */

int inode_num; 
char inode_file_type;
__u16 inode_file_mode;
__u16 inode_file_owner, inode_file_group;
__u16 inode_link_count; 
char * inode_creation_time; 
char * inode_modification_time;
char * inode_last_access_time; 
__u32 inode_file_size, inode_num_blocks;

/* ------------------ START OF FUNCTIONS ---------------------------------- */

void superblockSummary() {
	pread(ext2_fd, &sb, sizeof(sb), 1024);    
	block_size = EXT2_MIN_BLOCK_SIZE << sb.s_log_block_size; 
	inode_size = sb.s_inode_size;
	num_blocks = sb.s_blocks_count;
	num_inodes = sb.s_inodes_count;
	blocks_per_group = sb.s_blocks_per_group;
	inodes_per_group = sb.s_inodes_per_group;
	first_nonres_inode = sb.s_first_ino;
	 
	sprintf(reportBuf, "%s,%d,%d,%d,%d,%d,%d,%d", "SUPERBLOCK", num_blocks, num_inodes, 
			block_size, inode_size, blocks_per_group, inodes_per_group, first_nonres_inode);
	
	if (sb.s_magic != EXT2_SUPER_MAGIC) {
	 	fprintf(stderr, "MAGIC NUMBER DOES NOT WORK! Exiting...\n");
		exit(1); 		
	}

	printf("%s\n", reportBuf);
}


void groupSummary() {
	pread(ext2_fd, &groupDesc, sizeof(groupDesc), 1024+block_size);
	num_group = 0;
	//num_blocks also included here
	//num_inodes also included here
	free_blocks = groupDesc.bg_free_blocks_count;
	free_inodes = groupDesc.bg_free_inodes_count;	
	block_bitmap_blk_num = groupDesc.bg_block_bitmap;
	inode_bitmap_blk_num = groupDesc.bg_inode_bitmap;	
        first_inode_blk_num = groupDesc.bg_inode_table;	

	sprintf(reportBuf, "%s,%d,%d,%d,%d,%d,%d,%d,%d", "GROUP", num_group, num_blocks, 
			num_inodes, free_blocks, free_inodes, block_bitmap_blk_num, inode_bitmap_blk_num,
			first_inode_blk_num);

	
	printf("%s\n", reportBuf);
}


void checkFreeBlocks() {
	memset(reportBuf, '\0', sizeof(char)*1024); // clear the array
	//int count_check = 0;  // for de-bugging/sanity check purposes..
	for(i = 0; i < block_size; i++) {
		bit_mask = 1;
		pread(ext2_fd, &byte_buffer, sizeof(byte_buffer), 1024 + block_size*2 + i);  // read in the byte
		//  start at 3*1024 because boot table comes first, then super block, then group desciptor table, then bitmap blocks! :)
		
		for(j = 0; j < 8; j++) {
			int check = byte_buffer & bit_mask;
			if(check == 0) { 
				sprintf(reportBuf, "%s,%d", "BFREE", (i * 8 + j + 1));
				printf("%s\n", reportBuf); 
				//count_check++;
			} 		
			bit_mask <<= 1;
		}
	} // end of block for loop
	//if(count_check == free_blocks)
		//fprintf(stdout, "NUMBER OF FREE BLOCKS IS CORRECT\n");
}


void checkFreeInodes() {
	//int count_check = 0;   // for de-bugging and sanity purposes...
	for(i = 0; i < block_size; i++) {
		bit_mask = 1;
		pread(ext2_fd, &byte_buffer, sizeof(byte_buffer), 1024 + block_size*3 + i);

		for(j = 0; j < 8; j++) {
			int check = byte_buffer & bit_mask;
			if(check == 0) {
				sprintf(reportBuf, "%s,%d", "IFREE", (i * 8 + j + 1));
				printf("%s\n", reportBuf);
				//count_check++;
			}
			bit_mask <<= 1;
		}
	} // end of inode for loop
	//if(count_check == free_inodes)
		//fprintf(stdout, "NUMBER OF FREE INODES IS CORRECT\n");
}


int convertIntToOctal(__u16 mode) {
	return 511&mode;
}


char getFileType(int i_mode) {  // Helper function to get the char for file type for inode summary
	if((0x4000 & i_mode) == 0x4000)  // directory
		return 'd';
	else if((0x8000 & i_mode) == 0x8000)  // file
		return 'f';
	else if((0xA000 & i_mode) == 0xA000)  // symbolic link
		return 's';
	else 
		return '?'; // something else
}


char * convertToTime(__u32 timeIn) {  // Helper function to turn int time to string for inode summary
	time_t time = timeIn;
	struct tm * timeinfo = localtime(&time);
	snprintf(output, 30, "%02d/%02d/%02d %02d:%02d:%02d", timeinfo->tm_mon+1, timeinfo->tm_mday, timeinfo->tm_year%100,
			timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
	return output;  	
}


void inodeSummary() {
	for(i = 5*block_size; i < 5*block_size + num_inodes*sizeof(inode); i = i + sizeof(inode)) {
		pread(ext2_fd, &inode, sizeof(inode), i);
		inode_num = ((i - (5*block_size))/sizeof(inode)) + 1;
		inode_file_mode = convertIntToOctal(inode.i_mode);  // TODO: CHANGE INT TO OCTAL
		inode_file_type = getFileType(inode.i_mode);  
		inode_file_owner = inode.i_uid;
		inode_file_group = inode.i_gid;
		inode_link_count = inode.i_links_count;
		inode_creation_time = convertToTime(inode.i_ctime);
		inode_modification_time = convertToTime(inode.i_mtime);
		inode_last_access_time = convertToTime(inode.i_atime);
		inode_file_size = inode.i_size;
		inode_num_blocks = inode.i_blocks;
		
		if(inode_file_mode != 0 && inode_link_count > 0) {
			sprintf(reportBuf, "%s,%d,%c,%o,%d,%d,%d,%s,%s,%s,%d,%d", "INODE", inode_num, inode_file_type, inode_file_mode,
				inode_file_owner, inode_file_group, inode_link_count, inode_creation_time, inode_modification_time,
				inode_last_access_time, inode_file_size, inode_num_blocks);
		
			printf("%s\n", reportBuf);
		}
	}
}


int main(int argc, char** argv) {
	//-------------handle input argument----------------------
	if (argc < 2 || strstr(argv[1], ".img")==NULL) {
		printf("Not a valid filesystem\n");
		exit(1);
	}
	
	ext2_fd = open(argv[1], O_RDONLY);   	

	//------------get summaries------------------------------
	superblockSummary();
	groupSummary();
	checkFreeBlocks();  // parse bitmap for blocks to check for BFREE blocks to print out
	checkFreeInodes();  // parse bitmap for inodes to check for IFREE inodes to print out
	inodeSummary();
}
