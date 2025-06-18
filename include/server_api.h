#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <microhttpd.h>

extern int8_t server_api_open();
extern int8_t server_api_close();
extern int8_t server_api_status();