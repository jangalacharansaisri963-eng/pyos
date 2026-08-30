#define PY_SSIZE_T_CLEAN

#include <Python.h>

#include "posix_internal.h"

static PyObject *
pyos_resource_error(
    const char *operation
)
{
    int code;
    const char *message;

    code = pyos_get_errno();
    message = NULL;

    if (pyos_native_strerror != NULL) {
        message = pyos_native_strerror(code);
    }

    if (message == NULL) {
        message = "native resource operation failed";
    }

    PyErr_Format(
        PyExc_OSError,
        "[%s] %s",
        operation,
        message
    );

    return NULL;
}

static PyObject *
pyos_resource_getrlimit(
    PyObject *self,
    PyObject *args
)
{
    int resource;
    unsigned long long soft;
    unsigned long long hard;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "i:getrlimit",
            &resource
        )
    ) {
        return NULL;
    }

    if (
        pyos_native_getrlimit(
            resource,
            &soft,
            &hard
        ) < 0
    ) {
        return pyos_resource_error(
            "getrlimit"
        );
    }

    return Py_BuildValue(
        "(KK)",
        soft,
        hard
    );
}

static PyObject *
pyos_resource_setrlimit(
    PyObject *self,
    PyObject *args
)
{
    int resource;
    unsigned long long soft;
    unsigned long long hard;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "iKK:setrlimit",
            &resource,
            &soft,
            &hard
        )
    ) {
        return NULL;
    }

    if (
        pyos_native_setrlimit(
            resource,
            soft,
            hard
        ) < 0
    ) {
        return pyos_resource_error(
            "setrlimit"
        );
    }

    Py_RETURN_NONE;
}

static PyObject *
pyos_resource_getrusage(
    PyObject *self,
    PyObject *args
)
{
    int who;
    pyos_resource_usage usage;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "i:getrusage",
            &who
        )
    ) {
        return NULL;
    }

    if (
        pyos_native_getrusage(
            who,
            &usage
        ) < 0
    ) {
        return pyos_resource_error(
            "getrusage"
        );
    }

    return Py_BuildValue(
        "{s:L,s:L,s:L,s:L,s:L,s:L,s:L,s:L,s:L,s:L}",
        "user_seconds",
        usage.user_seconds,
        "user_microseconds",
        usage.user_microseconds,
        "system_seconds",
        usage.system_seconds,
        "system_microseconds",
        usage.system_microseconds,
        "max_rss",
        usage.max_rss,
        "minor_faults",
        usage.minor_faults,
        "major_faults",
        usage.major_faults,
        "input_blocks",
        usage.input_blocks,
        "output_blocks",
        usage.output_blocks,
        "signals",
        usage.signals
    );
}

static PyObject *
pyos_resource_getpriority(
    PyObject *self,
    PyObject *args
)
{
    int which;
    long long who;
    long priority;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "iL:getpriority",
            &which,
            &who
        )
    ) {
        return NULL;
    }

    priority =
        pyos_native_getpriority(
            which,
            who
        );

    if (
        priority < 0 &&
        pyos_get_errno() != 0
    ) {
        return pyos_resource_error(
            "getpriority"
        );
    }

    return PyLong_FromLong(priority);
}

static PyObject *
pyos_resource_setpriority(
    PyObject *self,
    PyObject *args
)
{
    int which;
    long long who;
    int priority;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "iLi:setpriority",
            &which,
            &who,
            &priority
        )
    ) {
        return NULL;
    }

    if (
        pyos_native_setpriority(
            which,
            who,
            priority
        ) < 0
    ) {
        return pyos_resource_error(
            "setpriority"
        );
    }

    Py_RETURN_NONE;
}

static PyObject *
pyos_resource_getpagesize(
    PyObject *self,
    PyObject *args
)
{
    long size;

    (void)self;
    (void)args;

    size = pyos_native_getpagesize();

    if (size <= 0) {
        return pyos_resource_error(
            "getpagesize"
        );
    }

    return PyLong_FromLong(size);
}

static PyObject *
pyos_resource_getphysmem(
    PyObject *self,
    PyObject *args
)
{
    unsigned long long total;
    unsigned long long available;

    (void)self;
    (void)args;

    if (
        pyos_native_get_memory_info(
            &total,
            &available
        ) < 0
    ) {
        return pyos_resource_error(
            "memory_info"
        );
    }

    return Py_BuildValue(
        "(KK)",
        total,
        available
    );
}

static PyObject *
pyos_resource_getloadavg(
    PyObject *self,
    PyObject *args
)
{
    int count;
    double values[3];
    PyObject *result;
    int index;

    (void)self;

    count = 3;

    if (
        !PyArg_ParseTuple(
            args,
            "|i:getloadavg",
            &count
        )
    ) {
        return NULL;
    }

    if (
        count < 1 ||
        count > 3
    ) {
        PyErr_SetString(
            PyExc_ValueError,
            "load average count must be between 1 and 3"
        );

        return NULL;
    }

    if (
        pyos_native_getloadavg(
            values,
            count
        ) < 0
    ) {
        return pyos_resource_error(
            "getloadavg"
        );
    }

    result = PyTuple_New(count);

    if (result == NULL) {
        return NULL;
    }

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

static PyObject *
pyos_resource_sync(
    PyObject *self,
    PyObject *args
)
{
    (void)self;
    (void)args;

    if (
        pyos_native_sync() < 0
    ) {
        return pyos_resource_error(
            "sync"
        );
    }

    Py_RETURN_NONE;
}

static PyObject *
pyos_resource_syncfs(
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
    ) {
        return NULL;
    }

    if (
        pyos_native_syncfs(fd) < 0
    ) {
        return pyos_resource_error(
            "syncfs"
        );
    }

    Py_RETURN_NONE;
}

static PyObject *
pyos_resource_umask(
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
    ) {
        return NULL;
    }

    previous =
        pyos_native_umask(mask);

    return PyLong_FromUnsignedLong(
        previous
    );
}

static PyObject *
pyos_resource_getumask(
    PyObject *self,
    PyObject *args
)
{
    unsigned int mask;

    (void)self;
    (void)args;

    mask = pyos_native_getumask();

    return PyLong_FromUnsignedLong(
        mask
    );
}

static PyObject *
pyos_resource_getrlimit_count(
    PyObject *self,
    PyObject *args
)
{
    int count;

    (void)self;
    (void)args;

    count =
        pyos_native_resource_count();

    if (count < 0) {
        return pyos_resource_error(
            "resource_count"
        );
    }

    return PyLong_FromLong(count);
}

static PyMethodDef pyos_resource_methods[] = {
    {
        "getrlimit",
        pyos_resource_getrlimit,
        METH_VARARGS,
        NULL
    },
    {
        "setrlimit",
        pyos_resource_setrlimit,
        METH_VARARGS,
        NULL
    },
    {
        "getrusage",
        pyos_resource_getrusage,
        METH_VARARGS,
        NULL
    },
    {
        "getpriority",
        pyos_resource_getpriority,
        METH_VARARGS,
        NULL
    },
    {
        "setpriority",
        pyos_resource_setpriority,
        METH_VARARGS,
        NULL
    },
    {
        "getpagesize",
        pyos_resource_getpagesize,
        METH_NOARGS,
        NULL
    },
    {
        "memory_info",
        pyos_resource_getphysmem,
        METH_NOARGS,
        NULL
    },
    {
        "getloadavg",
        pyos_resource_getloadavg,
        METH_VARARGS,
        NULL
    },
    {
        "sync",
        pyos_resource_sync,
        METH_NOARGS,
        NULL
    },
    {
        "syncfs",
        pyos_resource_syncfs,
        METH_VARARGS,
        NULL
    },
    {
        "umask",
        pyos_resource_umask,
        METH_VARARGS,
        NULL
    },
    {
        "getumask",
        pyos_resource_getumask,
        METH_NOARGS,
        NULL
    },
    {
        "resource_count",
        pyos_resource_getrlimit_count,
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
pyos_get_resource_methods(void)
{
    return pyos_resource_methods;
}
