#include "database_spectrum.h"
#include "wrapper_spdlog.h"
#include "predefined.h"

static PGconn *database_spectrum = NULL;

extern int8_t database_spectrum_open() {
	char command_open[512];
	snprintf(
	    command_open,
	    sizeof(command_open),
	    "host=localhost "
	    "port=%d "
	    "dbname=spectrum "
	    "user=postgres ",
	    NET_POSTGRESQL_PORT);

	database_spectrum = PQconnectdb(command_open);
	if (PQstatus(database_spectrum) != CONNECTION_OK) {
		PQfinish(database_spectrum);
		log_error("failed to connect database");
		return -1;
	}

	log_info("postgresql service started");
	return 0;
}

extern int8_t database_spectrum_close() {
	if (database_spectrum) {
		PQfinish(database_spectrum);
		log_info("postgresql service terminated");
	}

	return 0;
}
