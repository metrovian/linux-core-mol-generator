#include "database_external.h"
#include "database_spectrum.h"
#include "server_api.h"
#include "wrapper_openbabel.h"
#include "wrapper_spdlog.h"
#include "predefined.h"

int32_t main(int32_t argc, char *argv[]) {
	if (argc == 3) {
		database_spectrum_open();
		if (strncmp(argv[1], "massbank", 8) == 0) {
			database_external_massbank_migration(argv[2]);
			database_spectrum_close();
			return 0;
		} else if (strncmp(argv[1], "chemotion", 9) == 0) {
			database_external_chemotion_migration(argv[2]);
			database_spectrum_close();
			return 0;
		} else if (strncmp(argv[1], "nmrshiftdb", 10) == 0) {
			database_external_nmrshiftdb_migration(argv[2]);
			database_spectrum_close();
			return 0;
		} else {
			database_spectrum_close();
			log_critical("invalid parameters");
			return -1;
		}
	} else {
		log_critical("invalid parameters");
		return -1;
	}

	return 0;
}