#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "ext2_fs.h"
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

int ext2_fd;
int block_size;
char reportBuf[1024];
struct ext2_super_block sb;

void superblockSummary() {
	block_size = EXT2_MIN_BLOCK_SIZE << sb.s_log_block_size; 
	
	sprintf(reportBuf, "%s,%d,%d,%d,%d,%d,%d,%d", "SUPERBLOCK", sb.s_blocks_count, sb.s_inodes_count, 
			block_size, sb.s_inode_size, sb.s_blocks_per_group, sb.s_inodes_per_group, sb.s_first_ino);
	
	printf("%s\n", reportBuf);
}

int main(int argc, char** argv) {

	
	//-------------handle input argument----------------------
	if (argc < 2 || strstr(argv[1], ".img")==NULL) {
		printf("Not a valid filesystem\n");
		exit(1);
	}
	

	ext2_fd = open("EXT2_test.img", O_RDONLY);   

	pread(ext2_fd, &sb, sizeof(sb), 1024);
	if (sb.s_magic != EXT2_SUPER_MAGIC)
	    exit(1); 

	//------------get summaries------------------------------
	superblockSummary();
	
}
