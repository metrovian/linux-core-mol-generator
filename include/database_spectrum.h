#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <postgresql/libpq-fe.h>

extern int8_t database_spectrum_open();
extern int8_t database_spectrum_close();
extern int8_t database_spectrum_status();

extern int8_t database_spectrum_insert_molecule(const char *name, const char *inchi);
extern int8_t database_spectrum_insert_mass(const char *name, const char *inchi, float *peaks_data, int32_t peaks_number);
extern int8_t database_spectrum_insert_nmr(const char *name, const char *inchi, float *peaks_data, int32_t peaks_number);
extern int8_t database_spectrum_insert_optics(const char *name, const char *inchi, float *peaks_data, int32_t peaks_number);

extern float database_spectrum_select_mass(char *name, char *inchi, float *peaks_data, int32_t name_size, int32_t inchi_size, int32_t peaks_number);
extern float database_spectrum_select_nmr(char *name, char *inchi, float *peaks_data, int32_t name_size, int32_t inchi_size, int32_t peaks_number);
extern float database_spectrum_select_optics(char *name, char *inchi, float *peaks_data, int32_t name_size, int32_t inchi_size, int32_t peaks_number);