#include "server_api.h"
#include "database_spectrum.h"
#include "wrapper_openbabel.h"
#include "wrapper_spdlog.h"
#include "predefined.h"

struct MHD_Daemon *server_api = NULL;
struct MHD_Post {
	char *url;
	char *method;
	char *data;
	size_t size;
};

static char *server_api_request_mass(struct MHD_Post *post) {
	char *name[EXTERNAL_NAME_MAX];
	char *inchi[EXTERNAL_INCHI_MAX];
	float peaks_data[SPECTRUM_MASS_BIN];
	if (post->data) {
		const char *post_data = post->data;
		char *post_ptr = post->data;
		for (int32_t i = 0; i < SPECTRUM_MASS_BIN; ++i) {
			while (isspace((unsigned char)*post_data) || *post_data == ',') {
				if (*post_data == '\0') {
					log_error("failed to receive mass spectrum");
					return NULL;
				}

				post_data++;
			}

			float peak_intensity = strtof(post_data, &post_ptr);
			if (post_data == post_ptr) {
				log_error("failed to parse mass spectrum");
				return NULL;
			}

			peaks_data[i] = peak_intensity;
			post_data = post_ptr;
		}

		if (database_spectrum_select_mass(name, inchi, peaks_data, EXTERNAL_NAME_MAX, EXTERNAL_INCHI_MAX, SPECTRUM_MASS_BIN) > 0) {
			return mol_create((const char *)name, (const char *)inchi);
		}
	}

	log_critical("invalid mass spectrum");
	return NULL;
}

static char *server_api_request_nmr(struct MHD_Post *post) {
	char *name[EXTERNAL_NAME_MAX];
	char *inchi[EXTERNAL_INCHI_MAX];
	float peaks_data[SPECTRUM_NMR_BIN];
	if (post->data) {
		const char *post_data = post->data;
		char *post_ptr = post->data;
		for (int32_t i = 0; i < SPECTRUM_NMR_BIN; ++i) {
			while (isspace((unsigned char)*post_data) || *post_data == ',') {
				if (*post_data == '\0') {
					log_error("failed to receive nmr spectrum");
					return NULL;
				}

				post_data++;
			}

			float peak_intensity = strtof(post_data, &post_ptr);
			if (post_data == post_ptr) {
				log_error("failed to parse nmr spectrum");
				return NULL;
			}

			peaks_data[i] = peak_intensity;
			post_data = post_ptr;
		}

		if (database_spectrum_select_nmr(name, inchi, peaks_data, EXTERNAL_NAME_MAX, EXTERNAL_INCHI_MAX, SPECTRUM_NMR_BIN) > 0) {
			return mol_create((const char *)name, (const char *)inchi);
		}
	}

	log_critical("invalid nmr spectrum");
	return NULL;
}

static int32_t server_api_request_handler(
    void *cls,
    struct MHD_Connection *connection,
    const char *url,
    const char *method,
    const char *version,
    const char *data,
    size_t *size,
    void **con_cls) {
	if (strncmp(method, "POST", 4) == 0) {
		struct MHD_Post *con_post = *con_cls;
		if (!con_post) {
			con_post = calloc(1, sizeof(struct MHD_Post));
			if (con_post) {
				*con_cls = con_post;
				return MHD_YES;
			}

			log_error("failed to allocate request body");
			return MHD_NO;
		}

		if (*size > 0) {
			char *data_realloc = realloc(con_post->data, con_post->size + *size + 1);
			if (data_realloc) {
				con_post->data = data_realloc;
				memcpy(con_post->data + con_post->size, data, *size);
				con_post->size += *size;
				con_post->data[con_post->size] = 0;
				*size = 0;
				return MHD_YES;
			}

			free(con_post->data);
			free(con_post);
			log_error("failed to append request body");
			return MHD_NO;
		}

		if (*size == 0) {
			if (strncmp(url, "/mass", 5) == 0) {
				char *mass_mol = server_api_request_mass(con_post);
				if (!mass_mol) {
					struct MHD_Response *response =
					    MHD_create_response_from_buffer(
						0,
						NULL,
						MHD_RESPMEM_PERSISTENT);

					int32_t ret = MHD_queue_response(connection, MHD_HTTP_NOT_FOUND, response);
					MHD_destroy_response(response);
					free(con_post->data);
					free(con_post);
					log_error("failed to create molecular structure from mass spectrum");
					return ret;
				} else {
					struct MHD_Response *response =
					    MHD_create_response_from_buffer(
						strlen(mass_mol),
						(void *)mass_mol,
						MHD_RESPMEM_MUST_COPY);

					int32_t ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
					MHD_destroy_response(response);
					free(con_post->data);
					free(con_post);
					free(mass_mol);
					return ret;
				}
			} else if (strncmp(url, "/nmr", 4) == 0) {
				char *nmr_mol = server_api_request_nmr(con_post);
				if (!nmr_mol) {
					struct MHD_Response *response =
					    MHD_create_response_from_buffer(
						0,
						NULL,
						MHD_RESPMEM_PERSISTENT);

					int32_t ret = MHD_queue_response(connection, MHD_HTTP_NOT_FOUND, response);
					MHD_destroy_response(response);
					free(con_post->data);
					free(con_post);
					log_error("failed to create molecular structure from nmr spectrum");
					return ret;
				} else {
					struct MHD_Response *response =
					    MHD_create_response_from_buffer(
						strlen(nmr_mol),
						(void *)nmr_mol,
						MHD_RESPMEM_MUST_COPY);

					int32_t ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
					MHD_destroy_response(response);
					free(con_post->data);
					free(con_post);
					free(nmr_mol);
					return ret;
				}
			} else {
				free(con_post->data);
				free(con_post);
			}
		}
	}

	struct MHD_Response *response =
	    MHD_create_response_from_buffer(
		0,
		NULL,
		MHD_RESPMEM_PERSISTENT);

	int32_t ret = MHD_queue_response(connection, MHD_HTTP_NOT_FOUND, response);
	MHD_destroy_response(response);
	return ret;
}

extern int8_t server_api_open() {
	server_api =
	    MHD_start_daemon(
		MHD_USE_SELECT_INTERNALLY,
		NET_API_PORT,
		NULL,
		NULL,
		&server_api_request_handler,
		NULL,
		MHD_OPTION_END);

	if (!server_api) {
		log_error("failed to open api server");
		return -1;
	}

	log_info("api service started");
	return 0;
}

extern int8_t server_api_close() {
	if (server_api) {
		MHD_stop_daemon(server_api);
		server_api = NULL;
		log_info("api service terminated");
	}

	return 0;
}

extern int8_t server_api_status() {
	return server_api != NULL;
}