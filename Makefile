filesys: filesys.c
	gcc -std=c99 -o filesys filesys.c

image:
	rm fat32.img
	cp ../fat32.img .

clean:
	rm -f filesys
