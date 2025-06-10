#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <postgresql/libpq-fe.h>

extern int8_t database_spectrum_open();
extern int8_t database_spectrum_close();