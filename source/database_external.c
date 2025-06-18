#include "database_external.h"
#include "database_spectrum.h"
#include "wrapper_spdlog.h"
#include "predefined.h"

extern int8_t database_external_massbank_migration(const char *name) {
	FILE *fptr = fopen(name, "r");
	if (!fptr) {
		log_error("failed to open massbank spectrum");
		return -1;
	}

	char massbank_line[EXTERNAL_GENERAL_MAX];
	char mol_name[EXTERNAL_NAME_MAX];
	char mol_inchi[EXTERNAL_INCHI_MAX];
	char mol_peaks_number[EXTERNAL_GENERAL_MAX];
	float mol_peaks_data[SPECTRUM_MASS_BIN];
	float data_mzrate = 0;
	int32_t data_intensity = 0;
	int32_t peaks_number = 0;
	while (fgets(massbank_line, sizeof(massbank_line), fptr)) {
		if (strncmp(massbank_line, "Name: ", 6) == 0) {
			memset(mol_name, 0, sizeof(mol_name));
			memset(mol_inchi, 0, sizeof(mol_inchi));
			memset(mol_peaks_number, 0, sizeof(mol_peaks_number));
			memset(mol_peaks_data, 0, sizeof(mol_peaks_data));
			sscanf(massbank_line + 6, " %[^\n]", mol_name);
		} else if (strncmp(massbank_line, "InChI: ", 7) == 0) {
			sscanf(massbank_line + 7, " %[^\n]", mol_inchi);
		} else if (strncmp(massbank_line, "Num Peaks: ", 11) == 0) {
			sscanf(massbank_line + 11, " %[^\n]", mol_peaks_number);
			peaks_number = atoi(mol_peaks_number);
			while (peaks_number--) {
				fgets(massbank_line, sizeof(massbank_line), fptr);
				sscanf(massbank_line, "%f %d", &data_mzrate, &data_intensity);
				mol_peaks_data[(int32_t)(data_mzrate + 0.5)] = data_intensity;
			}

			database_spectrum_insert_mass(mol_name, mol_inchi, mol_peaks_data, SPECTRUM_MASS_BIN);
			continue;
		}
	}

	log_info("massbank spectrum migration success");
	return 0;
}