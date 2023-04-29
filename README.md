# COP-4610-Project-3

## Project Team Members
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Anthony Knapik,
Nicholas Ratzlaff,
Matthew Christofel,

## Division of Labor
Part 1: Mounting the Image  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Nick Ratzlaff Anthony Knapik Matthew Christoffel  
Part 2: Navigation:  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;cd: Anthony Knapik  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;ls: Nick Ratzlaff  
Part 3: Create:   
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;mkdir: Nick Ratzlaff   
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;creat: Anthony Knapik  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;cp:  Matthew Christoffel  
Part 4: Read:  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;open: Anthony Knapik, Nick Ratzlaff  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;close: Anthony Knapik, Matthew Christoffel  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;lsof: Matthew Christoffel, Nick Ratzlaff  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;size: Nick Ratzlaff, Anthony Knapik  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;lseek: Matthew Christoffel, Anthony Knapik  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;read: Anthony Knapik, Nick Ratzlaff, Matthew Christoffel  
Part 5: Update:  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;write: Matthew Christoffel, Anthony Knapik  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;rename: Anthony Knapik  
Part 6: Delete:  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;rm: Nick Ratzlaffl, Matthew Christoffel   
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Rmdir: Matthew Christoffel, Nick Ratzlaff  
Extra Credit:   
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Anthony Knapik Nick Ratzlaff Matthew Christoffel  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Long-name support: Nick Ratzalff  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;rm -r: Anthony Knapik  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Dump: Matthew Christoffel  

## Updated Division of Labor  
Part 1: Mounting the Image  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Nick Ratzalff, Anthony Knapik, Matthew Christoffel  
Part 2: Navigation  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;cd: Anthony Knapik, Matthew Christoffel  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;ls: Anthony Knapik, Matthew Christoffel  
Part 3: Create  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;mkdir: Nick Ratzalff, Matthew Christoffel  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;creat: Nick Ratzalff  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;cp:  Matthew Christoffel, Anthony Knapik, Nick Ratzalff  
Part 4: Read  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;open: Matthew Christoffel, Anthony Knapik, Nick Ratzalff  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;close: Matthew Christoffel, Anthony Knapik  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;lsof: Matthew Christoffel, Anthony Knapik  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;size: Matthew Christoffel, Anthony Knapik  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;lseek: Matthew Christoffel, Anthony Knapik, Nick Ratzalff  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;read: Matthew, Anthony Knapik, Nick Ratzlaff  
Part 5: Update  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;write: Matthew Christoffel, Anthony Knapik  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;rename: Matthew Christoffel, Anthony Knapik  
Part 6: Delete  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;rm: Matthew Christoffel, Anthony Knapik  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Rmdir: Matthew Christoffel, Anthony Knapik  
Extra Credit  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Anthony Knapik, Nick Ratzalff, Matthew Christoffel  

## Known Bugs
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;'creat' - displays multiple copies when creating a file in a created directory.  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;'rm' and 'mkdir' - using rm on longfiles and then using mkdir and then using cd into the new dirctory   
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;and running ls, it will show corrupted text for some reson. Unsure of why this happens.  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;'read' - Reading files somtimes leaves out a few characters of text when eading file longer than one cluster.  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;May be an issue with how many bytes are being read in with fcget().  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;'size' - when chainging the text with write() on a file, the size command sometimes hows the previous size of
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;the file.   


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

