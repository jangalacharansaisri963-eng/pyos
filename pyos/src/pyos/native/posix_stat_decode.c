#define PY_SSIZE_T_CLEAN

#include <Python.h>
#include "posix_internal.h"

static pyos_uint64
pyos_load_u64(
    const unsigned char *data
)
{
    pyos_uint64 value;

    value = 0;

    value |= ((pyos_uint64)data[0]) << 0;
    value |= ((pyos_uint64)data[1]) << 8;
    value |= ((pyos_uint64)data[2]) << 16;
    value |= ((pyos_uint64)data[3]) << 24;
    value |= ((pyos_uint64)data[4]) << 32;
    value |= ((pyos_uint64)data[5]) << 40;
    value |= ((pyos_uint64)data[6]) << 48;
    value |= ((pyos_uint64)data[7]) << 56;

    return value;
}

static pyos_uint32
pyos_load_u32(
    const unsigned char *data
)
{
    pyos_uint32 value;

    value = 0;

    value |= ((pyos_uint32)data[0]) << 0;
    value |= ((pyos_uint32)data[1]) << 8;
    value |= ((pyos_uint32)data[2]) << 16;
    value |= ((pyos_uint32)data[3]) << 24;

    return value;
}

static pyos_int64
pyos_load_i64(
    const unsigned char *data
)
{
    return (pyos_int64)pyos_load_u64(data);
}

static pyos_int32
pyos_load_i32(
    const unsigned char *data
)
{
    return (pyos_int32)pyos_load_u32(data);
}

static int
pyos_stat_decode_linux64(
    const unsigned char *data,
    pyos_stat_result *result
)
{
    pyos_uint64 dev;
    pyos_uint64 ino;
    pyos_uint64 size;
    pyos_uint64 blocks;
    pyos_uint32 mode;
    pyos_uint32 nlink;
    pyos_uint32 uid;
    pyos_uint32 gid;
    pyos_int64 atime;
    pyos_int64 mtime;
    pyos_int64 ctime;

    dev = pyos_load_u64(data + 0);
    ino = pyos_load_u64(data + 8);
    mode = pyos_load_u32(data + 16);
    nlink = pyos_load_u32(data + 20);
    uid = pyos_load_u32(data + 24);
    gid = pyos_load_u32(data + 28);

    size = pyos_load_u64(data + 48);

    atime = pyos_load_i64(data + 72);
    mtime = pyos_load_i64(data + 88);
    ctime = pyos_load_i64(data + 104);

    blocks = pyos_load_u64(data + 64);

    (void)blocks;

    result->st_dev = dev;
    result->st_ino = ino;
    result->st_mode = mode;
    result->st_nlink = nlink;
    result->st_uid = uid;
    result->st_gid = gid;
    result->st_size = (pyos_off_t)size;
    result->st_atime = (pyos_time_t)atime;
    result->st_mtime = (pyos_time_t)mtime;
    result->st_ctime = (pyos_time_t)ctime;

    return 0;
}

int
pyos_decode_stat(
    const void *native_stat,
    pyos_stat_result *result
)
{
    const unsigned char *data;

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

    data = (const unsigned char *)native_stat;

    result->st_mode = 0;
    result->st_ino = 0;
    result->st_dev = 0;
    result->st_nlink = 0;
    result->st_uid = 0;
    result->st_gid = 0;
    result->st_size = 0;
    result->st_atime = 0;
    result->st_mtime = 0;
    result->st_ctime = 0;

    return pyos_stat_decode_linux64(
        data,
        result
    );
}

static PyObject *
pyos_stat_decode_buffer(
    PyObject *self,
    PyObject *args
)
{
    PyObject *buffer_object;
    Py_buffer view;
    pyos_stat_result result;
    int status;

    (void)self;

    if (!PyArg_ParseTuple(
        args,
        "O:decode_stat",
        &buffer_object
    )) {
        return NULL;
    }

    if (
        PyObject_GetBuffer(
            buffer_object,
            &view,
            PyBUF_SIMPLE
        ) < 0
    ) {
        return NULL;
    }

    if (
        view.len < 112
    ) {
        PyBuffer_Release(&view);

        PyErr_SetString(
            PyExc_ValueError,
            "native stat buffer is too small"
        );

        return NULL;
    }

    status = pyos_decode_stat(
        view.buf,
        &result
    );

    PyBuffer_Release(&view);

    if (status < 0) {
        return NULL;
    }

    return pyos_make_stat_dict(
        &result
    );
}

static PyObject *
pyos_stat_type(
    PyObject *self,
    PyObject *args
)
{
    PyObject *mode_object;
    pyos_mode_t mode;

    (void)self;

    if (!PyArg_ParseTuple(
        args,
        "O:stat_type",
        &mode_object
    )) {
        return NULL;
    }

    if (
        pyos_require_integer(
            mode_object,
            "mode",
            (long long *)&mode
        ) < 0
    ) {
        return NULL;
    }

    return PyLong_FromUnsignedLong(
        mode & PYOS_S_IFMT
    );
}

static PyObject *
pyos_stat_is_regular_mode(
    PyObject *self,
    PyObject *args
)
{
    PyObject *mode_object;
    long long mode;

    (void)self;

    if (!PyArg_ParseTuple(
        args,
        "O:is_regular_mode",
        &mode_object
    )) {
        return NULL;
    }

    if (
        pyos_require_integer(
            mode_object,
            "mode",
            &mode
        ) < 0
    ) {
        return NULL;
    }

    return pyos_bool_result(
        pyos_mode_is_regular(
            (pyos_mode_t)mode
        )
    );
}

static PyObject *
pyos_stat_is_directory_mode(
    PyObject *self,
    PyObject *args
)
{
    PyObject *mode_object;
    long long mode;

    (void)self;

    if (!PyArg_ParseTuple(
        args,
        "O:is_directory_mode",
        &mode_object
    )) {
        return NULL;
    }

    if (
        pyos_require_integer(
            mode_object,
            "mode",
            &mode
        ) < 0
    ) {
        return NULL;
    }

    return pyos_bool_result(
        pyos_mode_is_directory(
            (pyos_mode_t)mode
        )
    );
}

static PyObject *
pyos_stat_is_symlink_mode(
    PyObject *self,
    PyObject *args
)
{
    PyObject *mode_object;
    long long mode;

    (void)self;

    if (!PyArg_ParseTuple(
        args,
        "O:is_symlink_mode",
        &mode_object
    )) {
        return NULL;
    }

    if (
        pyos_require_integer(
            mode_object,
            "mode",
            &mode
        ) < 0
    ) {
        return NULL;
    }

    return pyos_bool_result(
        pyos_mode_is_symlink(
            (pyos_mode_t)mode
        )
    );
}

PyMethodDef pyos_stat_decode_methods[] = {
    {
        "decode_stat",
        pyos_stat_decode_buffer,
        METH_VARARGS,
        NULL
    },
    {
        "stat_type",
        pyos_stat_type,
        METH_VARARGS,
        NULL
    },
    {
        "is_regular_mode",
        pyos_stat_is_regular_mode,
        METH_VARARGS,
        NULL
    },
    {
        "is_directory_mode",
        pyos_stat_is_directory_mode,
        METH_VARARGS,
        NULL
    },
    {
        "is_symlink_mode",
        pyos_stat_is_symlink_mode,
        METH_VARARGS,
        NULL
    },
    {
        NULL,
        NULL,
        0,
        NULL
    }
};
