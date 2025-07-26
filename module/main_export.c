#include "database_external.h"
#include "database_spectrum.h"
#include "server_api.h"
#include "wrapper_openbabel.h"
#include "wrapper_spdlog.h"
#include "predefined.h"

int32_t main(int32_t argc, char *argv[]) {
	char name[EXTERNAL_NAME_MAX];
	char inchi[EXTERNAL_INCHI_MAX];
	database_spectrum_open();
	while (database_spectrum_select_molecule(name, inchi, EXTERNAL_NAME_MAX, EXTERNAL_INCHI_MAX) > 0) {
		char *mol_data = mol_create(name, inchi);
		if (!mol_data) {
			continue;
		}

		char *mol_path = (char *)malloc(strlen(name) + 5);
		if (!mol_path) {
			database_spectrum_close();
			log_error("failed to allocate export path");
			return -1;
		}

		int32_t i = 0;
		int32_t j = 0;
		snprintf(mol_path, strlen(name) + 5, "%s%s", name, ".mol");
		for (i = 0; i < strlen(name) + 5; ++i) {
			if (mol_path[i] == '/' || mol_path[i] == ' ') {
				log_debug("path trim success");
				continue;
			} else if (mol_path[i] == '\0') {
				log_debug("path trim terminated");
				break;
			}

			mol_path[j++] = mol_path[i];
		}

		mol_path[j] = '\0';
		FILE *mol_fptr = fopen(mol_path, "w");
		if (!mol_fptr) {
			database_spectrum_close();
			log_error("failed to open molfile");
			return -1;
		}

		fwrite(mol_data, sizeof(char), strlen(mol_data), mol_fptr);
		fclose(mol_fptr);
		free(mol_data);
		free(mol_path);
	}

	system("tar -czf molfiles.tar.gz *.mol");
	system("rm -f *.mol");
	database_spectrum_close();
	return 0;
}