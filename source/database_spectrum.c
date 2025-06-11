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
		log_error("failed to database_spectrumect database");
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

extern int8_t database_spectrum_insert_molecule(const char *name, const char *smiles) {
	const char *params_query[2] = {name, smiles};
	PGresult *result_query =
	    PQexecParams(
		database_spectrum,
		"INSERT INTO molecule (name, smiles) VALUES ($1, $2)",
		2,
		NULL,
		params_query,
		NULL,
		NULL,
		0);

	if (PQresultStatus(result_query) != PGRES_COMMAND_OK) {
		PQclear(result_query);
		log_error("failed to insert molecule (%s, %s)", name, smiles);
		return -1;
	}

	PQclear(result_query);
	log_info("molecule insert success (%s, %s)", name, smiles);
	return 0;
}

extern int8_t database_spectrum_insert_mass(const char *name, const char *smiles, float *peaks_data, int32_t peaks_number) {
	PGresult *result_query = PQexec(database_spectrum, "BEGIN");
	if (PQresultStatus(result_query) != PGRES_COMMAND_OK) {
		PQclear(result_query);
		log_error("failed to begin transaction");
		log_error("failed to insert mass spectrum (%s, %s)", name, smiles);
		return -1;
	}

	const char *params_query[2] = {name, smiles};
	result_query =
	    PQexecParams(
		database_spectrum,
		"SELECT id FROM molecule WHERE name = $1 AND smiles = $2",
		2,
		NULL,
		params_query,
		NULL,
		NULL,
		0);

	if (PQresultStatus(result_query) != PGRES_TUPLES_OK) {
		PQclear(result_query);
		PQexec(database_spectrum, "ROLLBACK");
		log_error("failed to select molecule (%s, %s)", name, smiles);
		log_error("failed to insert mass spectrum (%s, %s)", name, smiles);
		return -1;
	}

	if (!PQntuples(result_query)) {
		PQclear(result_query);
		result_query =
		    PQexecParams(
			database_spectrum,
			"INSERT INTO molecule (name, smiles) VALUES ($1, $2) RETURNING id",
			2,
			NULL,
			params_query,
			NULL,
			NULL,
			0);

		if (PQresultStatus(result_query) != PGRES_TUPLES_OK) {
			PQclear(result_query);
			PQexec(database_spectrum, "ROLLBACK");
			log_error("failed to insert molecule (%s, %s)", name, smiles);
			log_error("failed to insert mass spectrum (%s, %s)", name, smiles);
			return -1;
		}

		log_info("molecule insert success (%s, %s)", name, smiles);
	}

	char molecule_id[32];
	char molecule_vector[4096];
	char *ptr_vector = molecule_vector;
	snprintf(molecule_id, sizeof(molecule_id), "%d", atoi(PQgetvalue(result_query, 0, 0)));
	ptr_vector += sprintf(ptr_vector, "[");
	for (int32_t i = 0; i < peaks_number; i++) {
		ptr_vector += sprintf(ptr_vector, "%.0f", peaks_data[i]);
		if (i < peaks_number - 1) {
			ptr_vector += sprintf(ptr_vector, ",");
		}
	}

	sprintf(ptr_vector, "]");
	const char *param_query_mol[2] = {(const char *)(molecule_id), (const char *)(molecule_vector)};
	int32_t length_query_mol[2] = {strlen(molecule_id), strlen(molecule_vector)};
	int32_t format_query_mol[2] = {0, 0};
	Oid type_query_mol[2] = {23, 0};
	PQclear(result_query);
	result_query =
	    PQexecParams(
		database_spectrum,
		"INSERT INTO mass_spectrum (molecule_id, spectrum_vector) VALUES ($1, $2)",
		2,
		type_query_mol,
		param_query_mol,
		length_query_mol,
		format_query_mol,
		0);

	if (PQresultStatus(result_query) != PGRES_COMMAND_OK) {
		PQclear(result_query);
		PQexec(database_spectrum, "ROLLBACK");
		log_error("failed to insert mass spectrum (%s, %s)", name, smiles);
		return -1;
	}

	PQclear(result_query);
	result_query = PQexec(database_spectrum, "COMMIT");
	if (PQresultStatus(result_query) != PGRES_COMMAND_OK) {
		PQclear(result_query);
		log_error("failed to commit transaction");
		log_error("failed to insert mass spectrum (%s, %s)", name, smiles);
		return -1;
	}

	PQclear(result_query);
	log_info("mass spectrum insert success (%s, %s)", name, smiles);
	return 0;
}