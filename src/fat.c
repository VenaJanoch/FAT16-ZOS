/*
 * fat.c
 *
 *  Created on: 13. 1. 2017
 *      Author: V�clav
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "fat.h"

const int FAT_UNUSED = 65535;
const int FAT_FILE_END = 65534;
const int FAT_BAD_CLUSTER = 65533;
const int TYPE_SOUBOR = 0;
const int TYPE_SLOZKA = 1;

FILE *file;
boot_record *p_boot_record;
root_directory *p_root_directory;
unsigned int **fat_table;
char *clusters;

FILE *nacti_soubor(char *file_name) {

	FILE *pom_file = fopen(file_name, "w+");

	if (pom_file == NULL) {
		printf("ERR: Non-existent file!");
	}

	fseek(pom_file, SEEK_SET, 0);

	return pom_file;

}

int nacti_boot_record(FILE *file) {

	if (fread(p_boot_record, sizeof(boot_record), 1, file) != 1) {
		free(p_boot_record);
		fclose(file);

		return 0;
	}
	return 1;
}

int nacti_root_directory(FILE *file) {
	int i;
	if (fread(p_root_directory,
			sizeof(root_directory)
					* p_boot_record->root_directory_max_entries_count, 1, file)
			!= 1) {
		free(p_boot_record);
		for (i = 0; i < p_boot_record->fat_copies; i++) {
			free(fat_table[i]);
		}
		fclose(file);

		return 0;
	}
	return 1;
}

char* nacti_cluster(FILE *file) {
	int i;
	char *cluster = malloc(
			sizeof(char) * p_boot_record->cluster_size
					* p_boot_record->cluster_count);

	if (fread(cluster,
			sizeof(char) * p_boot_record->cluster_size
					* p_boot_record->cluster_count, 1, file) != 1) {
		free(p_boot_record);
		for (i = 0; i < p_boot_record->fat_copies; i++) {
			free(fat_table[i]);
		}
		fclose(file);

		return 0;
	}

	return cluster;

}

int nacteni_fat_tabulek(FILE *file) {
	int i, j;
	for (i = 0; i < p_boot_record->fat_copies; i++) {
		unsigned int *nova_fat = (unsigned int *) malloc(
				sizeof(unsigned int) * p_boot_record->cluster_count); /* alokovani fat tabulky */

		if (fread(nova_fat, sizeof(unsigned int) * p_boot_record->cluster_count,
				1, file) != 1) { /* kdyz se to nepovede, smazani */
			free(p_boot_record);
			for (j = 0; j < i; j++) {
				free(fat_table[j]);
			}
			free(nova_fat);
			fclose(file);

			return 0;
		}

		fat_table[i] = nova_fat;
	}

	return 1;
}

int nacti_zaklad_fat(char* jmeno) {
	int i;
	file = nacti_soubor(jmeno);

	if (nacti_boot_record(file) != 0)
		return 0;

	fat_table = malloc(sizeof(unsigned int *) * p_boot_record->fat_copies);

	if (nacteni_fat_tabulek(file) != 0)
		return 0;

	p_root_directory = malloc(
			sizeof(root_directory)
					* p_boot_record->root_directory_max_entries_count);
	nacti_root_directory(file);

	clusters = nacti_cluster(file);

	fclose(file);

	return 1;
}

int uloz_fat(char *path) {
	FILE *soubor = nacti_soubor(path);

	if (fwrite(p_boot_record, sizeof(boot_record), 1, soubor) != 1) { /* Ulozeni bootu */
		fclose(file);

		return 0;
	}
	int i;
	for (i = 0; i < p_boot_record->fat_copies; i++) { /* Ulozeni fat tabulek */


		if (fwrite(fat_table[i],sizeof(unsigned int) * p_boot_record->cluster_count, 1, soubor)
				!= 1) {

			fclose(soubor);

			return 0;
		}

	}


	/* Ulozeni rootu */
	if (fwrite(p_root_directory,
			sizeof(root_directory)
					* p_boot_record->root_directory_max_entries_count, 1,
			soubor) != 1) {
		fclose(soubor);

		return 0;
	}


	/* Ulozeni clusteru*/
	if (fwrite(clusters,
			sizeof(char) * p_boot_record->cluster_size
					* p_boot_record->cluster_count, 1, soubor) != 1) {
		fclose(soubor);

		return 0;
	}


	fclose(soubor);

	return 1;
}

int vytovor_badblock(unsigned int zacatek, unsigned int konec) {

	unsigned int i,j;
	for (i = zacatek; i <= konec; i++) {
		char *cluster = clusters + i * p_boot_record->cluster_size;

		for (j = 0; j < 6; j++) {
			cluster[j] = 'F';
			cluster[p_boot_record->cluster_size - j - 2] = 'F';
		}
	}

	return 1;

}

int vytvor_testovaci_fat(char *path) {
	int i,j;
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
					sizeof(unsigned int) * p_boot_record->cluster_count); /* alokovani fat tabulky */

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
	for(i = 0; i < 10; i++){

		vytovor_badblock(i+100, i+250);

	}

	uloz_fat(path);
	printf("Vytvorena testovaci FAT\n");

	return 1;
}

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

char *najdi_jmeno(const char *start, const char *ending) {
	size_t name_length;
	char *name;

	name_length = ending - start;
	name = (char *) malloc(name_length + 1);
	memcpy(name, start, name_length);
	name[name_length] = '\0';
	return name;
}

root_directory *najdi_soubor(root_directory *slozka, char *jmeno) {

	while (slozka->file_name[0] != '\0') {
		if (strcmp(slozka->file_name, jmeno) == 0) {
			return slozka;
		}
		slozka++;
	}
	return NULL;
}
int pruchod_cestou(char** start, root_directory **soubor) {
	char *jmeno;
	char *end;
	int koren = 1;
	while (1) { /* Mezi lomitkama vemu */
		end = strchr((*start), '/');

		if (end == NULL) { /* Pokud jiz neni nalezeno '/' */
			break;
		}

		if (koren) { /* Kdyz neni v rootu, tak to skoci do slozky */
			koren = 0;
		} else {
			if ((*soubor)->file_type == TYPE_SLOZKA) {
				(*soubor) =
						(root_directory *) (clusters
								+ (*soubor)->first_cluster
										* p_boot_record->cluster_size); /* Skoci do slozky */
			} else { /* Nenalezeno*/

				return -1;
			}
		}
		jmeno = najdi_jmeno((*start), end); /* Nacteni pismen mezi lomitky */
		(*soubor) = najdi_soubor((*soubor), jmeno);
		free(jmeno);
		if ((*soubor) == NULL) { /* Not found */

			return -1;
		}

		(*start) = end + 1;
	}
	return koren;
}
int kontrola_cesty(char* cesta, root_directory **vysledny_soubor) {

	int koren = 1; /* Kdyz jsem v rootu, tak to je 1, 0 kdyz nejsem */
	char *start;

	root_directory *soubor = p_root_directory; /* Soucasna slozka */

	if (cesta[0] == '/') {
		cesta++;
	}

	if (cesta[0] == '\0') { /* Root */
		*vysledny_soubor = NULL;
		return 1;
	}

	start = cesta;
	koren = pruchod_cestou(&start, &soubor);

	if (koren == -1)
		return 0;
	if (*start != '\0') { /* Tady uz je konkretni soubor, po vsech lomenech nebo slozka */
		if (!koren) {
			if (soubor->file_type == TYPE_SLOZKA) {
				soubor = (root_directory *) (clusters
						+ soubor->first_cluster * p_boot_record->cluster_size);
			} else { /* Not found */

				return 0;
			}
		}

		soubor = najdi_soubor(soubor, start);

		if (soubor == NULL) { /* Not found */
			return 0;
		}
	}

	*vysledny_soubor = soubor;

	return 1;
}
int vytvor_slozku_soubor(root_directory **slozka) {
	if ((*slozka) == NULL) { /* Root */
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

char *ziskej_nazev(char *soubor) {
	char *jmeno_souboru = strrchr(soubor, '/'); /* tato ziskam nazev souboru na konci cesty bez lomitka */
	if (jmeno_souboru != NULL) {
		jmeno_souboru += 1;
	} else {
		jmeno_souboru = soubor; /* kdyz neni lomitko tak cely*/
	}
	return jmeno_souboru;
}

void nahraj_info(root_directory *novy_soubor, char* jmeno_souboru, int velikost,
		int prvni_cluster, int type) {
	memset(novy_soubor->file_name, 0, sizeof(novy_soubor->file_name));
	strcpy(novy_soubor->file_name, jmeno_souboru);

	novy_soubor->file_size = velikost;
	novy_soubor->file_type = type;
	novy_soubor->first_cluster = prvni_cluster;

	memset(novy_soubor + 1, 0, sizeof(novy_soubor)); /* Prida novy konec slozky do rodicovske slozky */
}
void pridej_cluster_fat(int cluster, int index) {
	int i;
	for (i = 0; i < p_boot_record->fat_copies; i++) {
		fat_table[i][index] = cluster;
	}
}

int zapis_soubor(char *soubor_pro_zapis, char *cesta) {

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

	FILE *soubor = nacti_soubor(soubor_pro_zapis);

	char *jmeno_souboru = ziskej_nazev(soubor_pro_zapis);

	while (1) { /* postupne obsazuje clustery */

		najdi_prvni_cluster(&novy_cluster);
		size_t count = fread(
				clusters + novy_cluster * p_boot_record->cluster_size, 1,
				p_boot_record->cluster_size - 1, soubor);
		if (count <= 0) {
			break;
		}
		last_count = count;

		file_size += count; /* prictu si to abych vedel jeho velikost */
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
		nahraj_info(novy_soubor, jmeno_souboru, file_size, prvni_cluster,
				TYPE_SOUBOR);
	}

	fclose(soubor);

	printf("OK\n");

	return 1;
}

int smaz_soubor(char *cesta) {

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
		unsigned int next_cluster = fat_table[0][current_cluster]; /* Zisk dalsiho clusteru */

		pridej_cluster_fat(FAT_UNUSED, current_cluster);

		current_cluster = next_cluster;
	}

	while (1) {
		*soubor = *(soubor + 1); /* Zkopiruje dalsi zaznam o souboru ve slozce -> tím se smaže */

		if (soubor->file_name[0] == '\0') {
			break;
		}

		soubor++;
	}

	printf("OK\n");

	return 1;
}
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

		unsigned int dalsi_cluster = fat_table[0][pom_cluster]; /* Zisk dalsiho clusteru */
		pom_cluster = dalsi_cluster;

		if (pom_cluster != FAT_FILE_END && pom_cluster != FAT_BAD_CLUSTER) {
			printf(",");
		}
	}

	printf("\n");

	return 1;
}

int vytvor_slozku(char *jmeno_slozky, char *cesta) {

	root_directory *slozka;
	if (!kontrola_cesty(cesta, &slozka)) { /* Otevreni souboru */
		printf("PATH NOT FOUND\n");

		return 0;
	}

	if (slozka != NULL && slozka->file_type != TYPE_SLOZKA) { /* Soubor */

		return 0;
	}

	unsigned int cluster = FAT_UNUSED;

	najdi_prvni_cluster(&cluster);

	/* Vyplneni zbytku nulama */
	memset(clusters + cluster * p_boot_record->cluster_size, 0,
			p_boot_record->cluster_size);

	pridej_cluster_fat(FAT_FILE_END, cluster);

	if (slozka == NULL) { /* Root */
		slozka = p_root_directory;
	} else {
		slozka = (root_directory *) (clusters
				+ slozka->first_cluster * p_boot_record->cluster_size);
	}

	while (slozka->file_name[0] != '\0') {
		if (strcmp(slozka->file_name, jmeno_slozky) == 0) { /* Nazev jiz existuje */
			return 0;
		}

		slozka++;
	}

	nahraj_info(slozka, jmeno_slozky, 0, cluster, TYPE_SLOZKA);
	memset(slozka + 1, 0, sizeof(root_directory)); /* Pridani prazdne slozky */

	printf("OK\n");

	return 1;
}

int smaz_prazdnou_slozku(char *cesta) {

	root_directory *slozka;
	if (!kontrola_cesty(cesta, &slozka)) { /* Otevreni souboru */
		printf("PATH NOT FOUND\n");
		return 0;
	}

	if (file == NULL) { /* Root */
		return 0;
	}

	if (slozka->file_type != TYPE_SLOZKA) { /* Soubor */
		return 0;
	}

	root_directory *pom = (root_directory *) (clusters
			+ slozka->first_cluster * p_boot_record->cluster_size);
	if (pom->file_name[0] != '\0') { /* Neni prazdny*/
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

	printf("OK\n");

	return 1;
}

int obsah_souboru(char *cesta) {

	root_directory *soubor;
	if (!kontrola_cesty(cesta, &soubor)) { /* Otevreni souboru */
		printf("PATH NOT FOUND\n");
		return 0;
	}

	printf("%s: ", soubor->file_name);

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

void print_tabs(unsigned pocet) {
	int i;
	for (i = 0; i < pocet; i++) {
		printf("\t");
	}
}
int print_content(root_directory *slozka, unsigned tabs) {

	while (slozka->file_name[0] != '\0') {
		if (slozka->file_type == TYPE_SOUBOR) {
			unsigned int pocet_clusteru = 0;

			unsigned int pom_cluster = slozka->first_cluster;
			while (pom_cluster != FAT_FILE_END && pom_cluster != FAT_BAD_CLUSTER) {

				unsigned int next_cluster = fat_table[0][pom_cluster];
				pom_cluster = next_cluster;

				pocet_clusteru++;
			}
			print_tabs(tabs);
			printf("-%s %u %u\n", slozka->file_name, slozka->first_cluster,
					pocet_clusteru);
		} else if (slozka->file_type == TYPE_SLOZKA) {
			print_tabs(tabs);
			printf("+%s %u\n", slozka->file_name, slozka->first_cluster);
			print_content(
					(root_directory *) (clusters
							+ slozka->first_cluster
									* p_boot_record->cluster_size), tabs + 1);
			print_tabs(tabs);
			printf("--\n");
		}
		slozka++;
	}

	return 1;
}

int content_of_fat() {

	root_directory *slozka = p_root_directory;
	if (slozka->file_name[0] == '\0') {
		printf("EMPTY\n");
		return 1;
	}

	printf("+ROOT\n");

	print_content(slozka, 1);
	printf("--\n");

	return 1;
}

