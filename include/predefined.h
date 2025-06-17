// clang-format off
#pragma once
#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#define SYS_LOGGER_SYSTEM       "system"
#define SYS_LOGGER_FORMAT       "log"
#define SYS_LOGGER_SIZE         4096
#define SYS_LOGGER_LOGLEVEL     2

#define SPECTRUM_MASS_BIN       1000
#define SPECTRUM_NMR_BIN        1500
#define SPECTRUM_OPTICS_BIN     2000

#define EXTERNAL_GENERAL_MAX    1024
#define EXTERNAL_NAME_MAX       1024
#define EXTERNAL_SMILES_MAX     1024

#define NET_DATABASE_PORT       5432
#define NET_API_PORT            5435
// clang-format on