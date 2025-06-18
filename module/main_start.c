#include "predefined.h"
#include "database_external.h"
#include "database_spectrum.h"
#include "server_api.h"
#include "wrapper_openbabel.h"
#include "wrapper_spdlog.h"
#include "predefined.h"

void HANDLE_SIGINT(int32_t signal) {
	server_api_close();
	database_spectrum_close();
	log_warn("SIGINT");
	return;
}

int32_t main(int32_t argc, char *argv[]) {
	signal(SIGINT, HANDLE_SIGINT);
	database_spectrum_open();
	server_api_open();
	while (database_spectrum_status() || server_api_status()) {
		continue;
	}

	return 0;
}