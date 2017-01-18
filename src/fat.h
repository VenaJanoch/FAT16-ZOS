/*
 * fat.h
 *
 *  Created on: 8. 1. 2017
 *      Author: Václav
 */

#ifndef SRC_FAT_H_
#define SRC_FAT_H_
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

//struktura na root directory - nova verze
typedef struct root_directory{
    char file_name[13];             //8+3 format + '/0'
    char file_mod[10];              //unix atributy souboru+ '/0'
    short file_type;                //0 = soubor, 1 = adresar
    long file_size;                 //pocet znaku v souboru
    unsigned int first_cluster;     //cluster ve FAT, kde soubor zacina - POZOR v cislovani root_directory ma prvni cluster index 0 (viz soubor a.txt)
}root_directory;


typedef struct boot_record {
    char volume_descriptor[251];               //popis
    int fat_type;                             //typ FAT - pocet clusteru = 2^fat_type (priklad FAT 12 = 4096)
    int fat_copies;                           //kolikrat je za sebou v souboru ulozena FAT
    unsigned int cluster_size;                //velikost clusteru ve znacich (n x char) + '/0' - tedy 128 znamena 127 vyznamovych znaku + '/0'
    long root_directory_max_entries_count;    //pocet polozek v root_directory = pocet souboru MAXIMALNE, nikoliv aktualne - pro urceni kde zacinaji data - resp velikost root_directory v souboru
    unsigned int cluster_count;               //pocet pouzitelnych clusteru (2^fat_type - reserved_cluster_count)
    unsigned int reserved_cluster_count;      //pocet rezervovanych clusteru pro systemove polozky
    char signature[4];                        //pro vstupni data od vyucujicich konzistence FAT - "OK","NOK","FAI" - v poradku / FAT1 != FAT2 != FATx / FAIL - ve FAT1 == FAT2 == FAT3, ale obsahuje chyby, nutna kontrola
}boot_record;

extern boot_record *p_boot_record;
extern root_directory *p_root_directory;
extern unsigned int **fat_table;
extern char *clusters;


FILE *nacti_soubor(char *file_name);
int nacti_boot_record(FILE *file);
int nacti_root_directory(FILE *file);
char* nacti_cluster(FILE *file);
int nacteni_fat_tabulek(FILE *file);
int nacti_zaklad_fat(char* jmeno);

int najdi_prvni_cluster(unsigned int *cluster);
int kontrola_cesty(char* cesta, root_directory **vysledny_soubor);
int zapis_soubor(char *soubor_pro_zapis, char *cesta);
int smaz_soubor(char *cesta);
int vypis_clustery(char* path);
int vytvor_slozku(char *jmeno_slozky, char *cesta);
int smaz_prazdnou_slozku(char *cesta);
int obsah_souboru( char *cesta);
int print_content(root_directory *slozka, unsigned tabs);
int vytvor_testovaci_fat(char *path);
int vytovor_badblock(unsigned int zacatek, unsigned int konec);
int uloz_fat(char *path);
#endif /* SRC_FAT_H_ */
