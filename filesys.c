/*#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])  
{
    //declare command arguemnts
    char* imgName = argv[1];
    char* mountDir = " ./mountpoint";
    char mountCommand[100] = "sudo mount ";
    char mkdirCommand[50] = "mkdir";

    //concatenate mount command arguments
    strcat(mountCommand, imgName);
    strcat(mountCommand, mountDir);

    //concatenate mkdir command arguemnts
    strcat(mkdirCommand, mountDir);

    //make mountpoint dir
    system(mkdirCommand);

    //mount image
    system(mountCommand);
}
*/
// starter file provided by operating systems ta's
// include libraries

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#define BUFSIZE 40
#define PATH_SIZE 4096 // max possible size for path string.
#define OPEN_FILE_TABLE_SIZE 10 // max files for files.
#define MAX_NAME_LENGTH 11 // 11 in total but 3 for extensions, we onl use 8.

// data structures for FAT32
// Hint: BPB, DIR Entry, Open File Table -- how will you structure it?
// stack implementaiton -- you will have to implement a dynamic stack
// Hint: For removal of files/directories

typedef struct __attribute__((packed)){
        unsigned char BS_jmpBoot[3];
        unsigned char BS_OEMName[8];
        unsigned short BPB_BytsPerSec;
        unsigned char BPB_SecPerClus;
        unsigned short BPB_RsvdSecCnt;
        unsigned char BPB_NumberofFATS;
        unsigned short BPB_RootEntCnt;
        unsigned short BPB_TotalSectorsShort;
        unsigned char BPB_MediaDescriptor;
        unsigned short BPB_FATSz16;
        unsigned short BPB_SectorPerTrack;
        unsigned short BPB_NumHeads;
        unsigned int BPB_HiddenSec;
        unsigned int BPB_TotalSec32;

        unsigned int BPB_FATSz32;
        unsigned short BPB_ExtFlags;
        unsigned short BPB_FSVer;
        unsigned int BPB_RootClus;

        unsigned short BPB_FSInfo;
        unsigned short BPB_BkBootSec;
        unsigned char BPB_Reserved[12];
        unsigned char BPB_DrvNum;
        unsigned char BS_Reserved1;
        unsigned char BS_BootSig;
        unsigned int BS_VolID;
        unsigned char BS_VolLab[11];
        unsigned char BS_FileSysType[8];
} BPB_Block;

typedef struct {
	char path[PATH_SIZE]; // path string
	// add a variable to help keep track of current working directory in file.
	// Hint: In the image, where does the first directory entry start?
} CWD;

typedef struct {
	int size;
	char ** items;
} tokenlist;

FILE *imgFile;
BPB_Block BootBlock;
Environment ENV;
int FirstDataSector;

tokenlist * tokenize(char * input);
void free_tokens(tokenlist * tokens);
char * get_input();
void add_token(tokenlist * tokens, char * item);
void add_to_path(char * dir);
void addtoPath(int, char*);

// global variables
CWD cwd;

int main(int argc, char * argv[]) {
	// error checking for number of arguments.
	// read and open argv[1] in file pointer.
	// obtain important information from bpb as well as initialize any important global variables

	//declare command arguemnts
	char* imgName = argv[1];
	char* mountDir = " ./mountpoint";
	char mountCommand[100] = "sudo mount ";
	char mkdirCommand[50] = "mkdir";

    	//concatenate mount command arguments
    	strcat(mountCommand, imgName);
    	strcat(mountCommand, mountDir);

    	//concatenate mkdir command arguemnts
    	strcat(mkdirCommand, mountDir);

    	//make mountpoint dir
    	system(mkdirCommand);

    	//mount image
    	system(mountCommand);

	memset(cwd.path, 0, PATH_SIZE);

	// parser
	char *input;

	while(1) {
		printf("%s/>", cwd.path);
		input = get_input();
		tokenlist * tokens = tokenize(input);
		printf("tokens size: %d\n", tokens->size);
		for(int i = 0; i < tokens->size; i++) {
			printf("token %d: %s\n", i, tokens->items[i]);
		}

		if(strcmp(tokens->items[0], "exit") == 0){
			return 0;
		}
		else if(strcmp(tokens->items[0], "info") == 0){
			printf("calling info\n");
		}
		else if(strcmp(tokens->items[0], "ls") == 0){
			printf("calling ls\n");
		}
		else if(strcmp(tokens->items[0], "cd") == 0){
			printf("calling cd\n");
		}
		else if(strcmp(tokens->items[0], "size") == 0){
			printf("calling cd\n");
		}
		else if(strcmp(tokens->items[0], "creat") == 0){
			printf("calling creat\n");
		}
		else if(strcmp(tokens->items[0], "mkdir") == 0){
			printf("calling mkdir\n");
		}
		else if(strcmp(tokens->items[0], "rm") == 0){
			printf("calling rm\n");
		}
		else if(strcmp(tokens->items[0], "rmdir") == 0){
			printf("calling rmdir\n");
		}
		else if(strcmp(tokens->items[0], "open") == 0){
			printf("calling open\n");
		}
		else if(strcmp(tokens->items[0], "close") == 0){
			printf("calling close\n");
		}
		else if(strcmp(tokens->items[0], "read") == 0){
			printf("calling read\n");
		}
		else if(strcmp(tokens->items[0], "write") == 0){
			printf("calling write\n");
		}

		//add_to_path(tokens->items[0]); // move this out to its correct place;
		free(input);
		free_tokens(tokens);
	}

	return 0;
}

// helper functions -- to navigate file image
// commands -- all commands mentioned in part 2-6 (17 cmds)
// add directory string to cwd path -- helps keep track of where we are in image.
void add_to_path(char * dir) {
	if(dir == NULL) {
		return;
	}
	else if(strcmp(dir, "..") == 0) {
		char *last = strrchr(cwd.path, '/');
		if(last != NULL) {
			*last = '\0';
		}
	} else if(strcmp(dir, ".") != 0) {
		strcat(cwd.path, "/");
		strcat(cwd.path, dir);
	}
}

void free_tokens(tokenlist *tokens)
{
	for (int i = 0; i < tokens->size; i++)
		free(tokens->items[i]);
	free(tokens->items);
	free(tokens);
}

// take care of delimiters {'\"', ' '}
tokenlist * tokenize(char * input) {
	int is_in_string = 0;
	tokenlist * tokens = (tokenlist *) malloc(sizeof(tokenlist));
	tokens->size = 0;
	tokens->items = (char **) malloc(sizeof(char *));
	char ** temp;
	int resizes = 1;
	char * token = input;
	for(; *input != '\0'; input++) {
		if(*input == '\"' && !is_in_string) {
			is_in_string = 1;
			token = input + 1;
		} else if(*input == '\"' && is_in_string) {
			*input = '\0';
			add_token(tokens, token);
			while(*(input + 1) == ' ') {
				input++;
			}
			token = input + 1;
			is_in_string = 0;
		} else if(*input == ' ' && !is_in_string) {
			*input = '\0';
			while(*(input + 1) == ' ') {
				input++;
			}
			add_token(tokens, token);
			token = input + 1;
		}
	}
	if(is_in_string) {
		printf("error: string not properly enclosed.\n");
		tokens->size = -1;
		return tokens;
	}

	// add in last token before null character.
	if(*token != '\0') {
		add_token(tokens, token);
	}

	return tokens;
}

void add_token(tokenlist *tokens, char *item)
{
	int i = tokens->size;
	tokens->items = (char **) realloc(tokens->items, (i + 2) * sizeof(char *));
	tokens->items[i] = (char *) malloc(strlen(item) + 1);
	tokens->items[i + 1] = NULL;
	strcpy(tokens->items[i], item);
	tokens->size += 1;
}

char * get_input() {
	char * buf = (char *) malloc(sizeof(char) * BUFSIZE);
	memset(buf, 0, BUFSIZE);
	char c;
	int len = 0;
	int resizes = 1;
	int is_leading_space = 1;
	while((c = fgetc(stdin)) != '\n' && !feof(stdin)) {
		// remove leading spaces.
		if(c != ' ') {
			is_leading_space = 0;
		} else if(is_leading_space) {
			continue;
		}
		buf[len] = c;
		if(++len >= (BUFSIZE * resizes)) {
			buf = (char *) realloc(buf, (BUFSIZE * ++resizes) + 1);
			memset(buf + (BUFSIZE * (resizes - 1)), 0, BUFSIZE);
		}
	}
	buf[len + 1] = '\0';
	// remove trailing spaces.
	char * end = &buf[len - 1];
	while(*end == ' ') {
		*end = '\0';
		end--;
	}
	return buf;
}
