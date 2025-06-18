#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
extern char *mol_create(const char *name, const char *inchi);
extern char *mol_hash(const char *inchi);
#ifdef __cplusplus
}
#endif