#define PY_SSIZE_T_CLEAN

#include <Python.h>
#include "posix_internal.h"

#include <sys/stat.h>
#include <unistd.h>

static PyObject *
pyos_stat_from_native(
    const void *native_stat
)
{
    pyos_stat_result result;

    if (
        pyos_decode_stat(
            native_stat,
            &result
        ) < 0
    ) {
        return NULL;
    }

    return pyos_make_stat_dict(
        &result
    );
}

static PyObject *
pyos_stat_path(
    PyObject *self,
    PyObject *args
)
{
    PyObject *path_object;
    pyos_path_buffer path;
    unsigned char native_stat[
        PYOS_NATIVE_STAT_SIZE
    ];

    (void)self;

    if (!PyArg_ParseTuple(
        args,
        "O:stat",
        &path_object
    )) {
        return NULL;
    }

    if (
        pyos_path_from_object(
            path_object,
            &path
        ) < 0
    ) {
        return NULL;
    }

    if (
        pyos_path_validate(
            &path
        ) < 0
    ) {
        pyos_path_release(
            &path
        );

        return NULL;
    }

    if (
        stat(
            path.data,
            native_stat
        ) < 0
    ) {
        PyObject *error;

        error = pyos_raise_errno_with_path(
            path.data
        );

        pyos_path_release(
            &path
        );

        return error;
    }

    pyos_path_release(
        &path
    );

    return pyos_stat_from_native(
        native_stat
    );
}

static PyObject *
pyos_lstat_path(
    PyObject *self,
    PyObject *args
)
{
    PyObject *path_object;
    pyos_path_buffer path;
    unsigned char native_stat[
        PYOS_NATIVE_STAT_SIZE
    ];

    (void)self;

    if (!PyArg_ParseTuple(
        args,
        "O:lstat",
        &path_object
    )) {
        return NULL;
    }

    if (
        pyos_path_from_object(
            path_object,
            &path
        ) < 0
    ) {
        return NULL;
    }

    if (
        pyos_path_validate(
            &path
        ) < 0
    ) {
        pyos_path_release(
            &path
        );

        return NULL;
    }

    if (
        lstat(
            path.data,
            native_stat
        ) < 0
    ) {
        PyObject *error;

        error = pyos_raise_errno_with_path(
            path.data
        );

        pyos_path_release(
            &path
        );

        return error;
    }

    pyos_path_release(
        &path
    );

    return pyos_stat_from_native(
        native_stat
    );
}

static PyObject *
pyos_fstat_fd(
    PyObject *self,
    PyObject *args
)
{
    PyObject *fd_object;
    pyos_fd_t fd;
    unsigned char native_stat[
        PYOS_NATIVE_STAT_SIZE
    ];

    (void)self;

    if (!PyArg_ParseTuple(
        args,
        "O:fstat",
        &fd_object
    )) {
        return NULL;
    }

    if (
        pyos_require_fd(
            fd_object,
            &fd
        ) < 0
    ) {
        return NULL;
    }

    if (
        fstat(
            fd,
            native_stat
        ) < 0
    ) {
        return pyos_raise_errno();
    }

    return pyos_stat_from_native(
        native_stat
    );
}

static PyObject *
pyos_stat_mode(
    PyObject *self,
    PyObject *args
)
{
    PyObject *path_object;
    PyObject *result;
    PyObject *mode;

    (void)self;

    if (!PyArg_ParseTuple(
        args,
        "O:stat_mode",
        &path_object
    )) {
        return NULL;
    }

    result = pyos_stat_path(
        self,
        args
    );

    if (result == NULL) {
        return NULL;
    }

    mode = PyDict_GetItemString(
        result,
        "st_mode"
    );

    if (mode == NULL) {
        Py_DECREF(result);

        PyErr_SetString(
            PyExc_RuntimeError,
            "stat result has no st_mode"
        );

        return NULL;
    }

    Py_INCREF(mode);
    Py_DECREF(result);

    return mode;
}

static PyObject *
pyos_stat_size(
    PyObject *self,
    PyObject *args
)
{
    PyObject *result;
    PyObject *size;

    result = pyos_stat_path(
        self,
        args
    );

    if (result == NULL) {
        return NULL;
    }

    size = PyDict_GetItemString(
        result,
        "st_size"
    );

    if (size == NULL) {
        Py_DECREF(result);

        PyErr_SetString(
            PyExc_RuntimeError,
            "stat result has no st_size"
        );

        return NULL;
    }

    Py_INCREF(size);
    Py_DECREF(result);

    return size;
}

static PyObject *
pyos_stat_mtime(
    PyObject *self,
    PyObject *args
)
{
    PyObject *result;
    PyObject *value;

    result = pyos_stat_path(
        self,
        args
    );

    if (result == NULL) {
        return NULL;
    }

    value = PyDict_GetItemString(
        result,
        "st_mtime"
    );

    if (value == NULL) {
        Py_DECREF(result);

        PyErr_SetString(
            PyExc_RuntimeError,
            "stat result has no st_mtime"
        );

        return NULL;
    }

    Py_INCREF(value);
    Py_DECREF(result);

    return value;
}

static PyObject *
pyos_stat_atime(
    PyObject *self,
    PyObject *args
)
{
    PyObject *result;
    PyObject *value;

    result = pyos_stat_path(
        self,
        args
    );

    if (result == NULL) {
        return NULL;
    }

    value = PyDict_GetItemString(
        result,
        "st_atime"
    );

    if (value == NULL) {
        Py_DECREF(result);

        PyErr_SetString(
            PyExc_RuntimeError,
            "stat result has no st_atime"
        );

        return NULL;
    }

    Py_INCREF(value);
    Py_DECREF(result);

    return value;
}

static PyObject *
pyos_stat_ctime(
    PyObject *self,
    PyObject *args
)
{
    PyObject *result;
    PyObject *value;

    result = pyos_stat_path(
        self,
        args
    );

    if (result == NULL) {
        return NULL;
    }

    value = PyDict_GetItemString(
        result,
        "st_ctime"
    );

    if (value == NULL) {
        Py_DECREF(result);

        PyErr_SetString(
            PyExc_RuntimeError,
            "stat result has no st_ctime"
        );

        return NULL;
    }

    Py_INCREF(value);
    Py_DECREF(result);

    return value;
}

static PyObject *
pyos_stat_isfile(
    PyObject *self,
    PyObject *args
)
{
    PyObject *result;
    PyObject *mode_object;
    long long mode;

    result = pyos_stat_path(
        self,
        args
    );

    if (result == NULL) {
        return NULL;
    }

    mode_object = PyDict_GetItemString(
        result,
        "st_mode"
    );

    if (mode_object == NULL) {
        Py_DECREF(result);

        PyErr_SetString(
            PyExc_RuntimeError,
            "stat result has no st_mode"
        );

        return NULL;
    }

    mode = PyLong_AsLongLong(
        mode_object
    );

    Py_DECREF(result);

    if (PyErr_Occurred()) {
        return NULL;
    }

    return pyos_bool_result(
        pyos_mode_is_regular(
            (pyos_mode_t)mode
        )
    );
}

static PyObject *
pyos_stat_isdir(
    PyObject *self,
    PyObject *args
)
{
    PyObject *result;
    PyObject *mode_object;
    long long mode;

    result = pyos_stat_path(
        self,
        args
    );

    if (result == NULL) {
        return NULL;
    }

    mode_object = PyDict_GetItemString(
        result,
        "st_mode"
    );

    if (mode_object == NULL) {
        Py_DECREF(result);

        PyErr_SetString(
            PyExc_RuntimeError,
            "stat result has no st_mode"
        );

        return NULL;
    }

    mode = PyLong_AsLongLong(
        mode_object
    );

    Py_DECREF(result);

    if (PyErr_Occurred()) {
        return NULL;
    }

    return pyos_bool_result(
        pyos_mode_is_directory(
            (pyos_mode_t)mode
        )
    );
}

static PyObject *
pyos_stat_islink(
    PyObject *self,
    PyObject *args
)
{
    PyObject *result;
    PyObject *mode_object;
    long long mode;

    result = pyos_lstat_path(
        self,
        args
    );

    if (result == NULL) {
        return NULL;
    }

    mode_object = PyDict_GetItemString(
        result,
        "st_mode"
    );

    if (mode_object == NULL) {
        Py_DECREF(result);

        PyErr_SetString(
            PyExc_RuntimeError,
            "stat result has no st_mode"
        );

        return NULL;
    }

    mode = PyLong_AsLongLong(
        mode_object
    );

    Py_DECREF(result);

    if (PyErr_Occurred()) {
        return NULL;
    }

    return pyos_bool_result(
        pyos_mode_is_symlink(
            (pyos_mode_t)mode
        )
    );
}

PyMethodDef pyos_stat_methods[] = {
    {
        "stat",
        pyos_stat_path,
        METH_VARARGS,
        NULL
    },
    {
        "lstat",
        pyos_lstat_path,
        METH_VARARGS,
        NULL
    },
    {
        "fstat",
        pyos_fstat_fd,
        METH_VARARGS,
        NULL
    },
    {
        "stat_mode",
        pyos_stat_mode,
        METH_VARARGS,
        NULL
    },
    {
        "stat_size",
        pyos_stat_size,
        METH_VARARGS,
        NULL
    },
    {
        "stat_mtime",
        pyos_stat_mtime,
        METH_VARARGS,
        NULL
    },
    {
        "stat_atime",
        pyos_stat_atime,
        METH_VARARGS,
        NULL
    },
    {
        "stat_ctime",
        pyos_stat_ctime,
        METH_VARARGS,
        NULL
    },
    {
        "stat_isfile",
        pyos_stat_isfile,
        METH_VARARGS,
        NULL
    },
    {
        "stat_isdir",
        pyos_stat_isdir,
        METH_VARARGS,
        NULL
    },
    {
        "stat_islink",
        pyos_stat_islink,
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
