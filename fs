//CSE3320-002
//Thanksgiving File System
//Code taken from video supplied by professor Trevor Bakker

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <unistd.h>


#define NUM_BLOCKS 4226
#define BLOCK_SIZE 8192
#define NUM_FILES 128
#define NUM_INODES 128
#define MAX_BLOCKS_PER_FILE 32

unsigned char data_blocks[NUM_BLOCKS][BLOCK_SIZE];
int used_blocks[NUM_BLOCKS];

struct directory_entry{
	char* name;
	int valid;
	int inode_idx;
};

struct directory_entry* directory_ptr;

struct inode{
	time_t date;
	int valid;
	int size;
	int blocks[MAX_BLOCKS_PER_FILE];
};

struct inode* inode_array_ptr[NUM_INODES];

//**********************************************************************
//This function is called when user types "df"
//This function is used in the put() as well
int df()
{
	int count = 0;
	int i = 0;
	
	for(i = 130; i < 4226; i++)
	{
		if(used_blocks[i] == 0)
		{
			count++;
		}
	}
	
	return count * BLOCK_SIZE;
}

//**********************************************************************	
int findFreeInodeBlockEntry(int inode_index)//need to write smarter version (52:32)
{
	int i;
	int retval = -1;
	for(i = 0; i < 32; i++)
	{
		if(inode_array_ptr[inode_index]->blocks[i] == -1)
		{
			retval = i;
			break;
		}
		
	}
	return retval;

}
//**********************************************************************


void init()
{
	int i; 

	directory_ptr = (struct directory_entry*) &data_blocks[0];
	
	for(i = 0; i < NUM_FILES; i++)
	{
		directory_ptr[i].valid = 0;
	}
	
	int inode_idx = 0;
	for(i = 1; i < 130; i++)
	{
		inode_array_ptr[inode_idx++] = (struct inode*) &data_blocks[i];
	}
}

//**********************************************************************
int findFreeDirectoryEntry()
{
	int i;
	int retval = -1;
	for(i = 0; i < 128; i++)
	{
		if(directory_ptr[i].valid == 0)
		{
			retval = i;
			break;
		}
	}
	return retval;
}

//**********************************************************************
int findFreeInode()
{
	int i;
	 int retval = -1; 
	 for(i = 0; i < 128; i++)
	 {
		 if(inode_array_ptr[i]->valid ==0)
		 {
			 retval = i;
			 break;
		 }
	 }
	 
	 return retval;
}
//**********************************************************************
int findFreeBlock()
{
	
	int retval = -1;
	int i = 0;
	for(i = 130; i < 4226; i++)
	{
		if(used_blocks[i] == 0)
		{
			retval = i;
			break;
		}
		
	}
	
	return retval;
	
}

//**********************************************************************
void put(char* filename)
{
	//put() starts here
	
	struct stat buf;
	
	int status = stat(filename, &buf);
	if(status == -1)
	{
		printf("Error: File not found \n");
		return;
	}

	if(buf.st_size > df())
	{
		printf("Error: Not enough room in the file system\n");
		return;
	}

	int dir_idx = findFreeDirectoryEntry();
	if(dir_idx == -1)
	{
		printf("Error: Not enough room in the file system\n");
		return;
	}

	
	directory_ptr[dir_idx].valid = 1;//we are using it; marked as being used
	
	directory_ptr[dir_idx].name = (char*) malloc(strlen(filename));
	
	strncpy(directory_ptr[dir_idx].name, filename, strlen(filename));


	int inode_idx = findFreeInode();
	
	if(inode_idx == -1)
	{
		printf("Error: No free inodes\n");
		return;
	}
	
	directory_ptr[dir_idx].inode_idx = inode_idx;
	
	
	
	inode_array_ptr[inode_idx]->size = buf.st_size;
	inode_array_ptr[inode_idx]->date = time(NULL);
	inode_array_ptr[inode_idx]->valid = 1;
	//Open the input file read-only
	FILE* ifp = fopen(filename, "r");
	
	
	int copy_size = buf.st_size;//set copy_size to be size of the file
	int offset = 0;//the first time we want to read those bytes
	
	
	int block_index = findFreeBlock();
	
	if(block_index == -1)
	{
		printf("Error: Can't find free block\n");
		//Cleanup a bunch of directory and inode stuff (optional)
		return;
	}
	
	while(copy_size >= BLOCK_SIZE)
	{
		int block_index = findFreeBlock();
		
		if(block_index == -1)
		{
			printf("Error: Can't find free block\n");
			//cleanup a bunch of directory and inode stuff
			return;
		}
		
		used_blocks[block_index] = 1;
		
		int inode_block_entry = findFreeInodeBlockEntry(inode_idx);
		if(inode_block_entry == -1)
		{
			printf("Error: Can't find free node block\n");
			//cleanup a bunch of directory and inode stuff
			return;
		}
		inode_array_ptr[inode_idx]->blocks[inode_block_entry] = block_index;
		fseek(ifp, offset, SEEK_SET);
		
		int bytes = fread(data_blocks[block_index], BLOCK_SIZE, 1, ifp);//number of bytes read; double check
		
		if(bytes == 0 && !feof(ifp))
		{
			printf("An error occured reading from the input file.\n");
			return;
		};
		
		clearerr(ifp);
		
		copy_size -= BLOCK_SIZE;
		offset += BLOCK_SIZE;
		
	}
	
	if(copy_size > 0)
	{
		int block_index = findFreeBlock();
		
		if(block_index == -1)
		{
			printf("Error: Can't find free block\n");
			//Cleanup a bunch of directory and inode stuff
			return;
		}
		
		int inode_block_entry = findFreeInodeBlockEntry(inode_idx);
		if(inode_block_entry == -1)
		{
			printf("Error: Can't find free node block\n");
			//Cleanup a bunch of directory and inode stuff
			return;
		}
		
		inode_array_ptr[inode_idx]->blocks[inode_block_entry] = block_index;
		
		used_blocks[block_index] = 1;
		
		//handle remainder (41 min 19 sec into video)
		fseek(ifp, offset, SEEK_SET);
		int bytes = fread(data_blocks[block_index], copy_size, 1, ifp);
		printf("main:if copy_size > 0: bytes = %d\n", bytes);
		
	}
	
	fclose(ifp);
	
	return;
}
	
	
	
//**********************************************************************
//**********************************************************************
//**********************************************************************
//**********************************************************************
int main()
{
	init();
	
	char* filename1 = "firstfile.txt";
	char* filename2 = "secondfile.txt";
	
	//Lets find a spot for thee first file
	int dir_idx = findFreeDirectoryEntry();
	printf("Found a free directory entry at %d for file %s\n", dir_idx, filename1);

	//Now set this directory entry as used so we don't use it twice
	directory_ptr[dir_idx].valid = 1;

	//Allocate room for the filename
	directory_ptr[dir_idx].name = (char*)malloc(strlen(filename1));

	//Copy the filename
	memcpy(directory_ptr[dir_idx].name, filename1, strlen(filename1));

	//Lets find a spot for the second file
	dir_idx = findFreeDirectoryEntry();

	//Now set this directory entry as used so we don't use it twice
	directory_ptr[dir_idx].valid = 1;

	//Allocate room for the filename
	directory_ptr[dir_idx].name = (char*)malloc(strlen(filename2));

	//Copy the filename
	memcpy(directory_ptr[dir_idx].name, filename2, strlen(filename2));
	printf("Found a free directory entry at %d for file %s\n", dir_idx, filename2);

	printf("Disk free; %d bytes\n", df());

	//Now iterate over the directory structure to make sure it is good
	//This is similar to the list() function you need to write

	int idx = 0;
	for(idx = 0; idx < 128; idx++)
	{
		if(directory_ptr[idx].valid == 1)
		{
			printf("File: %s\n", directory_ptr[idx].name);
		}
	}
	
	return 0;


		
}
