// The MIT License (MIT)
// 
// Copyright (c) 2016, 2017 Trevor Bakker 
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
#include <sys/stat.h>

#define WHITESPACE " \t\n"      // We want to split our command line up into tokens
                                // so we need to define what delimits our tokens.
                                // In this case  white space
                                // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255    // The maximum command-line size

#define MAX_NUM_ARGUMENTS 5     // Mav shell only supports five arguments

#define NUM_BLOCKS 512
#define BLOCK_SIZE 1024

unsigned char file_data[NUM_BLOCKS][BLOCK_SIZE];

int    status;                   // Hold the status of all return values.
struct stat buf;                 // stat struct to hold the returns from the stat call

int copy( char *file_read, char *file_write )
{

  
 
    // Open the input file read-only 
    FILE *ifp = fopen ( file_read, "r" ); 
    printf("Reading %d bytes from %s\n", (int) buf . st_size, file_read );
 
    // Save off the size of the input file since we'll use it in a couple of places and 
    // also initialize our index variables to zero. 
    int copy_size   = buf . st_size;

    // We want to copy and write in chunks of BLOCK_SIZE. So to do this 
    // we are going to use fseek to move along our file stream in chunks of BLOCK_SIZE.
    // We will copy bytes, increment our file pointer by BLOCK_SIZE and repeat.
    int offset      = 0;               

    // We are going to copy and store our file in BLOCK_SIZE chunks instead of one big 
    // memory pool. Why? We are simulating the way the file system stores file data in
    // blocks of space on the disk. block_index will keep us pointing to the area of
    // the area that we will read from or write to.
    int block_index = 0;
 
    // copy_size is initialized to the size of the input file so each loop iteration we
    // will copy BLOCK_SIZE bytes from the file then reduce our copy_size counter by
    // BLOCK_SIZE number of bytes. When copy_size is less than or equal to zero we know
    // we have copied all the data from the input file.
    while( copy_size > 0 )
    {

      // Index into the input file by offset number of bytes.  Initially offset is set to
      // zero so we copy BLOCK_SIZE number of bytes from the front of the file.  We 
      // then increase the offset by BLOCK_SIZE and continue the process.  This will
      // make us copy from offsets 0, BLOCK_SIZE, 2*BLOCK_SIZE, 3*BLOCK_SIZE, etc.
      fseek( ifp, offset, SEEK_SET );
 
      // Read BLOCK_SIZE number of bytes from the input file and store them in our
      // data array. 
      int bytes  = fread( file_data[block_index], BLOCK_SIZE, 1, ifp );

      // If bytes == 0 and we haven't reached the end of the file then something is 
      // wrong. If 0 is returned and we also have the EOF flag set then that is OK.
      // It means we've reached the end of our input file.
      if( bytes == 0 && !feof( ifp ) )
      {
        printf("An error occured reading from the input file.\n");
        return -1;
      }

      // Clear the EOF file flag.
      clearerr( ifp );

      // Reduce copy_size by the BLOCK_SIZE bytes.
      copy_size -= BLOCK_SIZE;
      
      // Increase the offset into our input file by BLOCK_SIZE.  This will allow
      // the fseek at the top of the loop to position us to the correct spot.
      offset    += BLOCK_SIZE;

      // Increment the index into the block array 
      block_index ++;
    }

    // We are done copying from the input file so close it out.
    fclose( ifp );
 
    //*********************************************************************************
    //
    // The following chunk of code demonstrates similar functionality to your get command
    //

    // Now, open the output file that we are going to write the data to.
    FILE *ofp;
    ofp = fopen(file_write, "w");

    if( ofp == NULL )
    {
      printf("Could not open output file: %s\n", file_write );
      perror("Opening output file returned");
      return -1;
    }

    // Initialize our offsets and pointers just we did above when reading from the file.
    block_index = 0;
    copy_size   = buf . st_size;
    offset      = 0;

    printf("Writing %d bytes to %s\n", (int) buf . st_size, file_write );

    // Using copy_size as a count to determine when we've copied enough bytes to the output file.
    // Each time through the loop, except the last time, we will copy BLOCK_SIZE number of bytes from
    // our stored data to the file fp, then we will increment the offset into the file we are writing to.
    // On the last iteration of the loop, instead of copying BLOCK_SIZE number of bytes we just copy
    // how ever much is remaining ( copy_size % BLOCK_SIZE ).  If we just copied BLOCK_SIZE on the
    // last iteration we'd end up with gibberish at the end of our file. 
    while( copy_size > 0 )
    { 

      int num_bytes;

      // If the remaining number of bytes we need to copy is less than BLOCK_SIZE then
      // only copy the amount that remains. If we copied BLOCK_SIZE number of bytes we'd
      // end up with garbage at the end of the file.
      if( copy_size < BLOCK_SIZE )
      {
        num_bytes = copy_size;
      }
      else 
      {
        num_bytes = BLOCK_SIZE;
      }

      // Write num_bytes number of bytes from our data array into our output file.
      fwrite( file_data[block_index], num_bytes, 1, ofp ); 

      // Reduce the amount of bytes remaining to copy, increase the offset into the file
      // and increment the block_index to move us to the next data block.
      copy_size -= BLOCK_SIZE;
      offset    += BLOCK_SIZE;
      block_index ++;

      // Since we've copied from the point pointed to by our current file pointer, increment
      // offset number of bytes so we will be ready to copy to the next area of our output file.
      fseek( ofp, offset, SEEK_SET );
    }

    // Close the output file, we're done. 
    fclose( ofp );
  
  

  return 0;
};

int run = 1;
int main()
{

  char * cmd_str = (char*) malloc( MAX_COMMAND_SIZE );

  while( run )
  {
    // Print out the mfs prompt
    printf ("mfs> ");

    // Read the command from the commandline.  The
    // maximum command that will be read is MAX_COMMAND_SIZE
    // This while command will wait here until the user
    // inputs something since fgets returns NULL when there
    // is no input
    while( !fgets (cmd_str, MAX_COMMAND_SIZE, stdin) );

    /* Parse input */
    char *token[MAX_NUM_ARGUMENTS];

    int   token_count = 0;                                 
                                                           
    // Pointer to point to the token
    // parsed by strsep
    char *arg_ptr;                                         
                                                           
    char *working_str  = strdup( cmd_str );                

    // we are going to move the working_str pointer so
    // keep track of its original value so we can deallocate
    // the correct amount at the end
    char *working_root = working_str;

    // Tokenize the input stringswith whitespace used as the delimiter
    while ( ( (arg_ptr = strsep(&working_str, WHITESPACE ) ) != NULL) && 
              (token_count<MAX_NUM_ARGUMENTS))
    {
      token[token_count] = strndup( arg_ptr, MAX_COMMAND_SIZE );
      if( strlen( token[token_count] ) == 0 )
      {
        token[token_count] = NULL;
      }
        token_count++;
    }

    // Now print the tokenized input as a debug check
    // \TODO Remove this code and replace with your shell functionality

    int token_index  = 0;
    for( token_index = 0; token_index < token_count; token_index ++ ) 
    {
      ///printf("token[%d] = %s\n", token_index, token[token_index] );
      
      if(token[token_index] != NULL)
      {
        int result;

        // if exit
          // use break
        char strexit[] = "exit";
        result = strcmp(token[token_index], strexit);
        if(result == 0)
        {
          run = 0;
        }
        // if copy command
        char strcopy[] = "copy";
        result = strcmp(token[token_index], strcopy);
        if(result == 0)
        {

          // Verify there are enough parameters passed to run the program.  This program should have
          // an argc count of 3.  argv[0] = program name, argv[1] = file to copy from, 
          // argv[2] = file to copy to.  If there aren't exactly three parameters then dump out a 
          // message and return. 
          if( token_count != 4 ) {
            printf("Incorrect number of parameters. \nUse: \n	copy filein fileout\n");
          } 

          //*********************************************************************************
          //
          // The following chunk of code demonstrates similar functionality of your put command
          //

          

          // Call stat with our input filename to verify that the file exists.  It will also 
          // allow us to get the file size. We also get interesting file system info about the
          // file such as inode number, block size, and number of blocks.  For now, we don't 
          // care about anything but the filesize.
          status =  stat( token[1], &buf ); 

          // If stat did not return -1 then we know the input file exists and we can use it.
          if( status != -1 )
          {
            copy(token[1], token[2]);
          }
          else
          {
            printf("Unable to open file: %s\n", token[1] );
            perror("Opening the input file returned: ");
          }
        }
      }
    }
    free( working_root );
  }
  return 0;
}
