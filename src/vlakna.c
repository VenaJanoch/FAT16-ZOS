#include <pthread.h>
#include "vlakna.h"
#include "fat.h"

#define POCET_VLAKEN 4       /* Pocet vlaken */

/**
 * static root_directory * ziskej_soubor(root_directory *slozka, unsigned int tabs,
 unsigned int cluster)
 * Funkce pro nalezení slozky a souboru
 */
static root_directory * ziskej_soubor(root_directory *slozka, unsigned int tabs,
		unsigned int cluster) {

	while (slozka->file_name[0] != '\0') {

		if (slozka->first_cluster == cluster) {
			return slozka;
		}

		if (slozka->file_type == TYPE_SOUBOR) {
			unsigned int pocet_clusteru = 0;

			unsigned int pom_cluster = slozka->first_cluster;
			while (pom_cluster != FAT_FILE_END && pom_cluster != FAT_BAD_CLUSTER) {

				unsigned int dalsi_cluster = fat_table[0][pom_cluster];
				pom_cluster = dalsi_cluster;

				pocet_clusteru++;
			}

		} else if (slozka->file_type == TYPE_SLOZKA) {
			root_directory * s;

			if ((s = ziskej_soubor(
					(root_directory *) (clusters
							+ slozka->first_cluster
									* p_boot_record->cluster_size), tabs + 1,
					cluster))) {
				return s;
			}
		}
		slozka++;
	}

	return slozka;
}

/**
 *  int zkontroluj_block(char * cluster, int j)
 *  Pomocna funkce pro nalezeni vadného blocku
 */
int zkontroluj_block(char * cluster, int j) {
	char* pom = clusters + p_boot_record->cluster_size * j;
	strcpy(pom, cluster);

	unsigned int i;
	for (i = 0; i < 6; i++) {
		if (pom[i] != 'F' || pom[p_boot_record->cluster_size - i - 2] != 'F') {

			return 0;
		}

	}
	printf("badblock %u\n", j);
	return 1;
}

/**
 *  void zaber_cluster(unsigned int index, int i)
 *  Pomocna funkce pro zabrani blocku ve fat pri nalezeni badblocku a naslednem presunu dat
 */
void zaber_cluster(unsigned int index, int i) {

	root_directory * name = ziskej_soubor(p_root_directory, 0, i);
	int j;

	for (j = 0; j < p_boot_record->fat_copies; j++) {
		name->first_cluster = index;
	}
}

/**
 *  void vymaz_badblock(char* cluster, unsigned int index)
 *  Pomocna funkce pro vymazani znaku oznacujici badblock
 */
void vymaz_badblock(char* cluster, unsigned int index) {
	int j;

	char *novy_cluster = clusters + p_boot_record->cluster_size * index;

	memcpy(novy_cluster, cluster, p_boot_record->cluster_size);

	for (j = 0; j < 6; j++) {
		novy_cluster[j] = ' ';
		novy_cluster[p_boot_record->cluster_size - j - 2] = ' ';
	}
}

/**
 *  void prirad_soubor(unsigned int index, int i)
 *  Pomocna funkce pro vytvoreni souboru s oznacenim vadneho badblocku
 */

void prirad_soubor(unsigned int index, int i) {
	char jmeno[FILE_NAME_LENGTH];
	int n = snprintf(jmeno, FILE_NAME_LENGTH, "nah_%u", i);

	if (n < 0 || n >= sizeof jmeno)
		exit(0);

	root_directory *soubor;
	int file_exists;
	unsigned int pokus = 0;

	do {
		soubor = p_root_directory;
		file_exists = 0;

		while (soubor->file_name[0] != '\0') {
			if (strcmp(soubor->file_name, jmeno) == 0) {

				file_exists = 1;
				pokus++;

				n = snprintf(jmeno, FILE_NAME_LENGTH, "nah_%u_%u", i, pokus);
				if (n < 0 || n >= sizeof jmeno)
					exit(0);
				break;
			}

			soubor++;
		}
	} while (file_exists);

	memset(soubor->file_name, 0, sizeof(soubor->file_name));
	strcpy(soubor->file_name, jmeno);

	soubor->file_size = p_boot_record->cluster_size - 1;
	soubor->file_type = TYPE_SOUBOR;
	soubor->first_cluster = index;

	memset(soubor + 1, 0, sizeof(root_directory));

}
/**
 * void *oprav_bloky_worker(void *data_)
 * Funkce predstavujici workera, vykonavající procházení svého úseku fat  a nacházení badblocku
 *
 */
void *oprav_bloky_worker(void *data_) {

	unsigned int i, j;
	worker_struct *data = (worker_struct *) data_;

	for (i = data->first_cluster; i < data->last_cluster; i++) {

		if (fat_table[0][i] == FAT_BAD_CLUSTER) {
			continue;
		}

		char *cluster = clusters + p_boot_record->cluster_size * i;

		int badblock = zkontroluj_block(cluster, i);

		if (badblock) {

			unsigned int next_cluster = fat_table[0][i];
			pridej_cluster_fat(FAT_BAD_CLUSTER, i);

			pthread_mutex_lock(data->allocation_mutex);

			unsigned int cluster_index;
			unsigned int predchozi = i;

			if (najdi_prvni_cluster(&cluster_index)) {

				if (!najdi_predchozi_cluster(&predchozi)) {

					zaber_cluster(cluster_index, i);
				} else {

					pridej_cluster_fat(cluster_index, predchozi);
				}

				pridej_cluster_fat(next_cluster, cluster_index);

				pthread_mutex_unlock(data->allocation_mutex);

				vymaz_badblock(cluster, cluster_index);

				prirad_soubor(cluster_index, i);

			} else {

				pthread_mutex_unlock(data->allocation_mutex);
			}
		}
	}

	return NULL;
}

/**
 * int oprav_blok(char* fat)
 * Funkce predstavujici Farmare pro rozdeleni prace Workerum
 *
 */
int oprav_blok(char* fat) {

	worker_struct threads[POCET_VLAKEN];
	pthread_mutex_t allocation_mutex;
	pthread_mutex_init(&allocation_mutex, NULL);

	unsigned int number_of_clusters_per_thread = (p_boot_record->cluster_count
			+ POCET_VLAKEN - 1) / POCET_VLAKEN;
	int i;
	for (i = 0; i < POCET_VLAKEN; i++) {

		threads[i].first_cluster = number_of_clusters_per_thread * i;
		threads[i].last_cluster = number_of_clusters_per_thread * (i + 1);
		threads[i].allocation_mutex = &allocation_mutex;
		pthread_create(&threads[i].thread, NULL, oprav_bloky_worker,
				&threads[i]);
	}

	for (i = 0; i < POCET_VLAKEN; i++) {
		void *prazdno;
		pthread_join(threads[i].thread, &prazdno);
	}
	printf("konec\n");

	pthread_mutex_destroy(&allocation_mutex);

	return 1;
}

