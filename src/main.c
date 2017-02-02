/*
 * main.c
 *
 *  Created on: 8. 1. 2017
 *      Author: V�clav
 */

#include "fat.h"
#include "vlakna.h"

/**
 * void help(){
 * Funkce pro vypsani napovedy jak spustit program
 */
void help(){


	printf("Usage:\n  vaseFAT.dat -a s1 ADR Nahraje soubor z adresare do cesty virtualni FAT tabulky \n"
			"		vaseFAT.dat -f s1  Smaze soubor s1 z vaseFAT.dat  (s1 je plna cesta ve virtuilni FAT) \n"
			" 		vaseFAT.dat -c s1 Vypse cisla clusteru, oddelene dvojteckou, obsahujici data souboru s1 (s1 je plna cesta ve virtualni FAT)\n "
			"		vaseFAT.dat -m ADR ADR2 Vytvori novy adresar ADR v ceste ADR2\n"
			"		vaseFAT.dat -r ADR Smaze prazdny adresar ADR (ADR je plna cesta ve virtualni FAT) \n"
			"		vaseFAT.dat -l s1 Vypise obsah souboru s1 na obrazovku (s1 je plna cesta ve virtualni FAT)\n"
			"		vaseFAT.dat -p Vypse obsah adresare ve formatu +adresar, +podadresar cluster, ukonceno --, - soubor prvni_cluster pocet_clusteru. Jeden zaznam jeden radek. \n"
			"		vaseFAT.dat -b badblock\n");
}



/**
 * int main(int argc, char** argv)
 * Hlavni spousteci funkce aplikace
 */
int main(int argc, char** argv) {
    int err = 0;
	if(argc < 3 ){
		help();
		return 1;
	}



    if(strcmp(argv[1], "-t") != 0){

    	err = nacti_zaklad_fat(argv[1]);

		if (strcmp(argv[2], "-a") == 0) {

		err = zapis_soubor(argv[1],argv[3], argv[4]);

	}else if(strcmp(argv[2], "-f") == 0){
		err = smaz_soubor(argv[1], argv[3]);
	}else if(strcmp(argv[2], "-c") == 0){
		err = vypis_clustery(argv[3]);
	}else if(strcmp(argv[2], "-m") == 0){
		err = vytvor_slozku(argv[1],argv[3], argv[4]);
	}else if(strcmp(argv[2], "-r") == 0){

	err = smaz_prazdnou_slozku(argv[1],argv[3]);

	}else if(strcmp(argv[2], "-l") == 0){
      err = vypis_obsah_souboru(argv[3]);
	}else if(strcmp(argv[2], "-p") == 0){
      err =  vypis_fat();
	}else if(strcmp(argv[2], "-b") == 0){

	err =	oprav_blok(argv[1]);

	}else {
		help();
		return 1;
	}


    }else if (strcmp(argv[1], "-t") == 0){
    err = vytvor_testovaci_fat(argv[2]);
    }else{
    help();
    return 1;

    }

uloz_fat(argv[1]);
    uvolni_fat();


	return err;
}

