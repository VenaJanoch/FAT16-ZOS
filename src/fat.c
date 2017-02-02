/*
 * fat.c
 *
 *  Created on: 13. 1. 2017
 *      Author: Vï¿½clav
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "fat.h"

boot_record *p_boot_record;
root_directory *p_root_directory;
unsigned int **fat_table;
char *clusters;

/**
 * uvolni_fat()
 * Uvolní fat z pamìti
 *
 */
void uvolni_fat() {
	int i;

	for (i = 0; i < p_boot_record->fat_copies; i++) {
		free(fat_table[i]);
	}

	free(p_boot_record);

	free(fat_table);
	free(p_root_directory);

	free(clusters);

}

/**
 * nacti_zaklad_fat(char* jmeno)
 * Nacte zakladni udaje o FAT
 * Boot_record, root_directory, fat_table, clusters
 *
 */
int nacti_zaklad_fat(char* jmeno) {

	FILE *file = fopen(jmeno, "r");
	if (file == NULL) {
		printf("Nenalezen soubor\n");
		return 1;
	}

	p_boot_record = (boot_record *) malloc(sizeof(boot_record));
	fread(p_boot_record, sizeof(boot_record), 1, file);

	fat_table = malloc(sizeof(unsigned int *) * p_boot_record->fat_copies);

	int i;
	for (i = 0; i < p_boot_record->fat_copies; i++) {
		unsigned int *nova_fat = (unsigned int *) malloc(
				sizeof(unsigned int) * p_boot_record->cluster_count);

		fread(nova_fat, sizeof(unsigned int) * p_boot_record->cluster_count, 1,
				file);
		fat_table[i] = nova_fat;
	}

	p_root_directory = malloc(
			sizeof(root_directory)
					* p_boot_record->root_directory_max_entries_count);
	fread(p_root_directory,
			sizeof(root_directory)
					* p_boot_record->root_directory_max_entries_count, 1, file);

	clusters = malloc(
			sizeof(char) * p_boot_record->cluster_size
					* p_boot_record->cluster_count);

	fread(clusters,
			sizeof(char) * p_boot_record->cluster_size
					* p_boot_record->cluster_count, 1, file);

	fclose(file);

	return 1;
}

/**
 * uloz_fat(char *path)
 * Ulozi FAT pod zadanym jmenem
 *
 */
int uloz_fat(char *path) {
	FILE *soubor = fopen(path, "w+");

	fwrite(p_boot_record, sizeof(boot_record), 1, soubor);

	int i;
	for (i = 0; i < p_boot_record->fat_copies; i++) {

		fwrite(fat_table[i],
				sizeof(unsigned int) * p_boot_record->cluster_count, 1, soubor);

	}

	fwrite(p_root_directory,
			sizeof(root_directory)
					* p_boot_record->root_directory_max_entries_count, 1,
			soubor);

	fwrite(clusters,
			sizeof(char) * p_boot_record->cluster_size
					* p_boot_record->cluster_count, 1, soubor);

	return 1;
}

/**
 * vytovor_badblock(unsigned int zacatek, unsigned int konec)
 * vytovøí badblocky na zadaném intervalu
 *
 */
int vytovor_badblock(unsigned int zacatek, unsigned int konec) {

	unsigned int i, j;
	for (i = zacatek; i <= konec; i++) {
		char *cluster = clusters + i * p_boot_record->cluster_size;

		for (j = 0; j < 6; j++) {
			cluster[j] = 'F';
			cluster[p_boot_record->cluster_size - j - 2] = 'F';
		}
	}

	return 1;

}

/**
 * vytvor_testovaci_fat(char *path)
 * Vytvoøí fat tabulku s testovacimy soubory a badblocky
 *
 */
int vytvor_testovaci_fat(char *path) {
	int i, j;
	p_boot_record = (boot_record *) malloc(sizeof(boot_record));

	strcpy(p_boot_record->volume_descriptor, "MojeFAT");
	p_boot_record->fat_copies = 2;
	p_boot_record->fat_type = 12;
	p_boot_record->cluster_size = 256;
	p_boot_record->cluster_count = 4086;
	p_boot_record->reserved_cluster_count = 10;
	p_boot_record->root_directory_max_entries_count = 28;
	fat_table = malloc(sizeof(unsigned int *) * p_boot_record->fat_copies);

	for (i = 0; i < p_boot_record->fat_copies; i++) {
		unsigned int *nova_fat = (unsigned int *) malloc(
				sizeof(unsigned int) * p_boot_record->cluster_count);

		for (j = 0; j < p_boot_record->cluster_count; j++) {
			nova_fat[j] = FAT_UNUSED;
		}
		fat_table[i] = nova_fat;
	}

	p_root_directory = (root_directory *) malloc(
			sizeof(root_directory)
					* p_boot_record->root_directory_max_entries_count);
	memset(p_root_directory, 0,
			sizeof(root_directory)
					* p_boot_record->root_directory_max_entries_count);
	clusters = malloc(
			sizeof(char) * p_boot_record->cluster_size
					* p_boot_record->cluster_count);
	memset(clusters, 0,
			sizeof(char) * p_boot_record->cluster_size
					* p_boot_record->cluster_count);

	zapis_soubor(path, "fat.h", "/");
	vytvor_slozku(path, "testovaci", "/");
	zapis_soubor(path, "vlakna.h", "/testovaci");
	vytovor_badblock(5, 6);
	vytovor_badblock(8, 9);
	vytovor_badblock(14, 15);

	uloz_fat(path);
	printf("Vytvorena testovaci FAT\n");
	return 1;
}

/**
 * najdi_prvni_cluster(unsigned int *cluster)
 * najde první volny cluste
 *
 */
int najdi_prvni_cluster(unsigned int *cluster) {
	unsigned int i;
	for (i = 0; i < p_boot_record->cluster_count; i++) {
		if (fat_table[0][i] == FAT_UNUSED) {
			*cluster = i;
			return 1;
		}
	}
	return 0;
}

/**
 * najdi_predchozi_cluster(unsigned int *cluster)
 * Najde index predchazejiciho clusteru
 *
 */
int najdi_predchozi_cluster(unsigned int *cluster) {

	unsigned int i;
	for (i = 0; i < p_boot_record->cluster_count; i++) {
		if (fat_table[0][i] == *cluster) {

			*cluster = i;
			return 1;
		}
	}
	return 0;
}
/**
 * najdi_jmeno(const char *zacatek, const char *konec)
 * Najde jmeno souboru mezi lomítky
 *
 */
char *najdi_jmeno(const char *zacatek, const char *konec) {
	size_t jmeno_length;
	char *jmeno;

	jmeno_length = konec - zacatek;
	jmeno = (char *) malloc(jmeno_length + 1);
	memcpy(jmeno, zacatek, jmeno_length);
	jmeno[jmeno_length] = '\0';
	return jmeno;
}

/**
 * root_directory *najdi_slozku(root_directory *slozka, char *jmeno)
 * Najde hledany soubor
 *
 */
root_directory *najdi_slozku(root_directory *slozka, char *jmeno) {

	while (slozka->file_name[0] != '\0') {
		if (strcmp(slozka->file_name, jmeno) == 0) {
			return slozka;
		}
		slozka++;
	}
	return NULL;
}

/**
 * pruchod_cestou(char** start, root_directory **soubor)
 * Projde danou cestu pro kontrolu existence
 *
 */
int pruchod_cestou(char** start, root_directory **soubor) {
	char *jmeno;
	char *end;
	int root = 1;

	while (1) {

		end = strchr((*start), '/');

		if (end == NULL) {
			break;
		}

		if (root) {
			root = 0;
		} else {
			if ((*soubor)->file_type == TYPE_SLOZKA) {
				(*soubor) =
						(root_directory *) (clusters
								+ (*soubor)->first_cluster
										* p_boot_record->cluster_size);
			} else {

				return -1;
			}
		}
		jmeno = najdi_jmeno((*start), end);
		(*soubor) = najdi_slozku((*soubor), jmeno);
		free(jmeno);
		if ((*soubor) == NULL) {

			return -1;
		}

		(*start) = end + 1;
	}
	return root;
}

/**
 * int kontrola_cesty(char* cesta, root_directory **vysledny_soubor)
 * Zjisti existenci pripadne neexistenci cesty
 *
 */
int kontrola_cesty(char* cesta, root_directory **vysledny_soubor) {

	int koren = 1;
	char *start;

	root_directory *soubor = p_root_directory;

	if (cesta[0] == '/') {
		cesta++;
	}

	if (cesta[0] == '\0') {
		*vysledny_soubor = NULL;
		return 1;
	}

	start = cesta;
	koren = pruchod_cestou(&start, &soubor);

	if (koren == -1)
		return 0;
	if (*start != '\0') {

		if (!koren) {

			if (soubor->file_type == TYPE_SLOZKA) {

				soubor = (root_directory *) (clusters
						+ soubor->first_cluster * p_boot_record->cluster_size);
			} else {

				return 0;
			}
		}
		soubor = najdi_slozku(soubor, start);

		if (soubor == NULL) {
			return 0;
		}
	}

	*vysledny_soubor = soubor;
	return 1;
}

/**
 * int vytvor_slozku_soubor(root_directory **slozka)
 * Pomocná metoda pro vytvoøení souboru ve FAT
 *
 */
int vytvor_slozku_soubor(root_directory **slozka) {
	if ((*slozka) == NULL) {
		(*slozka) = p_root_directory;
	} else {
		if ((*slozka)->file_type != TYPE_SLOZKA) {
			return 0;
		}
		(*slozka) = (root_directory *) (clusters
				+ (*slozka)->first_cluster * p_boot_record->cluster_size);
	}
	return 1;
}

/**
 * char *ziskej_nazev(char *soubor)
 * Pomocná metoda pro vytvoøení souboru ve FAT
 *
 */
char *ziskej_nazev(char *soubor) {
	char *jmeno_souboru = strrchr(soubor, '/');
	if (jmeno_souboru != NULL) {
		jmeno_souboru += 1;
	} else {
		jmeno_souboru = soubor;
	}
	return jmeno_souboru;
}

/**
 * void nahraj_info(root_directory **novy_soubor, char* jmeno_souboru,
		int velikost, int prvni_cluster, int type)
 * Nahraje informace o souboru\slozce do daného souboru
 *
 */
void nahraj_info(root_directory **novy_soubor, char* jmeno_souboru,
		int velikost, int prvni_cluster, int type) {

	memset((*novy_soubor)->file_name, 0, sizeof((*novy_soubor)->file_name));
	strcpy((*novy_soubor)->file_name, jmeno_souboru);
	(*novy_soubor)->file_size = velikost;
	(*novy_soubor)->file_type = type;
	(*novy_soubor)->first_cluster = prvni_cluster;
	memset((*novy_soubor) + 1, 0, sizeof(root_directory));

}

/**
 * void pridej_cluster_fat(int cluster, int index)
 * Pridani informaci do clusteru
 *
 */
void pridej_cluster_fat(int cluster, int index) {
	int i;
	for (i = 0; i < p_boot_record->fat_copies; i++) {
		fat_table[i][index] = cluster;
	}
}

/**
 * int zapis_soubor(char* fat, char *soubor_pro_zapis, char *cesta)
 * Funkce pro zápis souboru do pseudoFAT
 *
 */
int zapis_soubor(char* fat, char *soubor_pro_zapis, char *cesta) {

	unsigned int prvni_cluster = FAT_UNUSED;
	unsigned int novy_cluster = FAT_UNUSED;
	unsigned int predchozi_cluster = FAT_UNUSED;
	long file_size = 0;
	size_t last_count = 0;

	root_directory *slozka_pro_nacteni;
	root_directory *novy_soubor;

	if (!kontrola_cesty(cesta, &slozka_pro_nacteni)) {
		printf("PATH NOT FOUND\n");
		return 0;
	}

	if (vytvor_slozku_soubor(&slozka_pro_nacteni) != 1)
		return 0;

	FILE *soubor = fopen(soubor_pro_zapis, "r+");

	char *jmeno_souboru = ziskej_nazev(soubor_pro_zapis);

	while (1) {

		najdi_prvni_cluster(&novy_cluster);
		size_t count = fread(
				clusters + novy_cluster * p_boot_record->cluster_size, 1,
				p_boot_record->cluster_size - 1, soubor);
		if (count <= 0) {
			break;
		}
		last_count = count;

		file_size += count;
		if (predchozi_cluster != FAT_UNUSED) {

			pridej_cluster_fat(novy_cluster, predchozi_cluster);
		} else {

			prvni_cluster = novy_cluster;

		}

		predchozi_cluster = novy_cluster;

		pridej_cluster_fat(0, novy_cluster);

	}

	if (prvni_cluster != FAT_UNUSED) {
		if (predchozi_cluster != FAT_UNUSED) {
			pridej_cluster_fat(FAT_FILE_END, predchozi_cluster);

			memset(
					clusters + predchozi_cluster * p_boot_record->cluster_size
							+ last_count, 0,
					p_boot_record->cluster_size - last_count);
		}

		while (slozka_pro_nacteni->file_name[0] != '\0') {
			if (strcmp(slozka_pro_nacteni->file_name, jmeno_souboru) == 0) { /* Nazev adresare existuje */

				return 0;
			}

			slozka_pro_nacteni++;
		}

		novy_soubor = slozka_pro_nacteni;
		nahraj_info(&novy_soubor, jmeno_souboru, file_size, prvni_cluster,
		TYPE_SOUBOR);
	}

	fclose(soubor);
	printf("OK\n");

	return 1;
}

/**
 * smaz_soubor(char* fat, char *cesta)
 * smaze zadany soubor z fat
 *
 */
int smaz_soubor(char* fat, char *cesta) {

	root_directory *soubor;
	if (!kontrola_cesty(cesta, &soubor)) {
		printf("PATH NOT FOUND\n");
		return 0;
	}

	if (soubor->file_type != TYPE_SOUBOR) {
		printf("IS NOT FILE\n");
		return 0;
	}

	unsigned int current_cluster = soubor->first_cluster;

	while (current_cluster != FAT_FILE_END && current_cluster != FAT_BAD_CLUSTER) {
		unsigned int next_cluster = fat_table[0][current_cluster];

		pridej_cluster_fat(FAT_UNUSED, current_cluster);

		current_cluster = next_cluster;
	}

	while (1) {
		*soubor = *(soubor + 1);

		if (soubor->file_name[0] == '\0') {
			break;
		}

		soubor++;
	}

	uloz_fat(fat);
	printf("OK\n");

	return 1;
}

/**
 * vypis_clustery(char* path)
 * vypise cisla clusteru obsazenych danym souborem
 *
 */
int vypis_clustery(char* path) {

	root_directory *soubor;
	if (!kontrola_cesty(path, &soubor)) {
		printf("PATH NOT FOUND\n");

		return 0;
	}

	printf("%s ", soubor->file_name);

	unsigned int pom_cluster = soubor->first_cluster;

	while (pom_cluster != FAT_FILE_END && pom_cluster != FAT_BAD_CLUSTER) {
		printf("%u", pom_cluster);

		unsigned int dalsi_cluster = fat_table[0][pom_cluster];
		pom_cluster = dalsi_cluster;

		if (pom_cluster != FAT_FILE_END && pom_cluster != FAT_BAD_CLUSTER) {
			printf(",");
		}
	}

	printf("\n");

	return 1;
}


/**
 * int vytvor_slozku(char*fat, char *jmeno_slozky, char *cesta)
 * vytvori slozku na zadane ceste
 *
 */
int vytvor_slozku(char*fat, char *jmeno_slozky, char *cesta) {

	root_directory *slozka;
	if (!kontrola_cesty(cesta, &slozka)) {
		printf("PATH NOT FOUND\n");

		return 0;
	}

	if (slozka != NULL && slozka->file_type != TYPE_SLOZKA) {

		return 0;
	}

	unsigned int cluster = FAT_UNUSED;

	najdi_prvni_cluster(&cluster);

	memset(clusters + cluster * p_boot_record->cluster_size, 0,
			p_boot_record->cluster_size);

	pridej_cluster_fat(FAT_FILE_END, cluster);

	if (slozka == NULL) {
		slozka = p_root_directory;
	} else {
		slozka = (root_directory *) (clusters
				+ slozka->first_cluster * p_boot_record->cluster_size);
	}

	while (slozka->file_name[0] != '\0') {
		if (strcmp(slozka->file_name, jmeno_slozky) == 0) {
			return 0;
		}

		slozka++;
	}

	nahraj_info(&slozka, jmeno_slozky, 0, cluster, TYPE_SLOZKA);
	memset(slozka + 1, 0, sizeof(root_directory));
	uloz_fat(fat);
	printf("OK\n");

	return 1;
}

/**
 * int smaz_prazdnou_slozku(char* fat, char *cesta)
 * smaze slozku na zadane ceste
 *
 */
int smaz_prazdnou_slozku(char* fat, char *cesta) {

	root_directory *slozka;
	if (!kontrola_cesty(cesta, &slozka)) {
		printf("PATH NOT FOUND\n");

		return 0;
	}

	if (slozka == NULL) {
		return 0;
	}

	if (slozka->file_type != TYPE_SLOZKA) {

		return 0;
	}

	root_directory *pom = (root_directory *) (clusters
			+ slozka->first_cluster * p_boot_record->cluster_size);

	if (pom->file_name[0] != '\0') {
		printf("PATH NOT EMPTY\n");

		return 0;
	}

	root_directory *tmp = slozka;

	while (slozka->file_name[0] != '\0') {
		slozka++;
	}

	slozka--;
	*tmp = *slozka;
	memset(slozka, 0, sizeof(root_directory));
	uloz_fat(fat);
	printf("OK\n");

	return 1;
}

/**
 * int vypis_obsah_souboru(char *cesta)
 * vypise obsah nalezající se v souboru
 *
 */
int vypis_obsah_souboru(char *cesta) {

	root_directory *soubor;
	if (!kontrola_cesty(cesta, &soubor)) {
		printf("PATH NOT FOUND\n");
		return 0;
	}

	printf("soubor %s: ", soubor->file_name);

	unsigned int pom_cluster = soubor->first_cluster;

	while (pom_cluster != FAT_FILE_END && pom_cluster != FAT_BAD_CLUSTER) {

		printf("%*s", p_boot_record->cluster_size,
				clusters + p_boot_record->cluster_size * pom_cluster);

		unsigned int next_cluster = fat_table[0][pom_cluster];
		pom_cluster = next_cluster;
	}

	printf("\n");

	return 1;
}

/**
 * void vytvor_odsazeni(unsigned pocet)
 * Pomocna funkce pro vypis obsahu FAT
 *
 */
void vytvor_odsazeni(unsigned pocet) {
	int i;
	for (i = 0; i < pocet; i++) {
		printf("\t");
	}
}

/**
 * int vypis_obsah_rekurze(root_directory *slozka, unsigned tabs)
 * Funkce pro rekurzivni pruchod FAT a vypsani obsahu
 *
 */
int vypis_obsah_rekurze(root_directory *slozka, unsigned tabs) {

	while (slozka->file_name[0] != '\0') {
		if (slozka->file_type == TYPE_SOUBOR) {
			unsigned int pocet_clusteru = 0;

			unsigned int pom_cluster = slozka->first_cluster;
			while (pom_cluster != FAT_FILE_END && pom_cluster != FAT_BAD_CLUSTER) {

				unsigned int next_cluster = fat_table[0][pom_cluster];
				pom_cluster = next_cluster;

				pocet_clusteru++;
			}
			vytvor_odsazeni(tabs);
			printf("-%s %u %u\n", slozka->file_name, slozka->first_cluster,
					pocet_clusteru);
		} else if (slozka->file_type == TYPE_SLOZKA) {
			vytvor_odsazeni(tabs);
			printf("+%s %u\n", slozka->file_name, slozka->first_cluster);
			vypis_obsah_rekurze(
					(root_directory *) (clusters
							+ slozka->first_cluster
									* p_boot_record->cluster_size), tabs + 1);
			vytvor_odsazeni(tabs);
			printf("--\n");
		}
		slozka++;
	}

	return 1;
}
/**
 * int vypis_fat()
 * Kontrola zda neni fat prázdná a následné zavolání rekurzivního pruchodu FAT
 *
 */
int vypis_fat() {

	root_directory *slozka = p_root_directory;
	if (slozka->file_name[0] == '\0') {
		printf("EMPTY\n");
		return 1;
	}

	printf("+ROOT\n");

	vypis_obsah_rekurze(slozka, 1);
	printf("--\n");

	return 1;
}

