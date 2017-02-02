pseudoFat: main.o fat.o vlakna.o 
	gcc -o pseudoFat main.o fat.o vlakna.o -pthread
main.o: main.c
	gcc -c -o main.o  main.c
fat.o: fat.c
	gcc -c -o fat.o  fat.c
vlakna.o: vlakna.c
	gcc -c -o vlakna.o vlakna.c

