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
	long int currentFilePosition;
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

OpenFile OpenedFiles[10];
//int Cluster = 2;
tokenlist * tokenize(char * input);
void free_tokens(tokenlist * tokens);
char * get_input();
void add_token(tokenlist * tokens, char * item);
void add_to_path(char * dir);
int lsCmd(int);
int cdCmd(int, char*);

int main(int argc, char * argv[]) {
        // error checking for number of arguments.
        // read and open argv[1] in file pointer.
        // obtain important information from bpb as well as initialize any important global variables

        imgFile = fopen(argv[1], "r+");
        fread(&BootBlock, sizeof(BPB_Block), 1, imgFile);

//	OpenFile OpenedFiles[10];

        BytesPerCluster = BootBlock.BPB_BytsPerSec * BootBlock.BPB_SecPerClus;
        FirstDataSector = BootBlock.BPB_RsvdSecCnt + (BootBlock.BPB_NumberofFATS* BootBlock.BPB_FATSz32);
        FirstFatSector = BootBlock.BPB_RsvdSecCnt; //offet
        printf("FirstFatSector: %d\n", FirstFatSector);
        printf("rsvdSec CNT%d\n", BootBlock.BPB_RsvdSecCnt);
        add_to_path(argv[1]);

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
                        printf("calling info\n");
                        offset = BootBlock.BPB_BytsPerSec * BootBlock.BPB_FSInfo;
                        Info(offset);
                }
                else if(strcmp(tokens->items[0], "ls") == 0){
                        printf("calling ls\n");
                        int ls = 0;
                        ls = lsCmd(CurrentDirectory);
//                        test();
                }
                else if(strcmp(tokens->items[0], "cd") == 0){
                        printf("calling cd\n");
                        if(tokens->items[1] != NULL )
                        {
                        CurrentDirectory = cdCmd(CurrentDirectory, tokens->items[1]);
                        }
                }
                else if(strcmp(tokens->items[0], "size") == 0){
                        printf("calling cd\n");
                }
                else if(strcmp(tokens->items[0], "creat") == 0){
                        printf("calling creat\n");
                }
                else if(strcmp(tokens->items[0], "mkdir") == 0){
                        printf("calling mkdir\n");
			int mkdirRes = mkdir(BootBlock, tokens->items[1]);
                        if(mkdirRes == 0){
				printf("Directory %s was created\n", tokens->items[1]);
			}else if(mkdirRes == -1){
				printf("Directory %s already exists\n", tokens->items[1]);
			}
			// int startingCluster = findUnusedCluster(imgFile, FirstDataSector);
                        // //imgFile below this point in scope was fp, for the sake of compile testing
                        // //swapped to imgFile.
                        // createDirectoryEntry(imgFile, tokens->items[1], startingCluster);
                        // updateFatEntry(imgFile, startingCluster, 0xFFFFFFFF);
                }
                else if(strcmp(tokens->items[0], "rm") == 0){
                        printf("calling rm\n");
                }
                else if(strcmp(tokens->items[0], "rmdir") == 0){
                        printf("calling rmdir\n");
                }
                else if(strcmp(tokens->items[0], "open") == 0){
                        printf("calling open\n");
			OpenCmd(tokens->items[1], tokens->items[2]);
//                	test();
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

int getHiLoClus(unsigned short hi, unsigned short lo){
        hi = (hi << 8);
        unsigned int concat = hi | lo;
        return concat;
}

void OpenCmd(char* token1, char* token2){
        int next_cluster = CurrentDirectory;

        printf("Postion: %d\n", ftell(imgFile));
        DIR entry;
        int i;
	int j = 0;
        while(next_cluster < 0x0FFFFFF8) {
          fseek(imgFile, ClusterByteOffset(next_cluster), SEEK_SET);
          for(i = 0; i < (BytesPerCluster /32); i++) {
                fread(&entry, sizeof(DIR), 1, imgFile);
                if(entry.DIR_Attr == 0x20){
			trim(entry.DIR_Name);
			if(!strcmp(token1, entry.DIR_Name)){
				strcpy(OpenedFiles[j].fileName, entry.DIR_Name);
				CurrentFileCluster = 512*(getHiLoClus(entry.DIR_FstClusHi, entry.DIR_FstClusLo));
				printf("CurrentFileCluster: %d\n", CurrentFileCluster);
				char* buffer[8];
				fseek(imgFile, CurrentFileCluster, SEEK_SET);
				fread(buffer, sizeof(8), 1, imgFile);
				printf("%s\n", buffer);
				printf("current position: %d\nCurrentFileCluster: %d\n", ftell(imgFile), CurrentFileCluster);
				j++;
			}
		}
          }
          next_cluster = GetNextCluster(next_cluster);
	}
/*	printf("opened files are:\n");
	for(int i = 0; i < 10; i++){
		printf("%s\n", OpenedFiles[i].fileName);
	}
*/
}

void Info(long offset){
        fseek(imgFile, offset, SEEK_SET);
        fread(&fsi, sizeof(FSInfo), 1, imgFile);

        printf("Bytes Per Sector: %d\n", BootBlock.BPB_BytsPerSec);
        printf("Sectors Per Cluster: %d\n", BootBlock.BPB_SecPerClus);
        printf("Total clusters in Data Region: %d\n", BootBlock.BPB_TotalSec32);
        printf("# of enteries in one FAT: %d\n", BootBlock.BPB_FATSz32);
        printf("Size of Image (bytes): %d\n");                                          //not sure which variable this one is 
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
        printf("ByteOffsetOfCluster: %d\n", clus);
        return clus;
}

int lsCmd(int Directory)
{
        int next_cluster = CurrentDirectory;


        printf("Postion: %d\n", ftell(imgFile));
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
        //if cluster num of .. is 00 then set BPB_RootClus
       // printf("")
        // //long offset2;
        // int numChar = 11;
        // char buffer[BootBlock.BPB_BytsPerSec];
        // //offset2 = FirstDataSector * BootBlock.BPB_BytsPerSec + 32;
        // //Directory += 32;
        // //int counter = 32;
        // int i = 0;
        // // printf("buffer[0] = %d\n", buffer[0]);
        // do 
        // {
        //         // fseek(imgFile, CurrentDirectory, SEEK_SET);
        //         // fread(buffer, sizeof(char), 1, imgFile);
        //         printf("%d\n", Directory);
        //         //dirEntryInit(imgFile,Directory, buffer);
        //         printf("%d\n", FirstFatSector);
        //         if(directoryEnt.DIR_Name[0] != 0)
        //         {
        //         if(directoryEnt.DIR_Name[0] != "." && directoryEnt.DIR_Name[1] != 0 && directoryEnt.DIR_Name[2] != 0)
        //         {
        //         printf("Name is = %s\n", directoryEnt.DIR_Name);
        //         // printf("high = %s\nlow = %s\n", directoryEnt.DIR_FstClusHi, directoryEnt.DIR_FstClusLo);
        //         // FatEntryOffset(Cluster(directoryEnt.DIR_FstClusHi, directoryEnt.DIR_FstClusLo));
        //         FatEntryOffset(Cluster);
        //         }
        //         }
        //         // printf("location = %d\n", CurrentDirectory);
        //         Directory += 32;
        //         //counter += 64;
        //         i++;
        // } while(directoryEnt.DIR_Name[0] != 0);
        
        return 1;
        

}

int cdCmd(int CurrentDirectory, char* token)
{
        int next_cluster = CurrentDirectory;

        fseek(imgFile, ClusterByteOffset(next_cluster), SEEK_SET);

        printf("Postion: %d\n", ftell(imgFile));
        DIR entry;
        int i;
        //while(next_cluster <= 0x0FFFFFF8) {
        for(i = 0; i < (BytesPerCluster /32); i++) {
                fread(&entry, sizeof(DIR), 1, imgFile);
                if(entry.DIR_Attr == 0x0F){                             //if longfgile ignore 
                    continue;    
                }
                if(entry.DIR_Name[0] == 0 || entry.DIR_Name[0] == 0xE5) //if deleted entry ignore 
                {
                        continue;
                }
                //printf("%s\n", entry.DIR_Name);
                //CurrentDirectory = FatEntryOffset(getHiLoClus(entry.DIR_FstClusHi, entry.DIR_FstClusLo));
                trim(entry.DIR_Name);
                if(!strcmp(token, entry.DIR_Name)){
                        printf("%s found\n", token);
                        if(entry.DIR_Attr == 0x10){
                                printf("is directory\n");
                                CurrentDirectory = getHiLoClus(entry.DIR_FstClusHi, entry.DIR_FstClusLo);
				add_to_path(token);
                                //CurrentDirectory = FatEntryOffset(getHiLoClus(entry.DIR_FstClusHi, entry.DIR_FstClusLo));
                                //printf("Current DIrectory: %d\n", CurrentDirectory);
                                break;
                        }else{
                                printf("%s not a directory\n", token);
                        }
                }                         
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




