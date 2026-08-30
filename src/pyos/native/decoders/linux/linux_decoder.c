#define PY_SSIZE_T_CLEAN

#include <Python.h>
#include "linux_decoder.h"

int pyos_linux_decode_x86_64(
    const void *native_stat,
    pyos_stat_result *result
);

int pyos_linux_decode_aarch64(
    const void *native_stat,
    pyos_stat_result *result
);

int pyos_linux_decode_armv7(
    const void *native_stat,
    pyos_stat_result *result
);

int pyos_linux_decode_i386(
    const void *native_stat,
    pyos_stat_result *result
);

int pyos_linux_decode_riscv64(
    const void *native_stat,
    pyos_stat_result *result
);

int pyos_linux_decode_ppc64le(
    const void *native_stat,
    pyos_stat_result *result
);

unsigned long pyos_linux_stat_size_x86_64(void);
unsigned long pyos_linux_stat_size_aarch64(void);
unsigned long pyos_linux_stat_size_armv7(void);
unsigned long pyos_linux_stat_size_i386(void);
unsigned long pyos_linux_stat_size_riscv64(void);
unsigned long pyos_linux_stat_size_ppc64le(void);

static int
pyos_linux_architecture(void)
{
#if defined(__x86_64__)
    return 1;
#elif defined(__aarch64__)
    return 2;
#elif defined(__arm__)
    return 3;
#elif defined(__i386__)
    return 4;
#elif defined(__riscv) && (__riscv_xlen == 64)
    return 5;
#elif defined(__powerpc64__) && defined(__LITTLE_ENDIAN__)
    return 6;
#else
    return 0;
#endif
}

int
pyos_linux_decoder_init(void)
{
    if (
        pyos_linux_architecture() == 0
    ) {
        PyErr_SetString(
            PyExc_RuntimeError,
            "unsupported Linux architecture"
        );

        return -1;
    }

    return 0;
}

int
pyos_linux_decoder_available(void)
{
    return pyos_linux_architecture() != 0;
}

const char *
pyos_linux_decoder_name(void)
{
    switch (pyos_linux_architecture()) {
        case 1:
            return "x86_64";

        case 2:
            return "aarch64";

        case 3:
            return "armv7";

        case 4:
            return "i386";

        case 5:
            return "riscv64";

        case 6:
            return "ppc64le";

        default:
            return "unknown";
    }
}

unsigned long
pyos_linux_decoder_stat_size(void)
{
    switch (pyos_linux_architecture()) {
        case 1:
            return pyos_linux_stat_size_x86_64();

        case 2:
            return pyos_linux_stat_size_aarch64();

        case 3:
            return pyos_linux_stat_size_armv7();

        case 4:
            return pyos_linux_stat_size_i386();

        case 5:
            return pyos_linux_stat_size_riscv64();

        case 6:
            return pyos_linux_stat_size_ppc64le();

        default:
            return 0;
    }
}

int
pyos_linux_decode_stat(
    const void *native_stat,
    pyos_stat_result *result
)
{
    return pyos_linux_decode_stat_at(
        native_stat,
        result,
        pyos_linux_architecture()
    );
}

int
pyos_linux_decode_stat_at(
    const void *native_stat,
    pyos_stat_result *result,
    int architecture
)
{
    if (native_stat == NULL) {
        PyErr_SetString(
            PyExc_ValueError,
            "native stat buffer is NULL"
        );

        return -1;
    }

    if (result == NULL) {
        PyErr_SetString(
            PyExc_ValueError,
            "stat result is NULL"
        );

        return -1;
    }

    switch (architecture) {
        case 1:
            return pyos_linux_decode_x86_64(
                native_stat,
                result
            );

        case 2:
            return pyos_linux_decode_aarch64(
                native_stat,
                result
            );

        case 3:
            return pyos_linux_decode_armv7(
                native_stat,
                result
            );

        case 4:
            return pyos_linux_decode_i386(
                native_stat,
                result
            );

        case 5:
            return pyos_linux_decode_riscv64(
                native_stat,
                result
            );

        case 6:
            return pyos_linux_decode_ppc64le(
                native_stat,
                result
            );

        default:
            PyErr_SetString(
                PyExc_NotImplementedError,
                "unsupported Linux architecture"
            );

            return -1;
    }
}
