#define PY_SSIZE_T_CLEAN

#include <Python.h>

#include "posix_internal.h"

typedef struct {
    unsigned long long uptime;
    unsigned long long loads[3];
    unsigned long long totalram;
    unsigned long long freeram;
    unsigned long long sharedram;
    unsigned long long bufferram;
    unsigned long long totalswap;
    unsigned long long freeswap;
    unsigned short procs;
    unsigned long long totalhigh;
    unsigned long long freehigh;
    unsigned int mem_unit;
} pyos_linux_sysinfo;

static PyObject *
pyos_sysinfo_error(
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
        message = "system information operation failed";

    PyErr_Format(
        PyExc_OSError,
        "[%s] %s",
        operation,
        message
    );

    return NULL;
}

static PyObject *
pyos_sysinfo_get(
    PyObject *self,
    PyObject *args
)
{
    pyos_linux_sysinfo info;
    PyObject *result;
    unsigned long long unit;

    (void)self;
    (void)args;

    if (
        pyos_native_sysinfo(
            &info
        ) < 0
    )
        return pyos_sysinfo_error("sysinfo");

    unit = info.mem_unit;

    if (unit == 0)
        unit = 1;

    result = PyDict_New();

    if (result == NULL)
        return NULL;

#define PYOS_SYSINFO_ADD_ULL(name, value)                  \
    do {                                                    \
        PyObject *item = PyLong_FromUnsignedLongLong(      \
            (unsigned long long)(value)                    \
        );                                                  \
        if (item == NULL)                                   \
            goto failure;                                   \
        if (PyDict_SetItemString(result, name, item) < 0)  \
        {                                                   \
            Py_DECREF(item);                               \
            goto failure;                                   \
        }                                                   \
        Py_DECREF(item);                                   \
    } while (0)

    PYOS_SYSINFO_ADD_ULL(
        "uptime",
        info.uptime
    );

    PYOS_SYSINFO_ADD_ULL(
        "totalram",
        info.totalram * unit
    );

    PYOS_SYSINFO_ADD_ULL(
        "freeram",
        info.freeram * unit
    );

    PYOS_SYSINFO_ADD_ULL(
        "sharedram",
        info.sharedram * unit
    );

    PYOS_SYSINFO_ADD_ULL(
        "bufferram",
        info.bufferram * unit
    );

    PYOS_SYSINFO_ADD_ULL(
        "totalswap",
        info.totalswap * unit
    );

    PYOS_SYSINFO_ADD_ULL(
        "freeswap",
        info.freeswap * unit
    );

    PYOS_SYSINFO_ADD_ULL(
        "totalhigh",
        info.totalhigh * unit
    );

    PYOS_SYSINFO_ADD_ULL(
        "freehigh",
        info.freehigh * unit
    );

    PYOS_SYSINFO_ADD_ULL(
        "mem_unit",
        unit
    );

    PYOS_SYSINFO_ADD_ULL(
        "processes",
        info.procs
    );

    PYOS_SYSINFO_ADD_ULL(
        "load_1",
        info.loads[0]
    );

    PYOS_SYSINFO_ADD_ULL(
        "load_5",
        info.loads[1]
    );

    PYOS_SYSINFO_ADD_ULL(
        "load_15",
        info.loads[2]
    );

#undef PYOS_SYSINFO_ADD_ULL

    return result;

failure:
    Py_DECREF(result);
    return NULL;
}

static PyObject *
pyos_sysinfo_uptime(
    PyObject *self,
    PyObject *args
)
{
    pyos_linux_sysinfo info;

    (void)self;
    (void)args;

    if (
        pyos_native_sysinfo(
            &info
        ) < 0
    )
        return pyos_sysinfo_error(
            "sysinfo"
        );

    return PyLong_FromUnsignedLongLong(
        info.uptime
    );
}

static PyObject *
pyos_sysinfo_memory(
    PyObject *self,
    PyObject *args
)
{
    pyos_linux_sysinfo info;
    unsigned long long unit;
    PyObject *result;

    (void)self;
    (void)args;

    if (
        pyos_native_sysinfo(
            &info
        ) < 0
    )
        return pyos_sysinfo_error(
            "sysinfo"
        );

    unit = info.mem_unit;

    if (unit == 0)
        unit = 1;

    result = Py_BuildValue(
        "{s:K,s:K,s:K,s:K,s:K,s:K,s:K,s:K}",
        "total",
        info.totalram * unit,
        "free",
        info.freeram * unit,
        "shared",
        info.sharedram * unit,
        "buffers",
        info.bufferram * unit,
        "swap_total",
        info.totalswap * unit,
        "swap_free",
        info.freeswap * unit,
        "high_total",
        info.totalhigh * unit,
        "high_free",
        info.freehigh * unit
    );

    return result;
}

static PyObject *
pyos_sysinfo_loads(
    PyObject *self,
    PyObject *args
)
{
    pyos_linux_sysinfo info;

    (void)self;
    (void)args;

    if (
        pyos_native_sysinfo(
            &info
        ) < 0
    )
        return pyos_sysinfo_error(
            "sysinfo"
        );

    return Py_BuildValue(
        "(KKK)",
        info.loads[0],
        info.loads[1],
        info.loads[2]
    );
}

static PyObject *
pyos_sysinfo_processes(
    PyObject *self,
    PyObject *args
)
{
    pyos_linux_sysinfo info;

    (void)self;
    (void)args;

    if (
        pyos_native_sysinfo(
            &info
        ) < 0
    )
        return pyos_sysinfo_error(
            "sysinfo"
        );

    return PyLong_FromUnsignedLong(
        (unsigned long)info.procs
    );
}

static PyObject *
pyos_sysinfo_mem_unit(
    PyObject *self,
    PyObject *args
)
{
    pyos_linux_sysinfo info;
    unsigned int unit;

    (void)self;
    (void)args;

    if (
        pyos_native_sysinfo(
            &info
        ) < 0
    )
        return pyos_sysinfo_error(
            "sysinfo"
        );

    unit = info.mem_unit;

    if (unit == 0)
        unit = 1;

    return PyLong_FromUnsignedLong(
        (unsigned long)unit
    );
}

static PyMethodDef pyos_sysinfo_methods[] = {
    {
        "sysinfo",
        pyos_sysinfo_get,
        METH_NOARGS,
        NULL
    },
    {
        "uptime",
        pyos_sysinfo_uptime,
        METH_NOARGS,
        NULL
    },
    {
        "memory",
        pyos_sysinfo_memory,
        METH_NOARGS,
        NULL
    },
    {
        "loads",
        pyos_sysinfo_loads,
        METH_NOARGS,
        NULL
    },
    {
        "processes",
        pyos_sysinfo_processes,
        METH_NOARGS,
        NULL
    },
    {
        "mem_unit",
        pyos_sysinfo_mem_unit,
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
pyos_get_sysinfo_methods(void)
{
    return pyos_sysinfo_methods;
}
