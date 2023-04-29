// starter file provided by operating systems ta's
// include libraries

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
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

//struct to check on free space i believe. To my knowledge 
typedef struct __attribute__((packed)) {
        unsigned int FSI_LeadSig;
        unsigned char FSI_Reserved1[480];                      
        //FAT32 spec says its 480 byts so i think this is right
        unsigned int FSI_StrucSig;
        unsigned int FSI_Free_Count;
        unsigned int FSI_Nxt_Free;
        unsigned char FSI_Reserved2[12];                        
        //same here about the size
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
int cdCmd(char*);
void OpenCmd(char*, char*);
int getHiLoClus(unsigned short, unsigned short);
void closeCmd(char*);
void lsofCmd();
void sizeCmd(char*);
void lseekCmd(char*, unsigned int);
void Info(long);
void readCmd(char *, unsigned int);
void renameCmd(char *, char* );
void creat(char*);
int findFile(char*);
void mkdirCmd(char*);
void rmCmd(char*);
void rmdirCmd(char*);

int main(int argc, char * argv[]) {
        // error checking for number of arguments.
        // read and open argv[1] in file pointer.
        // obtain important information from bpb as well 
        //as initialize any important global variables

        imgFile = fopen(argv[1], "r+");
        fread(&BootBlock, sizeof(BPB_Block), 1, imgFile);

        BytesPerCluster = BootBlock.BPB_BytsPerSec * BootBlock.BPB_SecPerClus;
        FirstDataSector = BootBlock.BPB_RsvdSecCnt
        + (BootBlock.BPB_NumberofFATS* BootBlock.BPB_FATSz32);
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
                        CurrentDirectory = cdCmd(tokens->items[1]);
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
                        creat(tokens->items[1]);
                }
                else if(strcmp(tokens->items[0], "mkdir") == 0){
                        if(tokens->items[1]!= NULL){
                        mkdirCmd(tokens->items[1]);
                        }
                }
                else if(strcmp(tokens->items[0], "rm") == 0){
                        rmCmd(tokens->items[1]);
                }
                else if(strcmp(tokens->items[0], "rmdir") == 0){
                        if(tokens->items[1] != NULL)
                        {
                                rmdirCmd(tokens->items[1]);
                        }
                }
                else if(strcmp(tokens->items[0], "open") == 0){
			if(tokens->items[1] == NULL){
				printf("Enter valid filename\n");
			}else if(tokens->items[2] == NULL){
				printf("%s%s", "Enter a valid value for file operation.", 
                                " Either -r -w -rw or-wr\n");
			}else if(!strcmp(tokens->items[2], "-r") || !strcmp(tokens->items[2],
                        "-w") || !strcmp(tokens->items[2], "-rw") || !strcmp(tokens->items[2],
                        "-wr")){
				OpenCmd(tokens->items[1], tokens->items[2]);
			}else{
				printf("%s%s", "Enter a valid value for file operation.", 
                                " Either -r -w -rw or-wr\n");
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
			renameCmd(tokens->items[1], tokens->items[2]);
		}
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
        int clus = (FirstDataSector + ((cluster - 2) * BootBlock.BPB_SecPerClus)) * 
        BootBlock.BPB_BytsPerSec;
        return clus;
}

int BackToFat(int cluster){
	int next_cluster = (FatEntryOffset(cluster)) + 4;
	fseek(imgFile, next_cluster, SEEK_SET);
	int next_chain;
	fread(&next_chain, sizeof(int), 1, imgFile);
	if(next_chain < 0x0FFFFFF8){
		return next_chain;;
	}else{
		return 0;
	}
}

int allocateClus(int cluster)
{
        unsigned long i_pos = ftell(imgFile);
        unsigned int value;
        int ctr =0;
        unsigned int found = 0xFFFFFFFF;
        fseek(imgFile, FatEntryOffset(0), SEEK_SET);
        do{
                fread(&value, sizeof(unsigned int), 1, imgFile);
                ctr++;
        }while(value != 0);
        ctr -= 1;
        fseek(imgFile, ftell(imgFile)-4, SEEK_SET);
        fwrite(&found, sizeof(unsigned int), 1, imgFile);
        fseek(imgFile, i_pos, SEEK_SET);
        
        return ctr;
}

int extendFatChain(int cluster)
{
        int next_cluster = FatEntryOffset(cluster);
        fseek(imgFile, next_cluster, SEEK_SET);
        int end = -1;
        int newclust = allocateClus(cluster);

        while(next_cluster != -1)
        {
                fread(&next_cluster, sizeof(int),1, imgFile);
        }
        fseek(imgFile, ftell(imgFile)-4, SEEK_SET);
        fwrite(&newclust, sizeof(int), 1, imgFile);
        return newclust;
}



void OpenCmd(char* token1, char* token2){
        int next_cluster = CurrentDirectory;

        DIR entry;
        int i;

	if(NumOpenFiles == 10){
		printf("Maximum files opened\n");
		return;
	}  

        while(next_cluster < 0x0FFFFFF8) {
        	fseek(imgFile, ClusterByteOffset(next_cluster), SEEK_SET);
        	for(i = 0; i < (BytesPerCluster /32); i++) {
                	fread(&entry, sizeof(DIR), 1, imgFile);
                	if(entry.DIR_Attr == 0x20){
				for(int k = 0; k < 10; k++){
					if(!strcmp(OpenedFiles[k].fileName, token1) &&
                                        OpenedFiles[k].openedMethod != 0)
                                        {
                                        	printf("%s is already open\n", token1);
                                        	return;
                                	}
					if(OpenedFiles[k].openedMethod == 0){  
						trim(entry.DIR_Name);
						if(!strcmp(token1, entry.DIR_Name)){
							strcpy(OpenedFiles[k].fileName,
                                                        entry.DIR_Name);
							OpenedFiles[k].currentFilePosition =
                                                        getHiLoClus(entry.DIR_FstClusHi,
                                                        entry.DIR_FstClusLo);
                                                        if(OpenedFiles[k].currentFilePosition
                                                        == 0)
                                                        {
								OpenedFiles[k].currentFilePosition
                                                                = BootBlock.BPB_RootClus;
							}
							OpenedFiles[k].currentFilePositionOffset
                                                        = ClusterByteOffset(
                                                        OpenedFiles[k].currentFilePosition);
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
							OpenedFiles[k].fileSize =
                                                        entry.DIR_FileSize;
							OpenedFiles[k].offset = 0;
							OpenedFiles[k].filePath = cwd.path;
							printf("opened %s\n",
                                                        OpenedFiles[k].fileName);
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
}

void lsofCmd(){
	if(NumOpenFiles == 0){
		printf("No files are opened right now\n");
		return;
	}else{
		printf("INDEX NAME      MODE      OFFSET      PATH\n");
		for(int i = 0; i < 10; i++){
			if(OpenedFiles[i].openedMethod != 0){
				if(OpenedFiles[i].openedMethod == 1){
                                        //if file was opened in read
					printf("%-6d%-10s%-10s%-12d%-s\n", i,
                                        OpenedFiles[i].fileName, "r", 
					OpenedFiles[i].offset, OpenedFiles[i].filePath);
				}
				if(OpenedFiles[i].openedMethod == 2){
                                        //if file was oopened in write
					printf("%-6d%-10s%-10s%-12d%-s\n", i, 
                                        OpenedFiles[i].fileName, "w", 
					OpenedFiles[i].offset, OpenedFiles[i].filePath);
				}
				if(OpenedFiles[i].openedMethod == 3){
                                        //if file opened in read write
					printf("%-6d%-10s%-10s%-12d%-s\n", i, 
                                        OpenedFiles[i].fileName, "rw", 
					OpenedFiles[i].offset, OpenedFiles[i].filePath);
				}
				if(OpenedFiles[i].openedMethod == 4){
                                        //if file opened in write read
					printf("%-6d%-10s%-10s%-12d%-s\n", i, 
                                        OpenedFiles[i].fileName, "wr", 
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
				printf("%d %s\n", entry.DIR_FileSize, entry.DIR_Name);
				return;
			}
		}
          }
          next_cluster = GetNextCluster(next_cluster);
        }
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
				if(!strcmp(entry.DIR_Name, OpenedFiles[i].fileName)
                                && OpenedFiles[i].openedMethod == 0){
					return;
				}
				if(!strcmp(entry.DIR_Name, OpenedFiles[i].fileName)
                                && OpenedFiles[i].openedMethod != 0){
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
                                                        printf("%s%s%s", "File opened in",
                                                        "write (-w) mode.",
                                                        "Must be opened in -r -rw or -wr\n");
                                                        break;
                                                }
                                                else if(OpenedFiles[i].openedMethod == 0){
                                                        printf("%s%s","File not opened. " 
                                                        "must be opened in -r -rw or -wr mode\n");
                                                        break;
                                                }

						if(token2 >= OpenedFiles[i].fileSize){

						fseek(imgFile, 
                                                OpenedFiles[i].currentFilePositionOffset 
                                                + OpenedFiles[i].offset, SEEK_SET);
						for(int j = 0; j < BytesPerCluster; j++){
							char byte = fgetc(imgFile);
							printf("%c", byte);
						}
						OpenedFiles[i].offset = OpenedFiles[i].fileSize;
						int back = BackToFat(
                                                OpenedFiles[i].currentFilePosition);
						while(back != 0){
							fseek(imgFile, ClusterByteOffset(back),
                                                        SEEK_SET);
							for(int j = 0; j < BytesPerCluster; j++){
                                                        	char byte = fgetc(imgFile);
                                                        	printf("%c", byte);
								OpenedFiles[i].offset = 
                                                                OpenedFiles[i].offset + 1;
								printf("files offset: %d\n", 
                                                                OpenedFiles[i].offset);
                                                	}
							back = BackToFat(back);
						}
						return;
						}else{
							int counter = 0;
							while (counter < token2)
                                                        {
								fseek(imgFile, 
                                                                OpenedFiles[i].
                                                                currentFilePositionOffset
                                                                + OpenedFiles[i].offset,
                                                                SEEK_SET);
                                                		for(int j = 0; j <
                                                                BytesPerCluster; j++){
                                                        		char byte = 
                                                                        fgetc(imgFile);
                                                        		printf("%c", byte);
                                                			counter++;
									OpenedFiles[i].offset
                                                                        = OpenedFiles[i].offset
                                                                        + 1;
                                                                        if(!(counter < token2))
                                                                        {return;}
								}
                                                		int back = 
                                                                BackToFat(OpenedFiles[i].
                                                                currentFilePosition);
                                                		while(back != 0)
                                                                {
                                                        		fseek(imgFile,
                                                                        ClusterByteOffset(back),
                                                                        SEEK_SET);
                                                        		for(int j = 0; j
                                                                        < BytesPerCluster; j++){
										char byte 
                                                                                = fgetc(imgFile);
                                                                		printf("%c",
                                                                                byte);
                                                                                counter++;
                                                                                if(!(counter
                                                                                < token2))
                                                                                {
										   OpenedFiles[i].
                                                                                   offset
                                                                                   = counter;
										   return;
										}
                                                        		}
                                                        		back = BackToFat(back);
                                                		}
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
        }//if file is not in cwd
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
						if(!strcmp(OpenedFiles[j].fileName,
                                                entry.DIR_Name)){
							if(OpenedFiles[j].openedMethod != 0){
								printf("%s%s", "file must be", 
                                                                "closed to rename\n");
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
                             printf("file %s cannot be written to in read (r) mode\n",
                             OpenedFiles[j].fileName);
                             return;
                          }else if(OpenedFiles[j].openedMethod == 0){
                             printf("file %s must be opened to write to\n",
                             OpenedFiles[j].fileName);
                             return;
                             }else{
                                unsigned int sizePlusOffset
                                = OpenedFiles[j].offset + strlen(token2);
                                printf("size plus offset is %d\n", sizePlusOffset);
                                if(sizePlusOffset > OpenedFiles[j].fileSize){
                                   printf("%s%s", "filesize is less than size plus offset.",
                                   "must allocate new clusters \n");
   				   OpenedFiles[j].fileSize += strlen(token2);
				   printf("files position: %d\n", ftell(imgFile));
                                   fseek(imgFile, OpenedFiles[j].currentFilePositionOffset
                                   + OpenedFiles[j].offset, SEEK_SET);
                                   printf("files position: %d\n", ftell(imgFile));
				   char buffer[strlen(token2)];
				   strcpy(buffer, token2);
                                   for(int k = 0; k < strlen(token2); k++){
				      fwrite(&buffer[k], sizeof(char), 1, imgFile);
                                      OpenedFiles[j].offset++;
				      if(OpenedFiles[j].offset % BytesPerCluster == 0){
				         if(BackToFat(OpenedFiles[j].currentFilePosition) != 0){
					    fseek(imgFile, 
                                            BackToFat(OpenedFiles[j].currentFilePosition),
                                            SEEK_SET);
					 }else{
					    extendFatChain(OpenedFiles[j].currentFilePosition);
					    fseek(imgFile, 
                                            BackToFat(OpenedFiles[j].currentFilePosition),
                                            SEEK_SET);
					 }
				      }
        			  }
                                  }else{
                                     fseek(imgFile, OpenedFiles[j].currentFilePositionOffset
                                     + OpenedFiles[j].offset, SEEK_SET);
                                     char buffer[strlen(token2)];
                                     strcpy(buffer, token2);
                                     printf("buffer: %s\n", buffer);
                                     printf("length of token: %d\n", strlen(token2));
                                     printf("len of buffer: %d\n", strlen(buffer));
                                     fwrite(buffer, sizeof(char), strlen(token2), imgFile);
                                     OpenedFiles[j].offset += strlen(token2);
                                     }
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
        printf("# of enteries in one FAT: %d\n", ((BootBlock.BPB_FATSz32
        * BootBlock.BPB_BytsPerSec) / 4));
        printf("Size of Image (bytes): %d\n", BootBlock.BPB_TotalSec32
        * BootBlock.BPB_BytsPerSec);
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

void mkdirCmd(char * token)
{
        int next_cluster = CurrentDirectory;
        int thisDirectory =CurrentDirectory;
        DIR newEntry;
        DIR entry;
        char fileName[11];
        for (int i =0; i < 11; i++)
        {
                newEntry.DIR_Name[i] = token[i];
        }
        int loc= allocateClus(CurrentDirectory);
        newEntry.DIR_Attr = 0x10;
        newEntry.DIR_NTRes =0;
        newEntry.DIR_CrtTimeTenth=0;
        newEntry.DIR_CrtTime=0;
        newEntry.DIR_CrtDate=0;
        newEntry.DIR_LstAccDate=0;
        newEntry.DIR_FstClusHi=(loc>>16); 
        newEntry.DIR_WrtTime=0;
        newEntry.DIR_WrtDate=0;
        newEntry.DIR_FstClusLo = (loc& 0xffff);
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
                        if(entry.DIR_Name[0] == 0 || entry.DIR_Name[0] == 0xE5)
                        {
                                fseek(imgFile, newDirLoc, SEEK_SET);
                                fwrite(&newEntry, sizeof(DIR), 1, imgFile);
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
        fseek(imgFile, ClusterByteOffset(loc), SEEK_SET);
        DIR dot;
        dot.DIR_Attr = 0x10;
        dot.DIR_NTRes =0;
        dot.DIR_CrtTimeTenth=0;
        dot.DIR_CrtTime=0;
        dot.DIR_CrtDate=0;
        dot.DIR_LstAccDate=0;
        dot.DIR_FstClusHi=(loc >>16); 
        dot.DIR_WrtTime=0;
        dot.DIR_WrtDate=0;
        dot.DIR_FstClusLo = (loc & 0xffff);
        dot.DIR_FileSize = 0; 
        strcpy(dot.DIR_Name, ".");
        fwrite(&dot, sizeof(DIR), 1, imgFile);
        DIR dotdot;
        dotdot.DIR_Attr = 0x10;
        dotdot.DIR_NTRes =0;
        dotdot.DIR_CrtTimeTenth=0;
        dotdot.DIR_CrtTime=0;
        dotdot.DIR_CrtDate=0;
        dotdot.DIR_LstAccDate=0;
        dotdot.DIR_FstClusHi=(CurrentDirectory >>16); 
        dotdot.DIR_WrtTime=0;
        dotdot.DIR_WrtDate=0;
        dotdot.DIR_FstClusLo = (CurrentDirectory & 0xffff);
        dotdot.DIR_FileSize = 0; 
        strcpy(dotdot.DIR_Name, "..");
        fwrite(&dotdot, sizeof(DIR), 1, imgFile);
}

int lsCmd(int Directory)
{
        int next_cluster = CurrentDirectory;
        DIR entry;
        int i;
        while(next_cluster < 0x0FFFFFF8) {
          fseek(imgFile, ClusterByteOffset(next_cluster), SEEK_SET);
          for(i = 0; i < (BytesPerCluster /32); i++) {
                fread(&entry, sizeof(DIR), 1, imgFile);

                if(entry.DIR_Attr == 0x0F){
                    continue;    
                }
                if(entry.DIR_Name[0] == 0 || entry.DIR_Name[0] == 0xE5) 
                {
                        continue;
                }
                printf("%s\n", entry.DIR_Name);
          }
          next_cluster = GetNextCluster(next_cluster);
        }
        return 1;
}

int cdCmd(char* token)
{
        int next_cluster = CurrentDirectory;

        fseek(imgFile, ClusterByteOffset(next_cluster), SEEK_SET);

        DIR entry;
        int i;
        while(next_cluster < 0x0FFFFFF8) {
                fseek(imgFile, ClusterByteOffset(next_cluster), SEEK_SET);
        for(i = 0; i < (BytesPerCluster /32); i++) {
                fread(&entry, sizeof(DIR), 1, imgFile);
                if(entry.DIR_Attr == 0x0F){                             					
                //if longfgile ignore
                    continue;
                }
                if(entry.DIR_Name[0] == 0 || entry.DIR_Name[0] == 0xE5) //if deleted entry ignore
                {
                        continue;
                }
                trim(entry.DIR_Name);
                if(!strcmp(token, entry.DIR_Name)){
                        if(entry.DIR_Attr == 0x10){
                                CurrentDirectory
                                = getHiLoClus(entry.DIR_FstClusHi, entry.DIR_FstClusLo);
                                if(CurrentDirectory == 0)
                                {
                                        CurrentDirectory = BootBlock.BPB_RootClus;
                                }
				add_to_path(token);
                                break;
                        }else{
                                printf("%s not a directory\n", token);
                        }
                }
        }
          next_cluster = GetNextCluster(next_cluster);
        }

        return CurrentDirectory;
}


void Deallocate(cluster)
{
        int temp =0;
        int zero = 0;
        int location=0;
        fseek(imgFile, FatEntryOffset(cluster), SEEK_SET);
        do{
        location = ftell(imgFile);
        fread(&temp, sizeof(int), 1, imgFile);
        fseek(imgFile, location, SEEK_SET);
        fwrite(&zero, sizeof(int), 1, imgFile);
        fseek(imgFile, FatEntryOffset(temp), SEEK_SET);
        }while(temp < 0x0FFFFFF8);
}

int empty(cluster)
{
        int empty = 0;
        fseek(imgFile, ClusterByteOffset(cluster),SEEK_SET);
        DIR entry;
        for(int i = 0; i < (BytesPerCluster/32); i++)
        {
                fread(&entry, sizeof(DIR), 1, imgFile);
                trim(entry.DIR_Name);
                if(entry.DIR_Name[0] != '.' && entry.DIR_Name[0] != 0xE5
                && entry.DIR_Name[0] != 0)
                {
                        printf("directory not empty\n");
                        return empty;
                }
                
        }
        empty = 1;
        return empty;

}

void rmdirCmd(char* token)
{
        if(!strcmp(token, "..") || !strcmp(token, "."))
        {
                printf("cannot remove . or ..\n");
                return;
        }
        DIR entry;
        int next_cluster = CurrentDirectory;
        while(next_cluster < 0x0FFFFFF8) {
                fseek(imgFile, ClusterByteOffset(next_cluster), SEEK_SET);
                for(int i = 0; i < (BytesPerCluster /32); i++) {
                        int NameAddress = ftell(imgFile);
                        fread(&entry, sizeof(DIR), 1, imgFile);
                        if(entry.DIR_Attr == 0x0F){//if longfgile ignore
                                continue;
                        }
                        if(entry.DIR_Name[0] == 0 || entry.DIR_Name[0] == 0xE5)
                        //if deleted entry ignore
                        {
                                continue;
                        }
                        trim(entry.DIR_Name);
                        if(strcmp(entry.DIR_Name, token)==0)
                        {                                
                                if(entry.DIR_Attr == 0x10){
                                        if(empty(getHiLoClus(entry.DIR_FstClusHi,
                                        entry.DIR_FstClusLo)) == 0)
                                        {
                                                return;
                                        }
                                        
                                        Deallocate(getHiLoClus(entry.DIR_FstClusHi,
                                        entry.DIR_FstClusLo));
                                        char deleteVar;
                                        char* delete = &deleteVar;
                                        *delete = 0xE5;
                                        if(GetNextCluster(CurrentDirectory) < 0x0FFFFFF8)
                                        {
                                                fseek(imgFile, NameAddress, SEEK_SET);
                                                entry.DIR_Name[0] = 0xE5;
                                                fwrite(&entry, sizeof(DIR), 1, imgFile);
                                        }
                                        else{
                                                fseek(imgFile,NameAddress, SEEK_SET);
                                                entry.DIR_Name[0] = 0x00;
                                                
                                                fwrite(delete, sizeof(char), 1, imgFile);

                                        }
                                        break;
                                }
                                else{
                                        printf("%d is not a directory\n", token);
                                }
                        }
                }
                next_cluster = GetNextCluster(next_cluster);
        }

}

void rmCmd(char *token)
{
        
        
        DIR entry;
        int next_cluster = CurrentDirectory;
        while(next_cluster < 0x0FFFFFF8) {
                fseek(imgFile, ClusterByteOffset(next_cluster), SEEK_SET);
                for(int i = 0; i < (BytesPerCluster /32); i++) {
                        int NameAddress = ftell(imgFile);
                        fread(&entry, sizeof(DIR), 1, imgFile);
                        if(entry.DIR_Attr == 0x0F){ //if longfgile ignore
                                continue;
                        }
                        if(entry.DIR_Name[0] == 0 || entry.DIR_Name[0] == 0xE5)
                        //if deleted entry ignore
                        {
                                continue;
                        }
                        trim(entry.DIR_Name);
                        if(strcmp(entry.DIR_Name, token)==0)
                        {                                
                                if(entry.DIR_Attr == 0x20){
                                       
                                        Deallocate(getHiLoClus(entry.DIR_FstClusHi,
                                        entry.DIR_FstClusLo));
                                        char deleteVar;
                                        char* delete = &deleteVar;
                                        *delete = 0xE5;
                                        if(GetNextCluster(CurrentDirectory) < 0x0FFFFFF8)
                                        {
                                                fseek(imgFile, NameAddress, SEEK_SET);
                                                entry.DIR_Name[0] = 0xE5;
                                                fwrite(&entry, sizeof(DIR), 1, imgFile);
                                        }
                                        else{
                                                fseek(imgFile,NameAddress, SEEK_SET);
                                                entry.DIR_Name[0] = 0x00;
                                                
                                                fwrite(delete, sizeof(char), 1, imgFile);

                                        }
                                        break;
                                }
                                else{
                                        printf("%d is not a file\n", token);
                                }
                        }
                }
                next_cluster = GetNextCluster(next_cluster);
        }


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