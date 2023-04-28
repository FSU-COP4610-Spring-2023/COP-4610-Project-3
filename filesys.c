// starter file provided by operating systems ta's
// include libraries

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
//#include <ctype.h>
//#include <fcntl.h>
#define BUFSIZE 40
#define PATH_SIZE 4096 // max possible size for path string.
#define OPEN_FILE_TABLE_SIZE 10 // max files for files.
#define MAX_NAME_LENGTH 11 // 11 in total but 3 for extensions, we onl use 8.
#define NUM_ALPHA 26
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

//struct to check on free space i believe. To my knwoledge 
typedef struct __attribute__((packed)) {
        unsigned int FSI_LeadSig;
        unsigned char FSI_Reserved1[480];                       //FAT32 spec says its 480 byts so i think this is right
        unsigned int FSI_StrucSig;
        unsigned int FSI_Free_Count;
        unsigned int FSI_Nxt_Free;
        unsigned char FSI_Reserved2[12];                        //same here about the size
        unsigned int FSI_TrailSig;

} FSInfo;

typedef struct __attribute__((packed)){
unsigned char DIR_Name[11]; // 8 bytes (1 byte read), this
//representation is fine as it is character string.
unsigned char DIR_Attr;
unsigned char DIR_NTRes;
unsigned char DIR_CrtTimeTenth;
unsigned short DIR_CrtTime;
unsigned short DIR_CrtDate;
unsigned short DIR_LstAccDate;
unsigned short DIR_FstClusHi; // 2 bytes (2 byte read), this
//representation is in high word of first clus
unsigned short DIR_WrtTime;
unsigned short DIR_WrtDate;

unsigned short DIR_FstClusLo; // 2 bytes (2 byte read), this
//representation is in low word of first clus.
unsigned int DIR_FileSize; // 4 bytes (4 byte read), this
//representation is size of file.
} DIR;

typedef struct {
	char fileName[11];
	unsigned int currentFilePosition;
	unsigned int currentFilePositionOffset;
	int openedMethod;
	unsigned int offset;
	unsigned int fileSize;
	char* filePath;
} OpenFile;


// global variables
FILE *imgFile;
BPB_Block BootBlock;
FSInfo fsi;
int FirstDataSector;
CWD cwd;
int CurrentDirectory;
int CurrentFileCluster;
unsigned int FirstFatSector;
unsigned int BytesPerCluster;
int NumOpenFiles;

OpenFile OpenedFiles[10];
tokenlist * tokenize(char * input);
void free_tokens(tokenlist * tokens);
char * get_input();
void add_token(tokenlist * tokens, char * item);
void add_to_path(char * dir);
int lsCmd(int);
int cdCmd(int, char*);
void OpenCmd(char*, char*);
int getHiLoClus(unsigned short, unsigned short);
void closeCmd(char*);
void lsofCmd();
void sizeCmd(char*);
void lseekCmd(char*, unsigned int);
void Info(long);
void readCmd(char *, unsigned int);
void renameCmd(char *, char* );
//void findFile(int);
void creat(char*);
int findFile(char*);


int main(int argc, char * argv[]) {
        // error checking for number of arguments.
        // read and open argv[1] in file pointer.
        // obtain important information from bpb as well as initialize any important global variables

        imgFile = fopen(argv[1], "r+");
        fread(&BootBlock, sizeof(BPB_Block), 1, imgFile);

        BytesPerCluster = BootBlock.BPB_BytsPerSec * BootBlock.BPB_SecPerClus;
        FirstDataSector = BootBlock.BPB_RsvdSecCnt + (BootBlock.BPB_NumberofFATS* BootBlock.BPB_FATSz32);
        FirstFatSector = BootBlock.BPB_RsvdSecCnt; //offet
        add_to_path(argv[1]);

	//set all files to unopened or 0
	for(int i = 0; i < 10; i++){
		OpenedFiles[i].openedMethod = 0;
	}

        // parser
        char *input;

        long offset = 0;
       // CurrentDirectory = FirstDataSector * BootBlock.BPB_BytsPerSec +32;
        CurrentDirectory = BootBlock.BPB_RootClus;
        while(1) {
                printf("%s/>", cwd.path);
                input = get_input();
                tokenlist * tokens = tokenize(input);
//              add_to_path(tokens->items[0]);
//              printf("tokens size: %d\n", tokens->size);
//              for(int i = 0; i < tokens->size; i++) {
//                      printf("token %d: %s\n", i, tokens->items[i]);
//              }

                if(strcmp(tokens->items[0], "exit") == 0){
                        return 0;
                }
                else if(strcmp(tokens->items[0], "info") == 0){
                        offset = BootBlock.BPB_BytsPerSec * BootBlock.BPB_FSInfo;
                        Info(offset);
                }
                else if(strcmp(tokens->items[0], "ls") == 0){
                        int ls = 0;
                        ls = lsCmd(CurrentDirectory);
                }
                else if(strcmp(tokens->items[0], "cd") == 0){
                        if(tokens->items[1] != NULL )
                        {
                        CurrentDirectory = cdCmd(CurrentDirectory, tokens->items[1]);
                        }
                }
                else if(strcmp(tokens->items[0], "size") == 0){
			if(tokens->items[1] == NULL){
				printf("Enter a valid filename\n");
			}else {
				sizeCmd(tokens->items[1]);
			}
                }
                else if(strcmp(tokens->items[0], "creat") == 0){
                        printf("calling creat\n");
                        creat(tokens->items[1]);
                }
                else if(strcmp(tokens->items[0], "mkdir") == 0){
                        printf("calling mkdir\n");
                        if(tokens->items[1]!= NULL){
                        mkdir(tokens->items[1]);
                        }
/*			int mkdirRes = mkdir(BootBlock, tokens->items[1]);
                        if(mkdirRes == 0){
				printf("Directory %s was created\n", tokens->items[1]);
			}else if(mkdirRes == -1){
				printf("Directory %s already exists\n", tokens->items[1]);
			}
			int startingCluster = findUnusedCluster(imgFile, FirstDataSector);
                        //imgFile below this point in scope was fp, for the sake of compile testing
                        //swapped to imgFile.
                        createDirectoryEntry(imgFile, tokens->items[1], startingCluster);
                        updateFatEntry(imgFile, startingCluster, 0xFFFFFFFF);*/
                }
                else if(strcmp(tokens->items[0], "rm") == 0){
                        printf("calling rm\n");
                        printLoc();
                }
                else if(strcmp(tokens->items[0], "rmdir") == 0){
                        if(tokens->items[1] != NULL)
                        {
                                rmDir(tokens->items[1]);
                        }
                }
                else if(strcmp(tokens->items[0], "open") == 0){
			printf("calling opencommand\ntoken 2: %s\n", tokens->items[2]);
			if(tokens->items[1] == NULL){
				printf("Enter valid filename\n");
			}else if(tokens->items[2] == NULL){
				printf("Enter a valid value for file oeration. Either -r -w -rw or -wr\n");
			}else if(!strcmp(tokens->items[2], "-r") || !strcmp(tokens->items[2], "-w") 
				|| !strcmp(tokens->items[2], "-rw") || !strcmp(tokens->items[2], "-wr")){
				OpenCmd(tokens->items[1], tokens->items[2]);
			}else{
				printf("Enter a valid value for file oeration. Either -r -w -rw or -wr\n");
			}
		}
                else if(strcmp(tokens->items[0], "close") == 0){
			if(tokens->items[1] == NULL){
				printf("Enter a valid filename\n");
                	}else {
				closeCmd(tokens->items[1]);
			}
		}
		else if(strcmp(tokens->items[0], "lsof") == 0){
			lsofCmd();
		}
		else if(strcmp(tokens->items[0], "lseek") == 0){
			if(tokens->items[1] == NULL|| tokens->items[2] == NULL){
				printf("Enter valid values for lseek command\n");
			}else{
				lseekCmd(tokens->items[1], atoi(tokens->items[2]));
			}
		}
                else if(strcmp(tokens->items[0], "read") == 0){
			if(tokens->items[1] == NULL){
				printf("Enter a valid file name\n");
			}
			else{
				readCmd(tokens->items[1], atoi(tokens->items[2]));
			}
                }
                else if(strcmp(tokens->items[0], "write") == 0){
                        if(tokens->items[1] == NULL){
                                printf("Enter a valid filename\n");
                        }else if(tokens->items[2] == NULL){
                                printf("Enter a valid value to write into the file\n");
                        }else{
                                writeCmd(tokens->items[1], tokens->items[2]);
                        }
                }
		else if(strcmp(tokens->items[0], "rename") == 0){
			printf("calling rename\n");
			renameCmd(tokens->items[1], tokens->items[2]);
		}

                //add_to_path(tokens->items[0]); // move this out to its correct place;
                free(input);
                free_tokens(tokens);
        }

        return 0;
}

void trim(char* ptr)
{
        for(int i = 0; i < 11; i++)
        {
                if(ptr[i] == 0x20)
                {
                        ptr[i] = 0x00;
                }
        }
}

int getHiLoClus(unsigned short hi, unsigned short lo){
        hi = (hi << 8);
        unsigned int concat = hi | lo;
        return concat;
}

int FatEntryOffset(int cluster)
{
        int FatEntryOffset = (FirstFatSector * BootBlock.BPB_BytsPerSec) + (cluster * 4);
        //printf("FatEntryOffset = %d\nFirst Fat Sector = %d\nCluster = %d\n", FatEntryOffset, FirstFatSector, Cluster);
        //printf("%d\n", FirstFatSector);
        return FatEntryOffset;
}

int GetNextCluster(int cluster)
{
        fseek(imgFile, FatEntryOffset(cluster), SEEK_SET);
        int nextCluster;
        fread(&nextCluster, sizeof(int), 1, imgFile);
        return nextCluster;
}

int ClusterByteOffset(int cluster) {
        int clus = (FirstDataSector + ((cluster - 2) * BootBlock.BPB_SecPerClus)) * BootBlock.BPB_BytsPerSec;
        return clus;
}

int BackToFat(int cluster){
	int next_cluster = (FatEntryOffset(cluster)) + 4;			//lol
//	printf("next_cluster: %d\n", next_cluster);
//	printf("test 1\n");
	fseek(imgFile, next_cluster, SEEK_SET);
//	printf("test 2\n");
	int next_chain;
	fread(&next_chain, sizeof(int), 1, imgFile);
//	int test = ClusterByteOffset(*next_chain);
//	printf("test: %d\n", test);
//	printf("next_chain: %d\n", next_chain);
//	printf("test 3\n");
	if(next_chain < 0x0FFFFFF8){
		//printf("test 4\n");
		return next_chain;;
	}else{
		//printf("test 5\n");
		return 0;
	}
}

int allocateClus(cluster)
{
        int value;
        int* ptr = &value;
        int ctr;
        int found = 0xFFFFFFFF;
        fseek(imgFile, FatEntryOffset(cluster), SEEK_SET);
        do{
                fread(ptr, sizeof(int), 1, imgFile);
                ctr++;
                

        }while(*ptr != 0);
        fseek(imgFile, ftell(imgFile)-4, SEEK_SET);
        printf("ftell = %d\n", ftell(imgFile));
        fwrite(&found, sizeof(unsigned int), 1, imgFile);
        fseek(imgFile, ClusterByteOffset(cluster), SEEK_SET);
        return ctr;
}

void OpenCmd(char* token1, char* token2){
        int next_cluster = CurrentDirectory;

        DIR entry;
        int i;

	if(NumOpenFiles == 10){
		printf("Maximum files opened\n");
		return;
	}  //might need to be an else and put the while inside of it

        while(next_cluster < 0x0FFFFFF8) {
        	fseek(imgFile, ClusterByteOffset(next_cluster), SEEK_SET);
        	for(i = 0; i < (BytesPerCluster /32); i++) {
                	fread(&entry, sizeof(DIR), 1, imgFile);
                	if(entry.DIR_Attr == 0x20){
				//if(!strcmp(OpenedFiles[j].fileName, token1)){
				//	printf("%s is already open\n", token1);
				//	return;
				//}
				for(int k = 0; k < 10; k++){
					if(!strcmp(OpenedFiles[k].fileName, token1) && OpenedFiles[k].openedMethod != 0){
                                        	printf("%s is already open\n", token1);
                                        	return;
                                	}
					if(OpenedFiles[k].openedMethod == 0){  //only one that wa orinally i
						trim(entry.DIR_Name);
						if(!strcmp(token1, entry.DIR_Name)){
							strcpy(OpenedFiles[k].fileName, entry.DIR_Name);
							OpenedFiles[k].currentFilePosition = getHiLoClus(entry.DIR_FstClusHi, entry.DIR_FstClusLo);
							if(OpenedFiles[k].currentFilePosition == 0){
								OpenedFiles[k].currentFilePosition = BootBlock.BPB_RootClus;
								//OpenedFiles[j].currentFilePositionOffset = cwd offset idk whta value that is
							}
							OpenedFiles[k].currentFilePositionOffset = ClusterByteOffset(OpenedFiles[k].currentFilePosition);
							if(!strcmp(token2, "-r")){
								OpenedFiles[k].openedMethod = 1;
							}
							if(!strcmp(token2, "-w")){
                                        	        	OpenedFiles[k].openedMethod = 2;
                                        		}
							if(!strcmp(token2, "-rw")){
                                                		OpenedFiles[k].openedMethod = 3;
                                        		}
							if(!strcmp(token2, "-wr")){
                                                		OpenedFiles[k].openedMethod = 4;
                                        		}
							OpenedFiles[k].fileSize = entry.DIR_FileSize;
							OpenedFiles[k].offset = 0;
							OpenedFiles[k].filePath = cwd.path;
							printf("opened %s\n", OpenedFiles[k].fileName);
							NumOpenFiles += 1;
							break;
						}
					}
				}
			}
        	}
        	next_cluster = GetNextCluster(next_cluster);
	}
	return;
}

void closeCmd(char* token){
        int next_cluster = CurrentDirectory;


        DIR entry;
        int i;
        while(next_cluster < 0x0FFFFFF8) {
          fseek(imgFile, ClusterByteOffset(next_cluster), SEEK_SET);
          for(i = 0; i < (BytesPerCluster /32); i++) {
                fread(&entry, sizeof(DIR), 1, imgFile);
		for(int j = 0; j < 10; j++){
			if(!strcmp(token, OpenedFiles[j].fileName)){
				if(OpenedFiles[j].openedMethod == 0){
					printf("file is not open \n");
					return;
				}else{
					OpenedFiles[j].openedMethod = 0;
					printf("%s closed\n", OpenedFiles[j].fileName);
					NumOpenFiles--;
					return;
				}
			}
		}
          }
          next_cluster = GetNextCluster(next_cluster);
        }

	//needs to be a check to see if the file is in the curent working dircetory 
}

void lsofCmd(){
	if(NumOpenFiles == 0){
		printf("No files are opened right now\n");
		return;
	}else{
		printf("INDEX NAME      MODE      OFFSET      PATH\n");
		for(int i = 0; i < 10; i++){
			if(OpenedFiles[i].openedMethod != 0){
				if(OpenedFiles[i].openedMethod == 1){					//if file was opened in read
					printf("%-6d%-10s%-10s%-12d%-s\n", i, OpenedFiles[i].fileName, "r", 
						OpenedFiles[i].offset, OpenedFiles[i].filePath);
				}
				if(OpenedFiles[i].openedMethod == 2){					//if file was oopened in write
					printf("%-6d%-10s%-10s%-12d%-s\n", i, OpenedFiles[i].fileName, "w", 
						OpenedFiles[i].offset, OpenedFiles[i].filePath);
				}
				if(OpenedFiles[i].openedMethod == 3){					//if file opened in read write
					printf("%-6d%-10s%-10s%-12d%-s\n", i, OpenedFiles[i].fileName, "rw", 
						OpenedFiles[i].offset, OpenedFiles[i].filePath);
				}
				if(OpenedFiles[i].openedMethod == 4){					//if file opened in write read
					printf("%-6d%-10s%-10s%-12d%-s\n", i, OpenedFiles[i].fileName, "wr", 
						OpenedFiles[i].offset, OpenedFiles[i].filePath);
				}
			}
		}
	}
}

void sizeCmd(char* token){
        int next_cluster = CurrentDirectory;

        DIR entry;
        int i;
        while(next_cluster < 0x0FFFFFF8) {
          fseek(imgFile, ClusterByteOffset(next_cluster), SEEK_SET);
          for(i = 0; i < (BytesPerCluster /32); i++) {
                fread(&entry, sizeof(DIR), 1, imgFile);
		trim(entry.DIR_Name);
		if(!strcmp(entry.DIR_Name, token)){
			if(entry.DIR_Attr == 0x20){ 
				//&& !strcmp(token, entry.DIR_Name)){
				printf("%d %s\n", entry.DIR_FileSize, entry.DIR_Name);
				return;
			}
		}
          }
          next_cluster = GetNextCluster(next_cluster);
        }

	//printf("%s is a directory\n", entry.DIR_Name);

	//need error checking for if file not found unsure of how to do that bc line above doesnt work
}

void lseekCmd(char* token, unsigned int o){
        int next_cluster = CurrentDirectory;

        DIR entry;
        int i;
        while(next_cluster < 0x0FFFFFF8) {
          fseek(imgFile, ClusterByteOffset(next_cluster), SEEK_SET);
          for(i = 0; i < (BytesPerCluster /32); i++) {
                fread(&entry, sizeof(DIR), 1, imgFile);
		trim(entry.DIR_Name);
		if(!strcmp(entry.DIR_Name, token)){
			for(int i = 0; i < 10; i++){
				if(!strcmp(entry.DIR_Name, OpenedFiles[i].fileName) && OpenedFiles[i].openedMethod == 0){
					return;
				}
				if(!strcmp(entry.DIR_Name, OpenedFiles[i].fileName) && OpenedFiles[i].openedMethod != 0){
					if(o > entry.DIR_FileSize){
						printf("Offset is larger than the file size\n");
					}else{
						OpenedFiles[i].offset = o;
					}
					return;
				}
			}
			printf("%s must be opened to lseek\n", entry.DIR_Name);
			return;
		}
          }
          next_cluster = GetNextCluster(next_cluster);
        }

	//need error checking the same as the other functions
}

void readCmd(char* token, unsigned int token2){
        int next_cluster = CurrentDirectory;


        DIR entry;
        int i;
        while(next_cluster < 0x0FFFFFF8) {
                fseek(imgFile, ClusterByteOffset(next_cluster), SEEK_SET);
                for(i = 0; i < (BytesPerCluster /32); i++) {
                        fread(&entry, sizeof(DIR), 1, imgFile);
                        trim(entry.DIR_Name);
                        if(!strcmp(entry.DIR_Name, token)){
                                for(int i = 0; i < 10; i++){
                                        if(!strcmp(entry.DIR_Name, OpenedFiles[i].fileName)){
                                                if(OpenedFiles[i].openedMethod == 2){
                                                        printf("File opened in write (-w) mode. must be opened in -r -rw or -wr\n");
                                                        break;
                                                }
                                                else if(OpenedFiles[i].openedMethod == 0){
                                                        printf("File not opened. must be opened in -r -rw or -wr mode\n");
                                                        break;
                                                }

						if(token2 >= OpenedFiles[i].fileSize){

						fseek(imgFile, OpenedFiles[i].currentFilePositionOffset + OpenedFiles[i].offset, SEEK_SET);
						for(int j = 0; j < BytesPerCluster; j++){
							char byte = fgetc(imgFile);
							printf("%c", byte);
						}
						OpenedFiles[i].offset = OpenedFiles[i].fileSize;
						int back = BackToFat(OpenedFiles[i].currentFilePosition);
						while(back != 0){
							fseek(imgFile, ClusterByteOffset(back), SEEK_SET);
							for(int j = 0; j < BytesPerCluster; j++){
                                                        	char byte = fgetc(imgFile);
                                                        	printf("%c", byte);
								OpenedFiles[i].offset = OpenedFiles[i].offset + 1;
								printf("files offset: %d\n", OpenedFiles[i].offset);
                                                	}
							back = BackToFat(back);
						}
						return;
						}else{
							//boolean end = false;
							int counter = 0;
							while (counter < token2){
								fseek(imgFile, OpenedFiles[i].currentFilePositionOffset + OpenedFiles[i].offset, SEEK_SET);
                                                		for(int j = 0; j < BytesPerCluster; j++){
                                                        		char byte = fgetc(imgFile);
                                                        		printf("%c", byte);
                                                			counter++;
									OpenedFiles[i].offset = OpenedFiles[i].offset + 1;
                                                                        if(!(counter < token2))
                                                                        {return;}
								}
                                                		int back = BackToFat(OpenedFiles[i].currentFilePosition);
                                                		while(back != 0){
                                                        		fseek(imgFile, ClusterByteOffset(back), SEEK_SET);
                                                        		for(int j = 0; j < BytesPerCluster; j++){
										char byte = fgetc(imgFile);
                                                                		printf("%c", byte);
                                                                                counter++;
                                                                                if(!(counter < token2))
                                                                                {
											OpenedFiles[i].offset = counter;
											return;
										}
                                                        		}
                                                        		back = BackToFat(back);
                                                		}
								//OpenedFiles[i].offset = token2;
                                                		return;
							}
						}
                                        }
                                }
                                break;
                        }
                }
                next_cluster = GetNextCluster(next_cluster);
        }
}




int findFile(char* file){
        int next_cluster = CurrentDirectory;

        DIR entry;
        int i;
        while(next_cluster < 0x0FFFFFF8) {
          fseek(imgFile, ClusterByteOffset(next_cluster), SEEK_SET);
          for(i = 0; i < (BytesPerCluster /32); i++) {
                fread(&entry, sizeof(DIR), 1, imgFile);
		trim(entry.DIR_Name);
		if(!strcmp(entry.DIR_Name, file)){			//if file exists
			fseek(imgFile, ClusterByteOffset(CurrentDirectory), SEEK_SET);
			return 1;
		}
          }
          next_cluster = GetNextCluster(next_cluster);
        }								//if file is not in cwd

//	fseek(imgFile, ClusterByteOffset(CurrentDirectory), SEEK_SET);
        return -1;
}

void renameCmd(char * token1, char* token2){
        int next_cluster = CurrentDirectory;

	if(sizeof(token2) > 11){
		printf("name of file too large. Must be size 11 or less\n");
		return;
	}

        DIR entry;
        int i;
        while(next_cluster < 0x0FFFFFF8) {
        	fseek(imgFile, ClusterByteOffset(next_cluster), SEEK_SET);
        	for(i = 0; i < (BytesPerCluster /32); i++) {
			int currAddress = ftell(imgFile);
                	fread(&entry, sizeof(DIR), 1, imgFile);
			trim(entry.DIR_Name);
			if(!strcmp(entry.DIR_Name, token2)){
				printf("file %s already exists.\n", token2);
				return;
			}else{
				if(!strcmp(entry.DIR_Name, token1)){
					//check if file is open
					for(int j = 0; j < 10; j++){
						if(!strcmp(OpenedFiles[j].fileName, entry.DIR_Name)){
							if(OpenedFiles[j].openedMethod != 0){
								printf("file must be closed to rename\n");
								return;
							}
						}
					}
					//if the file is not opened
					fseek(imgFile, currAddress, SEEK_SET);
					printf("directory name: %s\n", entry.DIR_Name);
					strcpy(entry.DIR_Name, token2);
					printf("directory name: %s\n", entry.DIR_Name);
					fwrite(&entry, sizeof(DIR), 1, imgFile);
					return;
				}
			}
          	}
          	next_cluster = GetNextCluster(next_cluster);
        }
}

void writeCmd(char* token1, char* token2){
        int next_cluster = CurrentDirectory;

        DIR entry;
        int i;
        while(next_cluster < 0x0FFFFFF8) {
                fseek(imgFile, ClusterByteOffset(next_cluster), SEEK_SET);
                for(i = 0; i < (BytesPerCluster /32); i++) {
                        fread(&entry, sizeof(DIR), 1, imgFile);
                        trim(entry.DIR_Name);
                        if(entry.DIR_Attr == 0x20){
                                if(!strcmp(entry.DIR_Name, token1)){
                                        for(int j = 0; j < 10; j++){
                                                if(!strcmp(OpenedFiles[j].fileName, entry.DIR_Name)){
                                                        if(OpenedFiles[j].openedMethod == 1){
                                                                printf("file %s cannot be written to in read (r) mode\n", OpenedFiles[j].fileName);
                                                                return;
                                                        }else if(OpenedFiles[j].openedMethod == 0){
                                                                printf("file %s must be opened to write to\n", OpenedFiles[j].fileName);
                                                                return;
                                                        }else{
                                                                printf("writing to %s in -w mode\n", OpenedFiles[j].fileName);
                                                                return;
                                                        }
                                                }
                                        }
                                        printf("file has not been opened\n");
                                        return;
                                }
                        }
                }
                next_cluster = GetNextCluster(next_cluster);
        }
        printf("file %s is not in the current directory\n", token1);
}

void Info(long offset){
        fseek(imgFile, offset, SEEK_SET);
        fread(&fsi, sizeof(FSInfo), 1, imgFile);

	printf("Position of root cluster: %d\n", BootBlock.BPB_RootClus);
        printf("Bytes Per Sector: %d\n", BootBlock.BPB_BytsPerSec);
        printf("Sectors Per Cluster: %d\n", BootBlock.BPB_SecPerClus);
        printf("Total clusters in Data Region: %d\n", BootBlock.BPB_TotalSec32);
        printf("# of enteries in one FAT: %d\n", ((BootBlock.BPB_FATSz32 * BootBlock.BPB_BytsPerSec) / 4));
        printf("Size of Image (bytes): %d\n", BootBlock.BPB_TotalSec32 * BootBlock.BPB_BytsPerSec);
        printf("Root Cluster: %d\n", BootBlock.BPB_RootClus);
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

void mkdir(char* token)
{
        int next_cluster = CurrentDirectory;
        DIR newEntry;
        DIR entry;
        DIR parent;
        DIR CHILD;
        fseek(imgFile, ClusterByteOffset(CurrentDirectory), SEEK_SET);
        fread(&parent, sizeof(DIR), 1, imgFile);
        char fileName[11];
        for (int i =0; i < 11; i++)
        {
                newEntry.DIR_Name[i] = token[i];
        }
        newEntry.DIR_Attr = 0x10;
        newEntry.DIR_NTRes =0;
        newEntry.DIR_CrtTimeTenth=0;
        newEntry.DIR_CrtTime=0;
        newEntry.DIR_CrtDate=0;
        newEntry.DIR_LstAccDate=0;
        newEntry.DIR_FstClusHi=(allocateClus(CurrentDirectory)>>16); 
        newEntry.DIR_WrtTime=0;
        newEntry.DIR_WrtDate=0;
        newEntry.DIR_FstClusLo = (allocateClus(CurrentDirectory)& 0xffff);
        newEntry.DIR_FileSize = 0; 
        char stop = 'f';
        while (next_cluster < 0x0FFFFFF8)
        {
                fseek(imgFile, ClusterByteOffset(next_cluster), SEEK_SET);
                for(int i = 0; i < (BytesPerCluster /32); i++) {
                        int newDirLoc = ftell(imgFile);
                        fread(&entry, sizeof(DIR), 1, imgFile);
                        if(entry.DIR_Attr == 0x0F){
                                continue;    
                        }
                        if(entry.DIR_Name[0] == 0 || entry.DIR_Name[0] == 0xE5) //5e or e5?
                        {
                                fseek(imgFile, newDirLoc, SEEK_SET);
                                fwrite(&newEntry, sizeof(DIR), 1, imgFile);
                                CurrentDirectory = cdCmd(CurrentDirectory, token);
                                strcpy(parent.DIR_Name, "..");
                                fseek(imgFile, ClusterByteOffset(CurrentDirectory), SEEK_SET);
                                fwrite(&parent, sizeof(DIR), 1, imgFile);
                                CurrentDirectory = cdCmd(CurrentDirectory,"..");
                                stop = 't';
                                break;
                        }
                
                }
                if(stop == 't')
                {
                           break;
                }
                next_cluster = GetNextCluster(next_cluster);
        }
}

// void read_sector(FILE* imgFile, unsigned int sector_number, void* buffer)
// {
//         //printf("location = %u\n", sector_number);
//         fseek(imgFile, sector_number, SEEK_SET);
//         fread(buffer, BootBlock.BPB_BytsPerSec, 1, imgFile);


// }

// void dirEntryInit(FILE* imgFile, unsigned int sector_number, void* buffer)
// {

//         fseek(imgFile, sector_number, SEEK_SET);
//         fread(&directoryEnt, BootBlock.BPB_BytsPerSec, 1, imgFile);
//         

// }

// int GetCluster(unsigned short high, unsigned short low)
// {
//         Cluster = (high << 8) | low; 
//         printf("hi: %d\nlo: %d\n", high, low);
//         return cluster;
// }


int lsCmd(int Directory)
{
        int next_cluster = CurrentDirectory;


//        printf("Postion: %d\n", ftell(imgFile));
        DIR entry;
        int i;
        while(next_cluster < 0x0FFFFFF8) {
          fseek(imgFile, ClusterByteOffset(next_cluster), SEEK_SET);
          for(i = 0; i < (BytesPerCluster /32); i++) {
                fread(&entry, sizeof(DIR), 1, imgFile);
                //printf("name = %d\n", entry.DIR_Name[0]);

                if(entry.DIR_Attr == 0x0F){
                    continue;    
                }
                if(entry.DIR_Name[0] == 0 || entry.DIR_Name[0] == 0xE5) //5e or e5?
                {
                        continue;
                }
                printf("%s\n", entry.DIR_Name);
          }
          next_cluster = GetNextCluster(next_cluster);
        }
        return 1;
}

int cdCmd(int CurrentDirectory, char* token)
{
        int next_cluster = CurrentDirectory;

        fseek(imgFile, ClusterByteOffset(next_cluster), SEEK_SET);

        DIR entry;
        int i;
        while(next_cluster < 0x0FFFFFF8) {
                fseek(imgFile, ClusterByteOffset(next_cluster), SEEK_SET);
        for(i = 0; i < (BytesPerCluster /32); i++) {
                fread(&entry, sizeof(DIR), 1, imgFile);
                if(entry.DIR_Attr == 0x0F){                             					//if longfgile ignore
                    continue;
                }
                //printf("name = %d\n", entry.DIR_Name[0]);
                if(entry.DIR_Name[0] == 0 || entry.DIR_Name[0] == 0xE5) 					//if deleted entry ignore
                {
                        continue;
                }
                //CurrentDirectory = FatEntryOffset(getHiLoClus(entry.DIR_FstClusHi, entry.DIR_FstClusLo));
                trim(entry.DIR_Name);
                if(!strcmp(token, entry.DIR_Name)){
                        if(entry.DIR_Attr == 0x10){
                                CurrentDirectory = getHiLoClus(entry.DIR_FstClusHi, entry.DIR_FstClusLo);
                                if(CurrentDirectory == 0)
                                {
                                        CurrentDirectory = BootBlock.BPB_RootClus;
                                }
				add_to_path(token);
                                //CurrentDirectory = FatEntryOffset(getHiLoClus(entry.DIR_FstClusHi, entry.DIR_FstClusLo));
                                break;
                        }else{
                                printf("%s not a directory\n", token);
                        }
                }
        }
          next_cluster = GetNextCluster(next_cluster);
        }

        return CurrentDirectory;
        //offset2 = FirstDataSector * BootBlock.BPB_BytsPerSec + 32;
        //CurrentDirectory += 32;
        // do 
        // {
        //         // fseek(imgFile, CurrentDirectory, SEEK_SET);
        //         // fread(buffer, sizeof(char), 1, imgFile);
        //         dirEntryInit(imgFile,CurrentDirectory, buffer);
                
        //         printf("searching for %s\n", token);
        //         printf("location = %d\n", CurrentDirectory);
//                printf("token = %s , dir name = %s\n", token, directoryEnt.DIR_Name);
                
//                 if(!strcmp(token, directoryEnt.DIR_Name))
//                 {
//                         printf("%s found\n", token);
//                         //read_sector(imgFile, CurrentDirectory+11, buffer);
//                         //to get long file or shortfile -21
//                         if(directoryEnt.DIR_Attr == 0x10)
//                         {
//                                  printf("is directory\n");
                                
                                
//                                 short high = (directoryEnt.DIR_FstClusHi << 8);
//                                 short low = (directoryEnt.DIR_FstClusLo);
//                                 unsigned int concat = high | low;

//                                 // short low;
//                                 // read_sector(imgFile, CurrentDirectory+20, buffer);
//                                 // high = buffer[0] + buffer[1];
//                                 // printf("buffer = %s\n", buffer);
//                                 // printf("high = %d\n", high);
//                                 // read_sector(imgFile, CurrentDirectory+26, buffer);
//                                 // low = (buffer[0]) | (buffer[1]);
//                                 // printf("bufferLow = %s\n", buffer);
//                                 // printf("low = %d\n", low);
//                                 int newDirectory = (FirstDataSector + ((concat - 2) * BootBlock.BPB_SecPerClus)) * BootBlock.BPB_BytsPerSec;
//                                 printf("newDirectory: %d\n", newDirectory);
//                                 printf("currentDirectroy: %d\n", CurrentDirectory);
//                                 printf("high = %d\nLow = %d\n", high, low);
//                                 printf("concat = %d\n", concat);
//                                 CurrentDirectory = newDirectory;
// //                                Cluster = concat;
//                         }
//                         break;

//                 }

//                 // printf("location = %d\n", CurrentDirectory);
//                 CurrentDirectory += 64;
//         } while(directoryEnt.DIR_Name[0] != 0);
}



void rmDir(char * token)
{
       //printf("current Directory = %d\n", ClusterByteOffset(CurrentDirectory));
        CurrentDirectory = cdCmd(CurrentDirectory, token);
       //printf("current Directory = %d\n", ClusterByteOffset(CurrentDirectory));
        int next_cluster = CurrentDirectory;


//        printf("Postion: %d\n", ftell(imgFile));
        DIR entry;
        int i;
        char trueFalse = 'f';
        while(next_cluster < 0x0FFFFFF8) {
          fseek(imgFile, ClusterByteOffset(next_cluster), SEEK_SET);
          for(i = 0; i < (BytesPerCluster /32); i++) {
                fread(&entry, sizeof(DIR), 1, imgFile);
                if(entry.DIR_Attr == 0x0F){
                    continue;    
                }
                if(entry.DIR_Name[0] == 0 || entry.DIR_Name[0] == 0xE5) //5E or E5
                {
                        continue;
                }
                //printf("%c\n", entry.DIR_Name[0]);
                if(entry.DIR_Name[0] != 0x2E)
                {
                        trueFalse = 't';
                        break;
                }

          }
          next_cluster = GetNextCluster(next_cluster);
        }
        if(trueFalse == 'f')
        {
                
                fseek(imgFile, FatEntryOffset(CurrentDirectory), SEEK_SET);
                //printf("fat location: %d\n", FatEntryOffset(CurrentDirectory));
                int zeroVar;
                int* zero = &zeroVar;
                *zero = 0x00000000;
                //printf("location = %d\n", ftell(imgFile));
                fseek(imgFile,  ClusterByteOffset(CurrentDirectory), SEEK_SET);
                //printf("location = %d\n", ClusterByteOffset(CurrentDirectory));
                CurrentDirectory = cdCmd(CurrentDirectory, "..");
                int previousDirectory = CurrentDirectory;
                fseek(imgFile, ClusterByteOffset(CurrentDirectory), SEEK_SET);
                //printf("current location = %d\n", ClusterByteOffset(CurrentDirectory));
                while(CurrentDirectory < 0x0FFFFFF8) {
                fseek(imgFile, ClusterByteOffset(CurrentDirectory), SEEK_SET);
                //fseek(imgFile, FatEntryOffset(CurrentDirectory), SEEK_SET);
                //printf("ftell = %d\n", ftell(imgFile));
                //fseek(imgFile, ClusterByteOffset(CurrentDirectory), SEEK_SET);

                for(i = 0; i < (BytesPerCluster /32); i++) {
                        int NameAddress = ftell(imgFile);
                        //printf("ftell = %d\n", ftell(imgFile));
                        fread(&entry, sizeof(DIR), 1, imgFile);
                        if(entry.DIR_Attr == 0x0F){                             					//if longfgile ignore
                                continue;
                        }
                        if(entry.DIR_Name[0] == 0 || entry.DIR_Name[0] == 0xE5) 					//if deleted entry ignore
                        {
                                continue;
                        }
                        // if(GetNextCluster(CurrentDirectory) < 0x0FFFFFF8)
                        // {
                        //         entry.DIR_Name[0] = 0x5E;
                        //         printf("%d\n", entry.DIR_Name[0]);
                        // }
                        // else{
                        //         entry.DIR_Name[0] = 0x00;
                        //         printf("%d\n", entry.DIR_Name[0]);
                        // }
                        trim(entry.DIR_Name);
                        //printf("token = %s", token);
                        //printf("DirName = %s", entry.DIR_Name);
                        if(!strcmp(token, entry.DIR_Name)){
                                //printf("directory found");
                                //printf("location = %d\n", ClusterByteOffset(CurrentDirectory));
                                if(entry.DIR_Attr == 0x10){
                                        // CurrentDirectory = getHiLoClus(entry.DIR_FstClusHi, entry.DIR_FstClusLo);
                                        // if(CurrentDirectory == 0)
                                        // {
                                        //         CurrentDirectory = BootBlock.BPB_RootClus;
                                        // }
                                        //cdCmd(CurrentDirectory, token);
                                        //printf("location = %d\n", ClusterByteOffset(CurrentDirectory));
                                        char deleteVar;
                                        char* delete = &deleteVar;
                                        *delete = 0xE5;
                                        if(GetNextCluster(CurrentDirectory) < 0x0FFFFFF8)
                                        {
                                                //printf("DirName = %s\n", entry.DIR_Name);
                                                fseek(imgFile, NameAddress, SEEK_SET);

                                                //printf("location = %d\n", ftell(imgFile));
                                                entry.DIR_Name[0] = 0xE5;
                                                fwrite(&entry, sizeof(DIR), 1, imgFile);
                                                //printf("%d\n", entry.DIR_Name[0]);
                                                //printf("DirName = %s\n", entry.DIR_Name);

                                        }
                                        else{
                                                fseek(imgFile,NameAddress, SEEK_SET);
                                                entry.DIR_Name[0] = 0x00;
                                                
                                                fwrite(delete, sizeof(char), 1, imgFile);
                                                //printf("%d\n", entry.DIR_Name[0]);

                                        }
                                        break;
                                }
                                else{
                                }
            
                        }
                }
                CurrentDirectory = GetNextCluster(CurrentDirectory);
                }
                CurrentDirectory = previousDirectory;
        }
        else
        {
        CurrentDirectory = cdCmd(CurrentDirectory, "..");
        printf("directory not empty\n");
        }

        // int next_cluster = CurrentDirectory;

        // fseek(imgFile, ClusterByteOffset(next_cluster), SEEK_SET);

        // DIR entry;
        // int i;
        // //while(next_cluster <= 0x0FFFFFF8) {
        // for(i = 0; i < (BytesPerCluster /32); i++) {
        //         fread(&entry, sizeof(DIR), 1, imgFile);
        //         if(entry.DIR_Attr == 0x0F){                             					//if longfgile ignore
        //             continue;
        //         }
        //         if(entry.DIR_Name[0] == 0 || entry.DIR_Name[0] == 0xE5) 					//if deleted entry ignore
        //         {
        //                 continue;
        //         }
        //         //CurrentDirectory = FatEntryOffset(getHiLoClus(entry.DIR_FstClusHi, entry.DIR_FstClusLo));
        //         trim(entry.DIR_Name);
        //         if(!strcmp(token, entry.DIR_Name)){
        //                 if(entry.DIR_Attr == 0x10){
        //                         CurrentDirectory = getHiLoClus(entry.DIR_FstClusHi, entry.DIR_FstClusLo)
        //                         break;
        //                 }else{
        //                         printf("%s not a directory\n", token);
        //                 }
        //         }
        //}
//
  //      return CurrentDirectory;
}

void creat(char* filename) {
        // Check if file/directory already exists
        if (findFile(filename) == 1) 
        {
                printf("Error: File/Directory already exists\n");
                return;
        }
        // Create new file
        int next_cluster = CurrentDirectory;
        DIR newEntry;
        DIR entry;
        char fileName[11];
        for (int i =0; i < 11; i++)
        {
                newEntry.DIR_Name[i] = filename[i];
        }
        newEntry.DIR_Attr = 0x20;
        newEntry.DIR_NTRes =0;
        newEntry.DIR_CrtTimeTenth=0;
        newEntry.DIR_CrtTime=0;
        newEntry.DIR_CrtDate=0;
        newEntry.DIR_LstAccDate=0;
        newEntry.DIR_FstClusHi=0; 
        newEntry.DIR_WrtTime=0;
        newEntry.DIR_WrtDate=0;
        newEntry.DIR_FstClusLo = 0; 
        newEntry.DIR_FileSize = 0; 
        while (next_cluster < 0x0FFFFFF8)
        {
                fseek(imgFile, ClusterByteOffset(next_cluster), SEEK_SET);
                for(int i = 0; i < (BytesPerCluster /32); i++) 
                {
                        int newDirLoc = ftell(imgFile);
                        fread(&entry, sizeof(DIR), 1, imgFile);
                        if(entry.DIR_Attr == 0x0F)
                        {
                                continue;    
                        }
                        if(entry.DIR_Name[0] == 0 || entry.DIR_Name[0] == 0xE5) //5e or e5?
                        {
                                fseek(imgFile, newDirLoc, SEEK_SET);
                                fwrite(&newEntry, sizeof(DIR), 1, imgFile);
                                break;
                        }             
                }
                next_cluster = GetNextCluster(next_cluster);   
        }
}






// int findUnusedCluster(FILE *fp, int startCluster) {
//         int fatOffset = startCluster * 4;
//         int fatSector = BootBlock.BPB_RsvdSecCnt + (fatOffset / BootBlock.BPB_BytsPerSec);
//         int fatEntryOffset = fatOffset % BootBlock.BPB_BytsPerSec;

//         while (1) {
//                 fseek(fp, fatSector * BootBlock.BPB_BytsPerSec + fatEntryOffset, SEEK_SET);
//                 int value;
//                 fread(&value, 4, 1, fp);

//                 if (value == 0) {
//                 printf("found empty cluster\n");
//                 return startCluster;
//                 }
//                 printf("searching clusters\n");
//                 startCluster++;
//         }
// }



// void createDirectoryEntry(FILE *fp, char *name, int start_cluster) {
//     struct DirectoryEntry dir_entry = {0};
//     strncpy(dir_entry.DIR_Name, name, strlen(name));
//     dir_entry.DIR_Attr = 0x10;
//     dir_entry.DIR_FstClusHI = (start_cluster & 0xFFFF0000) >> 16;
//     dir_entry.DIR_FstClusLO = start_cluster & 0x0000FFFF;

//     // Need to write the new directory entry to the parent directory's cluster
//     // by calculating the offset of the new entry within the cluster
//     // and write the entry to that location - Nick
// }

// void updateFatEntry(FILE *fp, int cluster, int value) {
//     int fatOffset = cluster * 4;
//     int fatSector = BootBlock.BPB_RsvdSecCnt + (fatOffset / BootBlock.BPB_BytsPerSec);
//     int fatEntryOffset = fatOffset % BootBlock.BPB_BytsPerSec;

//     fseek(fp, fatSector * BootBlock.BPB_BytsPerSec + fatEntryOffset, SEEK_SET);
//     fwrite(&value, 4, 1, fp);
// }




    /*
                CurrentDirectory = cdCmd(CurrentDirectory, "..");
                printf("location = %d\n", ClusterByteOffset(CurrentDirectory));

                for(i = 0; i < (BytesPerCluster /32); i++) {
                fread(&entry, sizeof(DIR), 1, imgFile);
                if(entry.DIR_Attr == 0x0F){                             					//if longfgile ignore
                    continue;
                }
                
                //printf("location = %d\n", ClusterByteOffset(CurrentDirectory));

                if(entry.DIR_Name[0] == 0 || entry.DIR_Name[0] == 0xE5) 					//if deleted entry ignore
                {
                        continue;
                }
                */
                //printf("location = %d\n", ClusterByteOffset(CurrentDirectory));

                //CurrentDirectory = FatEntryOffset(getHiLoClus(entry.DIR_FstClusHi, entry.DIR_FstClusLo));
                /*
                trim(entry.DIR_Name);
                printf("token = %s", token);
                printf("DirName = %s", entry.DIR_Name);
                if(!strcmp(token, entry.DIR_Name)){
                printf("location = %d\n", ClusterByteOffset(CurrentDirectory));
                        if(entry.DIR_Attr == 0x10){
                                CurrentDirectory = getHiLoClus(entry.DIR_FstClusHi, entry.DIR_FstClusLo);
                                if(CurrentDirectory == 0)
                                {
                                        CurrentDirectory = BootBlock.BPB_RootClus;
                                }
                                fseek(imgFile, FatEntryOffset(CurrentDirectory), SEEK_SET);
                                int* zero;
                                *zero = 0x00000000;
                                fwrite(zero, sizeof(int), 1, imgFile);
                                fseek(imgFile,  ClusterByteOffset(CurrentDirectory), SEEK_SET);
                                //printf("location = %d\n", ClusterByteOffset(CurrentDirectory));
                                if(GetNextCluster(CurrentDirectory) < 0x0FFFFFF8)
                                {
                                        entry.DIR_Name[0] = 0x5E;
                                        printf("%d\n", entry.DIR_Name[0]);
                                }
                                else{
                                        entry.DIR_Name[0] = 0x00;
                                        printf("%d\n", entry.DIR_Name[0]);

                                }

                                break;
                        }else{
                        }
                }
                */
