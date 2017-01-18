/*
 * main.c
 *
 *  Created on: 8. 1. 2017
 *      Author: Václav
 */

#include "fat.h"


void help(){


	printf("Usage:\n  vaseFAT.dat -a s1 ADR Nahraje soubor z adresáøe do cesty virtuální FAT tabulky \n"
			"		vaseFAT.dat -f s1  Smaže soubor s1 z vaseFAT.dat  (s1 je plná cesta ve virtuální FAT) \n"
			" 		vaseFAT.dat -c s1 Vypíše èísla clusterù, oddìlené dvojteèkou, obsahující data souboru s1 (s1 je plná cesta ve virtuální FAT)\n "
			"		vaseFAT.dat -m ADR ADR2 Vytvoøí nový adresáø ADR v cestì ADR2\n"
			"		vaseFAT.dat -r ADR Smaže prázdný adresáø ADR (ADR je plná cesta ve virtuální FAT) \n"
			"		vaseFAT.dat -l s1 Vypíše obsah souboru s1 na obrazovku (s1 je plná cesta ve virtuální FAT)\n"
			"		vaseFAT.dat -p Vypíše obsah adresáøe ve formátu +adresáø, +podadresáø cluster, ukonèeno --, - soubor první_cluster poèet_clusterù. Jeden záznam jeden øádek. \n"
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

