# COP-4610-Project-3

## README.md MEANT TO BE READ IN GITHUB

## Project Team Members
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Anthony Knapik,
Nicholas Ratzlaff,
Matthew Christofel,

## Division of Labor
Part 1: Mounting the Image  
;Nick Ratzlaff Anthony Knapik Matthew Christoffel  
Part 2: Navigation:  
cd: Anthony Knapik  
ls: Nick Ratzlaff  
Part 3: Create:   
mkdir: Nick Ratzlaff   
creat: Anthony Knapik  
cp:  Matthew Christoffel  
Part 4: Read:  
open: Anthony Knapik, Nick Ratzlaff  
close: Anthony Knapik, Matthew Christoffel  
lsof: Matthew Christoffel, Nick Ratzlaff  
size: Nick Ratzlaff, Anthony Knapik  
lseek: Matthew Christoffel, Anthony Knapik  
read: Anthony Knapik, Nick Ratzlaff, Matthew Christoffel  
Part 5: Update:  
write: Matthew Christoffel, Anthony Knapik  
rename: Anthony Knapik  
Part 6: Delete:  
rm: Nick Ratzlaffl, Matthew Christoffel   
Rmdir: Matthew Christoffel, Nick Ratzlaff  
Extra Credit:   
Anthony Knapik Nick Ratzlaff Matthew Christoffel  
Long-name support: Nick Ratzalff  
rm -r: Anthony Knapik  
Dump: Matthew Christoffel  

## Updated Division of Labor  
Part 1: Mounting the Image  
Nick Ratzalff, Anthony Knapik, Matthew Christoffel  
Part 2: Navigation  
cd: Anthony Knapik, Matthew Christoffel  
ls: Anthony Knapik, Matthew Christoffel  
Part 3: Create  
mkdir: Nick Ratzalff, Matthew Christoffel  
creat: Nick Ratzalff  
cp:  Matthew Christoffel, Anthony Knapik, Nick Ratzalff  
Part 4: Read  
open: Matthew Christoffel, Anthony Knapik, Nick Ratzalff  
close: Matthew Christoffel, Anthony Knapik  
lsof: Matthew Christoffel, Anthony Knapik  
size: Matthew Christoffel, Anthony Knapik  
lseek: Matthew Christoffel, Anthony Knapik, Nick Ratzalff  
read: Matthew, Anthony Knapik, Nick Ratzlaff  
Part 5: Update  
write: Matthew Christoffel, Anthony Knapik  
rename: Matthew Christoffel, Anthony Knapik  
Part 6: Delete  
rm: Matthew Christoffel, Anthony Knapik  
Rmdir: Matthew Christoffel, Anthony Knapik  
Extra Credit  
Anthony Knapik, Nick Ratzalff, Matthew Christoffel  

## Known Bugs
'creat' - displays multiple copies when creating a file in a created directory.  
'rm' and 'mkdir' - using rm on longfiles and then using mkdir and then using cd into the new dirctory   
and running ls, it will show corrupted text for some reson. Unsure of why this happens.  
'read' - Reading files somtimes leaves out a few characters of text when eading file longer than one cluster.  
May be an issue with how many bytes are being read in with fcget().  
'size' - when chainging the text with write() on a file, the size command sometimes hows the previous size of
the file.   


## List of Files in the Repository
filesys.c - Contains all the fuctions to manipulate and read data of different files in the fat32.img. Also contains 
the menu to ru the functions required by the project.  
filesys - exectutable of filesys.c  
README.md - documentation of the project that we are currently reading. Contains bugs, work division, list of files,
and how to compile the project.  
fat32.img - fat image that contains the files we are manipulating in the project.   
Makefile - makefile to to compile the project creating the executable, as well as clean to clean out the exectuable.  

## How to Compile Using Makefile
typing 'make' creates the 'filesys' executable that can be run by typing './filesys fat32.img. This begins the   
project where you can run the commands reuired for the project.  

## Special Considerations
N/A

## Extra Credit
N/A3

