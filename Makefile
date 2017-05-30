# NAME: Priscilla Cheng and Saurabh Deo
# EMAIL: priscillaccheng@gmail.com and saurabhdeo27@gmail.com
# ID: 404159386 and 404616605

SOURCES = lab3a.c
EXECNAME = lab3a
CC = gcc
DEBUG = -g
LFLAGS = -Wall -Werror $(DEBUG) 
TARCONTENTS = lab3a.c Makefile README ext2_fs.h
TARNAME = lab3a-404616605.tar.gz  #TODO: Check this!


lab3a:
	$(CC) $(LFLAGS) $(SOURCES) -o $(EXECNAME)

clean:
	@- rm -rf lab3a $(TARNAME)

dist:
	tar -czvf $(TARNAME) $(TARCONTENTS) 
