CC = gcc
options = -Wall -g -std=c99 -O2 -fsanitize=address,undefined

all: spchk

spell.o: spell.c spell.h
	$(CC) -c $(options) spell.c

arraylist.o: arraylist.c arraylist.h
	$(CC) -c $(options) arraylist.c

alSTR.o: alSTR.c alSTR.h
	$(CC) -c $(options) alSTR.c

driver.o: driver.c driver.h spell.h arraylist.h alSTR.h
	$(CC) -c $(options) driver.c

spchk: driver.o spell.o arraylist.o alSTR.o
	$(CC) $(options) driver.o spell.o arraylist.o alSTR.o -o spchk

clean:
	rm -f *.o