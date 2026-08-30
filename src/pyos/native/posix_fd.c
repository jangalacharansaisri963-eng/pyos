#define PY_SSIZE_T_CLEAN

#include <Python.h>
#include "posix_internal.h"

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

static PyObject *
pyos_fd_open(PyObject *self, PyObject *args, PyObject *kwargs)
{
    PyObject *path_object;
    PyObject *flags_object;
    PyObject *mode_object = NULL;
    pyos_path_buffer path;
    long long flags;
    long long mode = 0666;
    int fd;

    static char *keywords[] = {
        "path",
        "flags",
        "mode",
        NULL
    };

    (void)self;

    if (!PyArg_ParseTupleAndKeywords(
        args,
        kwargs,
        "OO|O:open",
        keywords,
        &path_object,
        &flags_object,
        &mode_object
    )) {
        return NULL;
    }

    if (
        pyos_require_integer(
            flags_object,
            "flags",
            &flags
        ) < 0
    ) {
        return NULL;
    }

    if (mode_object != NULL) {
        if (
            pyos_require_integer(
                mode_object,
                "mode",
                &mode
            ) < 0
        ) {
            return NULL;
        }
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
        pyos_path_validate(&path) < 0
    ) {
        pyos_path_release(&path);
        return NULL;
    }

    fd = open(
        path.data,
        (int)flags,
        (unsigned int)mode
    );

    if (fd < 0) {
        PyObject *error;

        error = pyos_raise_errno_with_path(
            path.data
        );

        pyos_path_release(&path);

        return error;
    }

    pyos_path_release(&path);

    return PyLong_FromLong(fd);
}

static PyObject *
pyos_fd_close(PyObject *self, PyObject *args)
{
    PyObject *fd_object;
    pyos_fd_t fd;

    (void)self;

    if (!PyArg_ParseTuple(
        args,
        "O:close",
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

    if (close(fd) < 0) {
        return pyos_raise_errno();
    }

    Py_RETURN_NONE;
}

static PyObject *
pyos_fd_read(PyObject *self, PyObject *args)
{
    PyObject *fd_object;
    PyObject *size_object;
    pyos_fd_t fd;
    long long size;
    PyObject *result;
    char *buffer;
    long count;

    (void)self;

    if (!PyArg_ParseTuple(
        args,
        "OO:read",
        &fd_object,
        &size_object
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
        pyos_require_integer(
            size_object,
            "size",
            &size
        ) < 0
    ) {
        return NULL;
    }

    if (size < 0) {
        PyErr_SetString(
            PyExc_ValueError,
            "negative size"
        );

        return NULL;
    }

    if (
        size > 2147483647LL
    ) {
        PyErr_SetString(
            PyExc_OverflowError,
            "read size is too large"
        );

        return NULL;
    }

    result = PyBytes_FromStringAndSize(
        NULL,
        (Py_ssize_t)size
    );

    if (result == NULL) {
        return NULL;
    }

    buffer = PyBytes_AS_STRING(result);

    Py_BEGIN_ALLOW_THREADS

    count = read(
        fd,
        buffer,
        (unsigned long)size
    );

    Py_END_ALLOW_THREADS

    if (count < 0) {
        Py_DECREF(result);
        return pyos_raise_errno();
    }

    if (
        count != (long)size
    ) {
        if (
            _PyBytes_Resize(
                &result,
                (Py_ssize_t)count
            ) < 0
        ) {
            return NULL;
        }
    }

    return result;
}

static PyObject *
pyos_fd_write(PyObject *self, PyObject *args)
{
    PyObject *fd_object;
    PyObject *data;
    pyos_fd_t fd;
    char *buffer;
    Py_ssize_t size;
    long count;

    (void)self;

    if (!PyArg_ParseTuple(
        args,
        "OO:write",
        &fd_object,
        &data
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
        !PyBytes_Check(data)
    ) {
        PyErr_SetString(
            PyExc_TypeError,
            "write() argument must be bytes"
        );

        return NULL;
    }

    buffer = PyBytes_AS_STRING(data);
    size = PyBytes_GET_SIZE(data);

    Py_BEGIN_ALLOW_THREADS

    count = write(
        fd,
        buffer,
        (unsigned long)size
    );

    Py_END_ALLOW_THREADS

    if (count < 0) {
        return pyos_raise_errno();
    }

    return PyLong_FromLong(count);
}

static PyObject *
pyos_fd_lseek(PyObject *self, PyObject *args)
{
    PyObject *fd_object;
    PyObject *offset_object;
    PyObject *whence_object;
    pyos_fd_t fd;
    long long offset;
    long long whence;
    long long result;

    (void)self;

    if (!PyArg_ParseTuple(
        args,
        "OOO:lseek",
        &fd_object,
        &offset_object,
        &whence_object
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
        pyos_require_integer(
            offset_object,
            "offset",
            &offset
        ) < 0
    ) {
        return NULL;
    }

    if (
        pyos_require_integer(
            whence_object,
            "whence",
            &whence
        ) < 0
    ) {
        return NULL;
    }

    result = lseek(
        fd,
        offset,
        (int)whence
    );

    if (result < 0) {
        return pyos_raise_errno();
    }

    return PyLong_FromLongLong(result);
}

static PyObject *
pyos_fd_fsync(PyObject *self, PyObject *args)
{
    PyObject *fd_object;
    pyos_fd_t fd;

    (void)self;

    if (!PyArg_ParseTuple(
        args,
        "O:fsync",
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

    if (fsync(fd) < 0) {
        return pyos_raise_errno();
    }

    Py_RETURN_NONE;
}

static PyObject *
pyos_fd_dup(PyObject *self, PyObject *args)
{
    PyObject *fd_object;
    pyos_fd_t fd;
    int result;

    (void)self;

    if (!PyArg_ParseTuple(
        args,
        "O:dup",
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

    result = dup(fd);

    if (result < 0) {
        return pyos_raise_errno();
    }

    return PyLong_FromLong(result);
}

static PyObject *
pyos_fd_dup2(PyObject *self, PyObject *args)
{
    PyObject *old_object;
    PyObject *new_object;
    pyos_fd_t oldfd;
    pyos_fd_t newfd;
    int result;

    (void)self;

    if (!PyArg_ParseTuple(
        args,
        "OO:dup2",
        &old_object,
        &new_object
    )) {
        return NULL;
    }

    if (
        pyos_require_fd(
            old_object,
            &oldfd
        ) < 0
    ) {
        return NULL;
    }

    if (
        pyos_require_fd(
            new_object,
            &newfd
        ) < 0
    ) {
        return NULL;
    }

    result = dup2(
        oldfd,
        newfd
    );

    if (result < 0) {
        return pyos_raise_errno();
    }

    return PyLong_FromLong(result);
}

static PyObject *
pyos_fd_pipe(PyObject *self, PyObject *args)
{
    int descriptors[2];

    (void)self;
    (void)args;

    if (pipe(descriptors) < 0) {
        return pyos_raise_errno();
    }

    return Py_BuildValue(
        "(ii)",
        descriptors[0],
        descriptors[1]
    );
}

static PyObject *
pyos_fd_isatty(PyObject *self, PyObject *args)
{
    PyObject *fd_object;
    pyos_fd_t fd;

    (void)self;

    if (!PyArg_ParseTuple(
        args,
        "O:isatty",
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

    return pyos_bool_result(
        isatty(fd) != 0
    );
}

PyMethodDef pyos_fd_methods[] = {
    {
        "open",
        (PyCFunction)pyos_fd_open,
        METH_VARARGS | METH_KEYWORDS,
        NULL
    },
    {
        "close",
        pyos_fd_close,
        METH_VARARGS,
        NULL
    },
    {
        "read",
        pyos_fd_read,
        METH_VARARGS,
        NULL
    },
    {
        "write",
        pyos_fd_write,
        METH_VARARGS,
        NULL
    },
    {
        "lseek",
        pyos_fd_lseek,
        METH_VARARGS,
        NULL
    },
    {
        "fsync",
        pyos_fd_fsync,
        METH_VARARGS,
        NULL
    },
    {
        "dup",
        pyos_fd_dup,
        METH_VARARGS,
        NULL
    },
    {
        "dup2",
        pyos_fd_dup2,
        METH_VARARGS,
        NULL
    },
    {
        "pipe",
        pyos_fd_pipe,
        METH_NOARGS,
        NULL
    },
    {
        "isatty",
        pyos_fd_isatty,
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

PyMethodDef *
pyos_get_fd_methods(void)
{
    return pyos_fd_methods;
}
