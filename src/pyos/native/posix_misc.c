#define PY_SSIZE_T_CLEAN

#include <Python.h>

#include "posix_internal.h"

static PyObject *
pyos_misc_error(
    const char *operation
)
{
    int code;
    const char *message;

    code = pyos_get_errno();
    message = NULL;

    if (pyos_native_strerror != NULL)
        message = pyos_native_strerror(code);

    if (message == NULL)
        message = "native operation failed";

    PyErr_Format(
        PyExc_OSError,
        "[%s] %s",
        operation,
        message
    );

    return NULL;
}

static PyObject *
pyos_misc_sync(
    PyObject *self,
    PyObject *args
)
{
    (void)self;
    (void)args;

    pyos_native_sync();

    Py_RETURN_NONE;
}

static PyObject *
pyos_misc_syncfs(
    PyObject *self,
    PyObject *args
)
{
    int fd;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "i:syncfs",
            &fd
        )
    )
        return NULL;

    if (
        pyos_native_syncfs(fd) < 0
    )
        return pyos_misc_error(
            "syncfs"
        );

    Py_RETURN_NONE;
}

static PyObject *
pyos_misc_fsync(
    PyObject *self,
    PyObject *args
)
{
    int fd;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "i:fsync",
            &fd
        )
    )
        return NULL;

    if (
        pyos_native_fsync(fd) < 0
    )
        return pyos_misc_error(
            "fsync"
        );

    Py_RETURN_NONE;
}

static PyObject *
pyos_misc_fdatasync(
    PyObject *self,
    PyObject *args
)
{
    int fd;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "i:fdatasync",
            &fd
        )
    )
        return NULL;

    if (
        pyos_native_fdatasync(fd) < 0
    )
        return pyos_misc_error(
            "fdatasync"
        );

    Py_RETURN_NONE;
}

static PyObject *
pyos_misc_umask(
    PyObject *self,
    PyObject *args
)
{
    unsigned int mask;
    unsigned int previous;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "I:umask",
            &mask
        )
    )
        return NULL;

    previous = pyos_native_umask(
        mask
    );

    return PyLong_FromUnsignedLong(
        previous
    );
}

static PyObject *
pyos_misc_nice(
    PyObject *self,
    PyObject *args
)
{
    int increment;
    long result;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "i:nice",
            &increment
        )
    )
        return NULL;

    result = pyos_native_nice(
        increment
    );

    if (
        result == -1 &&
        pyos_get_errno() != 0
    )
        return pyos_misc_error(
            "nice"
        );

    return PyLong_FromLong(result);
}

static PyObject *
pyos_misc_getpriority(
    PyObject *self,
    PyObject *args
)
{
    int which;
    int who;
    int result;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "ii:getpriority",
            &which,
            &who
        )
    )
        return NULL;

    result = pyos_native_getpriority(
        which,
        who
    );

    if (
        result == -1 &&
        pyos_get_errno() != 0
    )
        return pyos_misc_error(
            "getpriority"
        );

    return PyLong_FromLong(result);
}

static PyObject *
pyos_misc_setpriority(
    PyObject *self,
    PyObject *args
)
{
    int which;
    int who;
    int priority;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "iii:setpriority",
            &which,
            &who,
            &priority
        )
    )
        return NULL;

    if (
        pyos_native_setpriority(
            which,
            who,
            priority
        ) < 0
    )
        return pyos_misc_error(
            "setpriority"
        );

    Py_RETURN_NONE;
}

static PyObject *
pyos_misc_getpagesize(
    PyObject *self,
    PyObject *args
)
{
    long value;

    (void)self;
    (void)args;

    value = pyos_native_getpagesize();

    if (value <= 0)
        return pyos_misc_error(
            "getpagesize"
        );

    return PyLong_FromLong(value);
}

static PyObject *
pyos_misc_getcwd(
    PyObject *self,
    PyObject *args
)
{
    char buffer[
        PYOS_PATH_MAX
    ];
    int result;

    (void)self;
    (void)args;

    result = pyos_native_getcwd(
        buffer,
        sizeof(buffer)
    );

    if (result < 0)
        return pyos_misc_error(
            "getcwd"
        );

    return PyUnicode_FromString(
        buffer
    );
}

static PyObject *
pyos_misc_getenv(
    PyObject *self,
    PyObject *args
)
{
    const char *name;
    const char *value;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "s:getenv",
            &name
        )
    )
        return NULL;

    value = pyos_native_getenv(
        name
    );

    if (value == NULL)
        Py_RETURN_NONE;

    return PyUnicode_FromString(
        value
    );
}

static PyObject *
pyos_misc_setenv(
    PyObject *self,
    PyObject *args
)
{
    const char *name;
    const char *value;
    int overwrite;

    (void)self;

    overwrite = 1;

    if (
        !PyArg_ParseTuple(
            args,
            "ss|p:setenv",
            &name,
            &value,
            &overwrite
        )
    )
        return NULL;

    if (
        pyos_native_setenv(
            name,
            value,
            overwrite
        ) < 0
    )
        return pyos_misc_error(
            "setenv"
        );

    Py_RETURN_NONE;
}

static PyObject *
pyos_misc_unsetenv(
    PyObject *self,
    PyObject *args
)
{
    const char *name;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "s:unsetenv",
            &name
        )
    )
        return NULL;

    if (
        pyos_native_unsetenv(name) < 0
    )
        return pyos_misc_error(
            "unsetenv"
        );

    Py_RETURN_NONE;
}

static PyObject *
pyos_misc_strerror(
    PyObject *self,
    PyObject *args
)
{
    int code;
    const char *message;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "i:strerror",
            &code
        )
    )
        return NULL;

    message = pyos_native_strerror(
        code
    );

    if (message == NULL) {
        PyErr_Format(
            PyExc_ValueError,
            "unknown errno %d",
            code
        );

        return NULL;
    }

    return PyUnicode_FromString(
        message
    );
}

static PyObject *
pyos_misc_errno(
    PyObject *self,
    PyObject *args
)
{
    (void)self;
    (void)args;

    return PyLong_FromLong(
        pyos_get_errno()
    );
}

static PyObject *
pyos_misc_getloadavg(
    PyObject *self,
    PyObject *args
)
{
    double values[3];
    int count;
    int index;
    PyObject *result;

    (void)self;
    (void)args;

    count = pyos_native_getloadavg(
        values,
        3
    );

    if (count < 0)
        return pyos_misc_error(
            "getloadavg"
        );

    result = PyTuple_New(count);

    if (result == NULL)
        return NULL;

    for (
        index = 0;
        index < count;
        index++
    ) {
        PyObject *value;

        value = PyFloat_FromDouble(
            values[index]
        );

        if (value == NULL) {
            Py_DECREF(result);
            return NULL;
        }

        PyTuple_SET_ITEM(
            result,
            index,
            value
        );
    }

    return result;
}

static PyMethodDef pyos_misc_methods[] = {
    {
        "sync",
        pyos_misc_sync,
        METH_NOARGS,
        NULL
    },
    {
        "syncfs",
        pyos_misc_syncfs,
        METH_VARARGS,
        NULL
    },
    {
        "fsync",
        pyos_misc_fsync,
        METH_VARARGS,
        NULL
    },
    {
        "fdatasync",
        pyos_misc_fdatasync,
        METH_VARARGS,
        NULL
    },
    {
        "umask",
        pyos_misc_umask,
        METH_VARARGS,
        NULL
    },
    {
        "nice",
        pyos_misc_nice,
        METH_VARARGS,
        NULL
    },
    {
        "getpriority",
        pyos_misc_getpriority,
        METH_VARARGS,
        NULL
    },
    {
        "setpriority",
        pyos_misc_setpriority,
        METH_VARARGS,
        NULL
    },
    {
        "getpagesize",
        pyos_misc_getpagesize,
        METH_NOARGS,
        NULL
    },
    {
        "getcwd",
        pyos_misc_getcwd,
        METH_NOARGS,
        NULL
    },
    {
        "getenv",
        pyos_misc_getenv,
        METH_VARARGS,
        NULL
    },
    {
        "setenv",
        pyos_misc_setenv,
        METH_VARARGS,
        NULL
    },
    {
        "unsetenv",
        pyos_misc_unsetenv,
        METH_VARARGS,
        NULL
    },
    {
        "strerror",
        pyos_misc_strerror,
        METH_VARARGS,
        NULL
    },
    {
        "errno",
        pyos_misc_errno,
        METH_NOARGS,
        NULL
    },
    {
        "getloadavg",
        pyos_misc_getloadavg,
        METH_NOARGS,
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
pyos_get_misc_methods(void)
{
    return pyos_misc_methods;
}
