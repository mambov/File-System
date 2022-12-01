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

#define MAX_COMMAND_SIZE 255    // The maximum command-line size

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

void init()
{
	printf("\nBeginning init()\n");
	
	//..............................................................
	int i; 

	directory_ptr = (struct directory_entry*) &data_blocks[0];
	
	for(i = 0; i < NUM_FILES; i++)
	{
		directory_ptr[i].name = NULL;
		directory_ptr[i].valid = 0;
		directory_ptr[i].inode_idx = 0;
		
	//char* name;
	//int valid;
	//int inode_idx;
		
		
		
	}
	
	//..............................................................
	int inode_idx = 1;
	//for(i = 5; i <= 130; i++)
	for(i = 1; i < 126; i++)
	{
		inode_array_ptr[inode_idx++] = (struct inode*) &data_blocks[i];
		printf("init: inside second for loop: i = %d\n", i);
		//printf("init: inode_array_ptr[%d] = %p", inode_idx++, inode_array_ptr[inode_idx++]);
	}
	//..............................................................
	printf("\nEnding init()\n\n");
}

//**********************************************************************

void open(char* filename)
{
    printf("\nBeginning open()\n");
    init();
  
  //Open the input file read-only
	printf("open(): open %s for reading\n", filename);
	FILE* ifp = fopen(filename, "r");
	printf("open: after FILE* ifp = fopen(): ifp = %p\n", ifp);
	
	//fread(&data_blocks[0], 8192*4226, 1, ifp);
	fread(&data_blocks[0], 8192*4096, 1, ifp);
	
    printf("\nEnding open()\n\n");
    return;
    
}



//**********************************************************************

void listImageFiles()
{
    //printf("\nBeginning listImageFiles()\n");
    int i = 0;
    //for(i = 0; i < 127; i++)
    
    	time_t file_time;
	
	file_time = time(NULL);
	
	printf("Time as time_t (time in seconds from January 1, 1970): %ld\n", file_time);
	printf("Time as string; %s\n", ctime(&file_time));
	
	//printf("%d %s %s\n", 65535, ctime(&file_time), "file.txt");
     
    
	for(i = 0; i < 125; i++)
	{
		if(directory_ptr[i].valid != 0)//inode is not being used
		{
		      printf("listImageFiles: inside if statement\n");
		      printf("listImageFiles: i = %d\n", i);
		      //printf("listImageFiles: directory_ptr[%d].name = %s\n", i, directory_ptr[i].name);
		      //printf("File name %d = %s\n", i, directory_ptr[i].name);
		      
		      //printf("%d %s %s\n", inode_array_ptr[i].size, ctime(&file_time), "file.txt");
		      printf("%s\n", ctime(&file_time));
		      //printf("%d\n", inode_array_ptr[i]->size);

		      printf("%s\n", directory_ptr[i].name);
		      printf("%d %lu %s\n", inode_array_ptr[i]->size , inode_array_ptr[i]->date, directory_ptr[i].name);
		}
	}
	//printf("Ending findFreeDirectoryEntry()\n\n");
    return;
    
}

//**********************************************************************
//This function is called when user types "df"
//This function is used in the put() as well
int df()
{
	printf("\nBeginning df()\n");
	int count = 0;
	int i = 0;
	
	for(i = 130; i < 4226; i++)
	//for(i = 127; i < 4226; i++)//??????????????????????????????????????????????????????
	{
		if(used_blocks[i] == 0)
		{
			count++;
		}
	}
	printf("df:count = %d\n", count);
    printf("BLOCK_SIZE = %d\n", BLOCK_SIZE);
    printf("Ending df()\n\n");
	return count*BLOCK_SIZE;
	
}

//**********************************************************************	
int findFreeInodeBlockEntry(int inode_index)//need to write smarter version (52:32)
{
  	printf("\nBeginning findFreeInodeBlockEntry\n");
	int i;
	int retval = 0;
	for(i = 0; i < 32; i++)
	{
		if(inode_array_ptr[inode_index]->blocks[i] == 0)
		{
			retval = i;
			break;
		}
		
	}
	printf("Ending findFreeInodeBlockEntry\n\n");
	return retval;

}

//**********************************************************************
int findFreeDirectoryEntry()//Find inode that is free
{
    printf("\nBeginning findFreeDirectoryEntry()\n");
	int i;
	//int retval = -1;
	int retval = 0;
    printf("findFreeDirectoryEntry: retval = %d\n", retval);
    
	for(i = 0; i < 125; i++)
	{
		if(directory_ptr[i].valid == 0)//inode is not being used
		{
			retval = i;
			break;
		}
	}
	printf("Ending findFreeDirectoryEntry()\n\n");
	return retval;
}

//**********************************************************************
int findFreeInode()
{
	printf("\nBeginning findFreeInode\n");
	int i;
	//int retval = -1;
	int retval = 0;
	 //for(i = 0; i < 125; i++)
	 for(i = 1; i < 126; i++)
	 {
	   printf("findFreeInode: inside for loop: inode_array_ptr[%d]->valid = %d\n",i, inode_array_ptr[i]->valid);
		 if(inode_array_ptr[i]->valid ==0)
		 {
			 retval = i;
			 break;
		 }
	 }
	 printf("findFreeInode: just before return retval: retval = %d\n", retval);
	 printf("End findFreeInode\n\n");
	 return retval;
}
//**********************************************************************
int findFreeBlock()
{
	printf("Beginning findFreeBlock()\n");
	//int retval = -1;
	int retval = 0;
	int i = 0;
	//for(i = 130; i < 4226; i++)
	for(i = 130; i < 4226; i++)
	{
		if(used_blocks[i] == 0)
		{
			retval = i;
			break;
		}
		
	}
	
	return retval;
	
	printf("End findFreeBlock()\n\n");
}

//**********************************************************************
void put(char* filename)
{
	//put() starts here
	printf("Beginning put()\n");
	struct stat buf;
	
	//..................................................................
	//Check to see if file name exceeds 32 characters
	int file_name_length = strlen(filename);
	printf("put: file_name_length = %d\n", file_name_length);
	if(file_name_length > 32)
	{
	  printf("put error: File name too long.\n");
	  return;
	}
    
	//..................................................................
    printf("put(): Error: File not found\n");
	int status = stat(filename, &buf);
	if(status == -1)//????????????????????????????????????????????????????????????
	//if(status == 0)
	{
		printf("Error: File not found \n");
		return;
	}
    
	//..................................................................
    printf("put(): just before if(buf.st_size > df())\n");
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
	printf("strlen(filename) = %d\n", strlen(filename));
	strncpy(directory_ptr[dir_idx].name, filename, strlen(filename));
    printf("put(): directory_ptr[dir_idx].name = %s\n", directory_ptr[dir_idx].name);

	int inode_idx = findFreeInode();//Call function findFreeInode()
	
	if(inode_idx == -1)
	{
		printf("Error: No free inodes\n");
		return;
	}
	
	printf("put: just before directory_ptr[dir_idx].inode_idx = inode_idx: inode_idx = %d\n", inode_idx);
	directory_ptr[dir_idx].inode_idx = inode_idx;
	
	
	
	inode_array_ptr[inode_idx]->size = buf.st_size;
	printf("put: inode_array_ptr[inode_idx]->size: %d", inode_array_ptr[inode_idx]->size);
	inode_array_ptr[inode_idx]->date = time(NULL);
	inode_array_ptr[inode_idx]->valid = 1;
	
	
	
	//Open the input file read-only
	printf("put(): open %s for reading\n", filename);
	FILE* ifp = fopen(filename, "r");
	

	
	
	int copy_size = buf.st_size;//set copy_size to be size of the file
	int offset = 0;//the first time we want to read those bytes
	
	//..................................................................
	int block_index = findFreeBlock();
	
	if(block_index == -1)
	{
		printf("Error: Can't find free block(1)\n");
		//Cleanup a bunch of directory and inode stuff (optional)
		return;
	}
	//..................................................................
    
	while(copy_size >= BLOCK_SIZE)
	{
		int block_index = findFreeBlock();
		
		if(block_index == -1)
		{
			printf("Error: Can't find free block(2)\n");
			//cleanup a bunch of directory and inode stuff
			return;
		}
		
		used_blocks[block_index] = 1;
		
		int inode_block_entry = findFreeInodeBlockEntry(inode_idx);
		if(inode_block_entry == -1)
		{
			printf("Error: Can't find free node block(1)\n");
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
			printf("Error: Can't find free block(3)\n");
			//Cleanup a bunch of directory and inode stuff
			return;
		}
		printf("put: just before int inode_block_entry = findFreeInodeBlockEntry(inode_idx): inode_idx = %d\n", inode_idx);
		int inode_block_entry = findFreeInodeBlockEntry(inode_idx);
		printf("put: if(copy_size > 0): inode_block_entry = %d\n", inode_block_entry);
		if(inode_block_entry == -1)
		{
			printf("Error: Can't find free node block(2)\n");
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
    //printf ("mfs>");//print the bash shell prompt
    printf ("mfs++++++++++++++++++++++++++++>");//print the bash shell prompt

    
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

    //execute command "put"

    if((strstr(token[0], "put")!=NULL) && strstr(token[1], "")!=NULL)//call put()
    {
	
        //char* filename;
        printf("Call function put()\n");
        printf("Local file name token[1]= %s\n", token[1]);
		printf("token[1] = %s\n", token[1]);
		int token_length = strlen(token[1]);
		printf("token_length = %d\n", token_length);
        put(token[1]);


    }
        

    //..................................................................
    
        //execute command "list"

    if((strstr(token[0], "list")!=NULL))//call listImageFiles()
    {
	printf("Call function listImageFiles()\n");
           listImageFiles();

    }
        
    //..................................................................   
    //execute command "df"

    if((strstr(token[0], "df")!=NULL))//call df()
    {
	printf("Call function df()\n");
        int df_return = df();
        printf("%d bytes free.\n", df_return);//df_return = count*BLOCK_SIZE

    }
        
    //..................................................................   
    
        //execute command "open"

    if((strstr(token[0], "open")!=NULL) && strstr(token[1], "")!=NULL)//call put()
    //if((strstr(token[0], "open")!=NULL))//call put()
    {
        //char* filename;
        printf("Call function open()\n");
        printf("Local file name token[1]= %s\n", token[1]);
        put(token[1]);


    }
        


    
    free( head_ptr );

  }//while(1) loop
  
  return 0;
  
  // e2520ca2-76f3-11ec-90d6-0242ac120003
}


//

// File System Requiremments: 0/14

/*
1.1 Your Program will print out a prompt of mfs> when it is ready to accept input
1.2 The following commands shall be supported:
	put
	get
	get
	del
	undel
	list
	df
	open
	close
	createfs
	savefs
	attrib
	quit

1.3 Your program will be allocating 4226 blocks for the file system.

1.4 The file system shall support files up to 10,240,000 bytes in size.

1.5 The file system shall support up to 125 files

1.6 The file system block size shall be 8192 bytes.

1.7 The file system shall use an indexed allocation scheme.

1.8 The file system shall support file names of up to 32 characters.

1.9 Supported file names shall only be alphanumeric with ".".
	There shall be no restriction to how many characters appear before or after the "."
	There shall be support for files without a "."


1.10 The file system shall store the directory in the first two(blocks 0-1) disk blocks

1.11 The file system shall allocate blocks 5-130 for inodes

1.12 The file system shall use block 2 for the free inode map.
	Use at most 8 bits for each entry.

1.13 The file system shall use block 3 for the free block map.
	Use at most 8 bits for each entry.

1.14 The directory structure shall be a single level hierarchy with no subdirectories.
*/


// Command Requirements 0/10

/*
2.1 The put command shall alow the user to put a new file into the file system
2.1.1 The command shall take the form:
	put <filename>

2.1.2 If the filename is too long 
	put error: File name too long.

2.1.3 If there is not enough disk space for the file an error will be returned stating:
	put error: Not enough disk space.

2.2 The get command shall allow the user to retrieve a file from the file system and place it in the current working directory.
2.2.1 The command shall take the form:
	get <filename>
	and
	get <filename> <newfilename>
2.2.2 If no new filename is specified the get shall copy the file to the current working directory using the old filename.
2.2.3 If the file does not exist in the file system an error will be printed that states: 
	get error: File not found.

2.3 The del command shall allow the user to delete a file from the file system
2.3.2 If the file does exist in the file system it shall be deleted and all the space available for additional files.
	del error: File not found.

2.4 The undel command - the undel command shall allow the user to undelete a file that has been deleted from the file system.
2.4.2 If the file does exist in the file systme direcory and marked deleted it shall be undeleted.
2.4.3 If the file is not found in the directory then the following shall be printed:
	undel: Can not find the file

2.5 The list command shall display 
	all the files in the file system
	their size in bytes and
	the time they were added to the file system
2.5.1 If no files are in the file system a message shall be printed:
	list: No files found.
2.5.2 File that are marked as hidden shall not be listed

2.6 The df command
	The df command shall display the amount of free space int he file system in bytes

2.7 The open command
	The open command shall open a file system image file with the name and path given by the user
2.7.1 If the file is not found a message shall be printed
	open: File not found

2.8 The save command
	The save command shall write the file system to disk.

2.9 The attrib command
	The attrib command sets or removes an attribute from the file.
2.9.1 Valid attributes are:
	h
	r
2.9.2 To set the attribute on the file the attribute tag is given with a +, ex:
	attrib +h foo.txt
2.9.3 To remove the attribute on the file the attribute tag is given with a -, ex:
	attrib -h foo.txt

2.9.4 If the file is not found a message shall be printed:
	attrib: File not found

2.10 The createfs command
	The createfs command shall create a file system image file with the named provided by the user

2.10.1 If the file name is not provided a message shall be printed:
	createfs: File not found

*/

// 3.0 Nonfunctional Requirements 0/10

/*
3.1 You may code your solution in C or C++
3.2 C files shall end in .c
3.3 Your source files must be ASCII text files. No binary submissions will be accepted.
3.4 Tabs or spaes shall be used to indent the code
	Your code must use one or the other
	All indentation must be consistent
3.5 No line of code shall exceed 100 characters.
3.6 Each source code file shall have the following header filled out:
3.7 All code must be well commented.
	This means descriptive comments that tell the intent of the code, not just what the code is executing.

3.8 Keep your curly brace placement consistent. If you place curly braces on a new line, always plaec curly braces on a new end.
	Don't mix end line brace placement with new line brace placement
3.9 Each function should have a header that describes its name, any parameters expected, any return values, as well as a description of what the function does.
3.10 If your solution uses multiple source files you must also submit a cmake file or a makefile.
	Subsmissions without a cmake file or makefile will have a 20 pt deduction.

*/
