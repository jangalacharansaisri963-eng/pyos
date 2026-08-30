#define PY_SSIZE_T_CLEAN

#include <Python.h>
#include <sys/stat.h>
#include "linux_decoder.h"

unsigned long
pyos_linux_stat_size_riscv64(void)
{
    return (unsigned long)sizeof(struct stat);
}

int
pyos_linux_decode_riscv64(
    const void *native_stat,
    pyos_stat_result *result
)
{
    const struct stat *source;

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

    source = (const struct stat *)native_stat;

    result->st_mode =
        (pyos_mode_t)source->st_mode;

    result->st_ino =
        (pyos_ino_t)source->st_ino;

    result->st_dev =
        (pyos_dev_t)source->st_dev;

    result->st_nlink =
        (pyos_nlink_t)source->st_nlink;

    result->st_uid =
        (pyos_uid_t)source->st_uid;

    result->st_gid =
        (pyos_gid_t)source->st_gid;

    result->st_size =
        (pyos_off_t)source->st_size;

    result->st_atime =
        (pyos_time_t)source->st_atim.tv_sec;

    result->st_mtime =
        (pyos_time_t)source->st_mtim.tv_sec;

    result->st_ctime =
        (pyos_time_t)source->st_ctim.tv_sec;

    return 0;
}
