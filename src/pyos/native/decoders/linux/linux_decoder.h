#ifndef PYOS_LINUX_DECODER_H
#define PYOS_LINUX_DECODER_H

#include <Python.h>
#include "../../posix_internal.h"

typedef struct {
    const char *name;
    int available;
    unsigned long stat_size;
} pyos_linux_decoder_info;

int pyos_linux_decoder_init(void);

int pyos_linux_decoder_available(void);

const char *pyos_linux_decoder_name(void);

unsigned long pyos_linux_decoder_stat_size(void);

int pyos_linux_decode_stat(
    const void *native_stat,
    pyos_stat_result *result
);

int pyos_linux_decode_stat_at(
    const void *native_stat,
    pyos_stat_result *result,
    int architecture
);

#endif
