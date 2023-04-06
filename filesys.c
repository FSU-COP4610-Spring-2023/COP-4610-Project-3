#include <stdio.h>
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