/*
 * main.c
 *
 *  Created on: 8. 1. 2017
 *      Author: V�clav
 */

#include "fat.h"


void help(){


	printf("Usage:\n  vaseFAT.dat -a s1 ADR Nahraje soubor z adres��e do cesty virtu�ln� FAT tabulky \n"
			"		vaseFAT.dat -f s1  Sma�e soubor s1 z vaseFAT.dat  (s1 je pln� cesta ve virtu�ln� FAT) \n"
			" 		vaseFAT.dat -c s1 Vyp�e ��sla cluster�, odd�len� dvojte�kou, obsahuj�c� data souboru s1 (s1 je pln� cesta ve virtu�ln� FAT)\n "
			"		vaseFAT.dat -m ADR ADR2 Vytvo�� nov� adres�� ADR v cest� ADR2\n"
			"		vaseFAT.dat -r ADR Sma�e pr�zdn� adres�� ADR (ADR je pln� cesta ve virtu�ln� FAT) \n"
			"		vaseFAT.dat -l s1 Vyp�e obsah souboru s1 na obrazovku (s1 je pln� cesta ve virtu�ln� FAT)\n"
			"		vaseFAT.dat -p Vyp�e obsah adres��e ve form�tu +adres��, +podadres�� cluster, ukon�eno --, - soubor prvn�_cluster po�et_cluster�. Jeden z�znam jeden ��dek. \n"
			"		vaseFAT.dat -b badblock\n");
}




int main(int argc, char** argv) {

	if(argc < 3 ){
		help();
		return 1;
	}

	if (strcmp(argv[2], "-a") == 0) {

		zapis_soubor(argv[3], argv[4]);

	}else if(strcmp(argv[2], "-f") == 0){

	}else if(strcmp(argv[2], "-c") == 0){

	}else if(strcmp(argv[2], "-m") == 0){

	}else if(strcmp(argv[2], "-r") == 0){

	}else if(strcmp(argv[2], "-l") == 0){

	}else if(strcmp(argv[2], "-p") == 0){

	}else if(strcmp(argv[2], "-b") == 0){

	}else if(strcmp(argv[1], "-t") == 0){

		vytvor_testovaci_fat("MojeFAT.txt");
	}else{
		help();
		return 1;
	}

	return 0;
}

