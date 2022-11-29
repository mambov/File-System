/*
 
  Name: Michael McKinley
  ID: 1000255408
  
  Name: Diego Vester
  ID: 1001329342
 
 */

/*
CSE3320-002
Fall 2022
Programming Assignment 4: File System
Assigned: November 14, 2022
Due: Wednesday, November 30, 2022 by 5:30 PM CST
*/


// The MIT License (MIT)
// 
// Copyright (c) 2016 Trevor Bakker 
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <sys/stat.h>

#define WHITESPACE " \t\n"      // We want to split our command line up into tokens
                                // so we need to define what delimits our tokens.
                                // In this case  white space
                                // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 32    // The maximum command-line size

#define MAX_NUM_ARGUMENTS 4     // Mav shell only supports four arguments


//The total memory size is 4226*8192 = 34619392 = ~33 MB
#define NUM_BLOCKS 4226
#define BLOCK_SIZE 8192
#define NUM_FILES 125
#define NUM_INODES 125
#define MAX_BLOCKS_PER_FILE 32

unsigned char data_blocks[NUM_BLOCKS][BLOCK_SIZE];
int used_blocks[NUM_BLOCKS];

//......................................................................
struct directory_entry{
	char* name;
	int valid;
	int inode_idx;
};

//......................................................................

struct directory_entry* directory_ptr;//pointer to directory_entry struct

//......................................................................

struct inode{
	time_t date;
	int valid;
	int size;
	int blocks[MAX_BLOCKS_PER_FILE];
};

//......................................................................

struct inode* inode_array_ptr[NUM_INODES];


//**********************************************************************

void listImageFiles()
{
    int i = 0;
    for(i = 0; i < 127; i++)
    {
        
        
    }
    
    return;
    
}

//**********************************************************************
//This function is called when user types "df"
//This function is used in the put() as well
int df()
{
	int count = 0;
	int i = 0;
	
	for(i = 126; i < 4226; i++)
	{
		if(used_blocks[i] == 0)
		{
			count++;
		}
	}
	printf("df:count = %d\n", count);
    printf("BLOCK_SIZE = %d\n", BLOCK_SIZE);
	return count*BLOCK_SIZE;
	
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
	for(i = 1; i < 126; i++)
	{
		inode_array_ptr[inode_idx++] = (struct inode*) &data_blocks[i];
	}
}

//**********************************************************************
int findFreeDirectoryEntry()//Find inode that is free
{
    printf("Beginning findFreeDirectoryEntry()\n");
	int i;
	int retval = -1;
    printf("findFreeDirectoryEntry: retval = %d\n", retval);
    
	for(i = 0; i < 125; i++)
	{
		if(directory_ptr[i].valid == 0)//inode is not being used
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
	 for(i = 0; i < 125; i++)
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
	for(i = 126; i < 4226; i++)
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
	printf("Beginning put()\n");
	struct stat buf;
    
	//..................................................................
    printf("put(): Error: File not found\n");
	int status = stat(filename, &buf);
	if(status == -1)
	{
		printf("Error: File not found \n");
		return;
	}
    
	//..................................................................
    printf("put(): Error: Not enough room in the file system (1)\n");
    if(buf.st_size > df())
    {
        printf("Error: Not enough room in the file system (1) \n");
        return;
    }
    
	//..................................................................
    printf("put(): Just before int dir_idx = findFreeDirectoryEntry()\n");
    int dir_idx = findFreeDirectoryEntry();
    printf("put(): Just before: Error: Not enough room in the file system (2)\n");
    if(dir_idx == -1)
    {
        printf("Error: Not enough room in the file system (2)\n");
        return;
    }
    
	//..................................................................
	printf("put(): after first three Error checks: just before directory_ptr[] statements\n");
	directory_ptr[dir_idx].valid = 1;//we are using it; marked as being used
    printf("put(): directory_ptr[dir_idx].valid = %d\n", directory_ptr[dir_idx].valid);
	
	directory_ptr[dir_idx].name = (char*) malloc(strlen(filename));
	printf("put(): directory_ptr[dir_idx].name = %s\n", directory_ptr[dir_idx].name);
    
	strncpy(directory_ptr[dir_idx].name, filename, strlen(filename));
    printf("put(): directory_ptr[dir_idx].name = %s\n", directory_ptr[dir_idx].name);

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
    printf("put(): open %s for reading\n", filename);
	FILE* ifp = fopen(filename, "r");
	
	
	int copy_size = buf.st_size;//set copy_size to be size of the file
	int offset = 0;//the first time we want to read those bytes
	
	//..................................................................
/*		
	int block_index = findFreeBlock();

	if(block_index == -1)
	{
		printf("Error: Can't find free block\n");
		//Cleanup a bunch of directory and inode stuff (optional)
		return;
	}
*/
	//..................................................................
    
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
//**********************************************************************	
//**********************************************************************
//**********************************************************************
//**********************************************************************
//**********************************************************************
//**********************************************************************
int main(int argc, char* argv[])
{
	init();
	time_t file_time;
	file_time = time(NULL);
	
	printf("Time as time_t (time in seconds from January 1, 1970): %ld\n", file_time);
	printf("Time as string; %s\n", ctime(&file_time));
	
	printf("%d %s %s\n", 65535, ctime(&file_time), "file.txt");
    
    //This is pointer to where the command line input string is stored
  	char* command_string = (char*) malloc( MAX_COMMAND_SIZE );
  
    
	while( 1 )//continuous input in the bash shell loop
	{
		// Print out the msh prompt
		printf ("mfs>");//print the bash shell prompt

		
		char* fgetsreturn = fgets (command_string, MAX_COMMAND_SIZE, stdin);

		while( !fgetsreturn );
		//..................................................................
		//REQUIREMENT 6
		//Check for blank line input (Requirement 6)
		if(command_string[0] == '\n') 
		{
			continue;
		}

		
		
		
		//..................................................................
		//Separate input into tokens
		/* Parse input */
		char* token[MAX_NUM_ARGUMENTS];

		int   token_count = 0;                                 
															
		// Pointer to point to the token
		// parsed by strsep
		char* argument_ptr;                                         
															
		char* working_string  = strdup( command_string );  
			
		// we are going to move the working_string pointer so
		// keep track of its original value so we can deallocate
		// the correct amount at the end
		char* head_ptr = working_string;

		// Tokenize the input strings with whitespace used as the delimiter
		while ( ( (argument_ptr = strsep(&working_string, WHITESPACE ) ) != NULL) && 
				(token_count<MAX_NUM_ARGUMENTS))
		{
		token[token_count] = strndup( argument_ptr, MAX_COMMAND_SIZE );
		if( strlen( token[token_count] ) == 0 )
		{
			token[token_count] = NULL;
		}
			token_count++;
		}
		
		//..................................................................
		//Now print the tokenized input as a debug check
		//TODO Remove this code and replace with your shell functionality

			//..................................................................
		//Requirement 5
		//Check for exit commands "quit" and "exit" 
		
		if(strstr(token[0], "exit")!=NULL) 
			return  0;
		
		//if(strcmp("exit", token[0])==0) 
		if(strstr(token[0], "quit")!=NULL)   
			return  0;
		
		//..................................................................

		//Get current working directory
		char cwd[256];
		getcwd(cwd, sizeof(cwd));

		//..................................................................
		//execute command "df"

		if((strstr(token[0], "df")!=NULL))//call df()
		{

			int df_return = df();
			printf("%d bytes free.\n", df_return);//df_return = count*BLOCK_SIZE

		}
			
		//..................................................................   

		//execute command "put"

		if((strstr(token[0], "put")!=NULL))//call put()
		{
			//char* filename;
			printf("Call function put()\n");
			printf("Local file name token[1]= %s\n", token[1]);
			put(token[1]);


		}
			
		//..................................................................   
		
		
		free( head_ptr );

	}//while(1) loop
  
  return 0;
  
  // e2520ca2-76f3-11ec-90d6-0242ac120003
}
