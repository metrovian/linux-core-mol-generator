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

	fclose(fptr);
	log_info("massbank spectrum migration success");
	return 0;
}

extern int8_t database_external_nmrshiftdb_migration(const char *name) {
	FILE *fptr = fopen(name, "r");
	if (!fptr) {
		log_error("failed to open nmrshiftdb spectrum");
		return -1;
	}

	char nmrshiftdb_line[EXTERNAL_GENERAL_MAX];
	char mol_name[EXTERNAL_NAME_MAX];
	char mol_inchi[EXTERNAL_INCHI_MAX];
	float mol_peaks_data[SPECTRUM_NMR_BIN];
	float data_ppm = 0;
	int32_t data_ofs = 0;
	int32_t data_resolution = 100;
	while (fgets(nmrshiftdb_line, sizeof(nmrshiftdb_line), fptr)) {
		if (strncmp(nmrshiftdb_line, "$$$$", 4) == 0) {
			fgets(nmrshiftdb_line, sizeof(nmrshiftdb_line), fptr);
			sscanf(nmrshiftdb_line, " %[^\r\n]", mol_name);
			memset(mol_inchi, 0, sizeof(mol_inchi));
			memset(mol_peaks_data, 0, sizeof(mol_peaks_data));
			data_ofs = 0;
		} else if (strncmp(nmrshiftdb_line, "> <INChI>", 9) == 0) {
			fgets(nmrshiftdb_line, sizeof(nmrshiftdb_line), fptr);
			sscanf(nmrshiftdb_line, "%[^\r\n]", mol_inchi);
		} else if (strncmp(nmrshiftdb_line, "> <Spectrum 1H 1>", 17) == 0) {
			fgets(nmrshiftdb_line, sizeof(nmrshiftdb_line), fptr);
			char *nmrshiftdb_token = strtok(nmrshiftdb_line, "|");
			while (nmrshiftdb_token != NULL) {
				sscanf(nmrshiftdb_token, "%f;%*s", &data_ppm);
				if ((int32_t)(data_ppm * 100 + 0.5) < 0) {
					data_ofs = (int32_t)(data_ppm * data_resolution + 0.5);
				}

				mol_peaks_data[(int32_t)(data_ppm * data_resolution + 0.5) - data_ofs] = 1;
				nmrshiftdb_token = strtok(NULL, "|");
			}

			database_spectrum_insert_nmr(mol_name, mol_inchi, mol_peaks_data, SPECTRUM_NMR_BIN);
			continue;
		}
	}

	fclose(fptr);
	log_info("nmrshiftdb spectrum migration success");
	return 0;
}

extern int8_t database_external_chemotion_migration(const char *name) {
	json_error_t json_error;
	json_t *json_root = json_load_file(name, 0, &json_error);
	if (!json_root) {
		log_error("failed to open chemotion spectrum");
		return -1;
	}

	json_t *json_graph = json_object_get(json_root, "@graph");
	if (!json_is_array(json_graph)) {
		log_critical("invalid chemotion spectrum");
		json_decref(json_root);
		return -1;
	}

	int32_t data_wavelength = 0;
	char data_intensity[4];
	char mol_line[EXTERNAL_GENERAL_MAX];
	char mol_command[EXTERNAL_GENERAL_MAX];
	char mol_name[EXTERNAL_NAME_MAX];
	char mol_inchi[EXTERNAL_INCHI_MAX];
	float mol_peaks_data[SPECTRUM_OPTICS_BIN];
	char *chemotion_ptr = NULL;
	char *chemotion_token = NULL;
	for (int32_t i = 0; i < json_array_size(json_graph); ++i) {
		json_t *json_item = json_array_get(json_graph, i);
		json_t *json_spectrum = json_object_get(json_item, "name");
		if (!json_is_string(json_spectrum)) {
			log_critical("invalid chemotion spectrum");
			json_decref(json_root);
			return -1;
		}

		if (strstr(json_string_value(json_spectrum), "FTIR")) {
			json_t *json_url = json_object_get(json_item, "url");
			json_t *json_part = json_object_get(json_item, "isPartOf");
			json_t *json_about = json_object_get(json_part, "about");
			json_t *json_about_array = json_array_get(json_about, 0);
			json_t *json_name = json_object_get(json_about_array, "name");
			json_t *json_inchi = json_object_get(json_about_array, "alternateName");
			memset(mol_peaks_data, 0, sizeof(mol_peaks_data));
			strncpy(mol_name, json_string_value(json_name), sizeof(mol_name));
			strncpy(mol_inchi, json_string_value(json_inchi), sizeof(mol_inchi));
			snprintf(
			    mol_command,
			    sizeof(mol_command),
			    "chromium "
			    "--headless "
			    "--disable-gpu "
			    "--dump-dom "
			    "--log-level=3 "
			    "%s |"
			    "grep -oP \"data-clipboard-text=\\\"\\\\KIR.*?cm.?–?1(?=\\\\.)\"",
			    json_string_value(json_url));

			FILE *chromium_ptr = popen(mol_command, "r");
			if (!chromium_ptr) {
				json_decref(json_root);
				log_error("failed to execute chromium command");
				return -1;
			}

			while (fgets(mol_line, sizeof(mol_line), chromium_ptr)) {
				chemotion_ptr = strchr(mol_line, '=');
				if (chemotion_ptr) {
					chemotion_token = strtok(++chemotion_ptr, ",");
					while (chemotion_token) {
						while (*chemotion_token == ' ') {
							chemotion_token++;
						}

						if (sscanf(chemotion_token, "%d (%3[^, )])", &data_wavelength, data_intensity) == 2) {
							if (data_wavelength >= 0 && data_wavelength < SPECTRUM_OPTICS_BIN) {
								if (strncmp(data_intensity, "vw", 2)) {
									mol_peaks_data[data_wavelength] = 1;
								} else if (strncmp(data_intensity, "w", 1)) {
									mol_peaks_data[data_wavelength] = 2;
								} else if (strncmp(data_intensity, "m", 1)) {
									mol_peaks_data[data_wavelength] = 3;
								} else if (strncmp(data_intensity, "s", 1)) {
									mol_peaks_data[data_wavelength] = 4;
								} else if (strncmp(data_intensity, "vs", 2)) {
									mol_peaks_data[data_wavelength] = 5;
								} else {
									pclose(chromium_ptr);
									json_decref(json_root);
									log_critical("invalid optics spectrum intensity");
									return -1;
								}
							}
						}

						chemotion_token = strtok(NULL, ",");
						continue;
					}
				}
			}

			pclose(chromium_ptr);
			database_spectrum_insert_optics(mol_name, mol_inchi, mol_peaks_data, SPECTRUM_OPTICS_BIN);
			continue;
		}
	}

	json_decref(json_root);
	log_info("chemotion spectrum migration success");
	return 0;
}