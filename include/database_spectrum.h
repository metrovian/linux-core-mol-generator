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
extern int8_t database_spectrum_insert_molecule(const char *name, const char *smiles);
extern int8_t database_spectrum_insert_mass(const char *name, const char *smiles, float *peaks_data, int32_t peaks_number);
extern float database_spectrum_select_mass(char *name, char *smiles, float *peaks_data, int32_t name_size, int32_t smiles_size, int32_t peaks_number);