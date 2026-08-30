#define PY_SSIZE_T_CLEAN

#include <Python.h>

#include "posix_internal.h"

static PyObject *
pyos_user_error(
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
        message = "user operation failed";

    PyErr_Format(
        PyExc_OSError,
        "[%s] %s",
        operation,
        message
    );

    return NULL;
}

static PyObject *
pyos_user_getuid(
    PyObject *self,
    PyObject *args
)
{
    unsigned long value;

    (void)self;
    (void)args;

    value = pyos_native_getuid();

    return PyLong_FromUnsignedLong(value);
}

static PyObject *
pyos_user_geteuid(
    PyObject *self,
    PyObject *args
)
{
    unsigned long value;

    (void)self;
    (void)args;

    value = pyos_native_geteuid();

    return PyLong_FromUnsignedLong(value);
}

static PyObject *
pyos_user_getgid(
    PyObject *self,
    PyObject *args
)
{
    unsigned long value;

    (void)self;
    (void)args;

    value = pyos_native_getgid();

    return PyLong_FromUnsignedLong(value);
}

static PyObject *
pyos_user_getegid(
    PyObject *self,
    PyObject *args
)
{
    unsigned long value;

    (void)self;
    (void)args;

    value = pyos_native_getegid();

    return PyLong_FromUnsignedLong(value);
}

static PyObject *
pyos_user_getgroups(
    PyObject *self,
    PyObject *args
)
{
    unsigned long *groups;
    long count;
    long index;
    PyObject *result;

    (void)self;
    (void)args;

    count = pyos_native_getgroups(
        0,
        NULL
    );

    if (count < 0)
        return pyos_user_error(
            "getgroups"
        );

    if (count == 0)
        return PyList_New(0);

    groups = PyMem_Malloc(
        sizeof(unsigned long) *
        (size_t)count
    );

    if (groups == NULL)
        return PyErr_NoMemory();

    count = pyos_native_getgroups(
        count,
        groups
    );

    if (count < 0) {
        PyMem_Free(groups);

        return pyos_user_error(
            "getgroups"
        );
    }

    result = PyList_New(count);

    if (result == NULL) {
        PyMem_Free(groups);
        return NULL;
    }

    for (
        index = 0;
        index < count;
        index++
    ) {
        PyObject *value;

        value = PyLong_FromUnsignedLong(
            groups[index]
        );

        if (value == NULL) {
            Py_DECREF(result);
            PyMem_Free(groups);

            return NULL;
        }

        PyList_SET_ITEM(
            result,
            index,
            value
        );
    }

    PyMem_Free(groups);

    return result;
}

static PyObject *
pyos_user_setuid(
    PyObject *self,
    PyObject *args
)
{
    unsigned long uid;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "k:setuid",
            &uid
        )
    )
        return NULL;

    if (
        pyos_native_setuid(uid) < 0
    )
        return pyos_user_error(
            "setuid"
        );

    Py_RETURN_NONE;
}

static PyObject *
pyos_user_setgid(
    PyObject *self,
    PyObject *args
)
{
    unsigned long gid;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "k:setgid",
            &gid
        )
    )
        return NULL;

    if (
        pyos_native_setgid(gid) < 0
    )
        return pyos_user_error(
            "setgid"
        );

    Py_RETURN_NONE;
}

static PyObject *
pyos_user_setgroups(
    PyObject *self,
    PyObject *args
)
{
    PyObject *sequence;
    Py_ssize_t count;
    Py_ssize_t index;
    unsigned long *groups;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "O:setgroups",
            &sequence
        )
    )
        return NULL;

    sequence = PySequence_Fast(
        sequence,
        "groups must be a sequence"
    );

    if (sequence == NULL)
        return NULL;

    count = PySequence_Fast_GET_SIZE(
        sequence
    );

    if (count > 0) {
        groups = PyMem_Malloc(
            sizeof(unsigned long) *
            (size_t)count
        );

        if (groups == NULL) {
            Py_DECREF(sequence);
            return PyErr_NoMemory();
        }
    } else {
        groups = NULL;
    }

    for (
        index = 0;
        index < count;
        index++
    ) {
        PyObject *item;
        unsigned long value;

        item = PySequence_Fast_GET_ITEM(
            sequence,
            index
        );

        value = PyLong_AsUnsignedLong(
            item
        );

        if (
            value ==
            (unsigned long)-1 &&
            PyErr_Occurred()
        ) {
            PyMem_Free(groups);
            Py_DECREF(sequence);

            return NULL;
        }

        groups[index] = value;
    }

    if (
        pyos_native_setgroups(
            (long)count,
            groups
        ) < 0
    ) {
        PyMem_Free(groups);
        Py_DECREF(sequence);

        return pyos_user_error(
            "setgroups"
        );
    }

    PyMem_Free(groups);
    Py_DECREF(sequence);

    Py_RETURN_NONE;
}

static PyObject *
pyos_user_getlogin(
    PyObject *self,
    PyObject *args
)
{
    const char *name;

    (void)self;
    (void)args;

    name = pyos_native_getlogin();

    if (name == NULL)
        return pyos_user_error(
            "getlogin"
        );

    return PyUnicode_FromString(name);
}

static PyObject *
pyos_user_gethostname(
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

    result = pyos_native_gethostname(
        buffer,
        sizeof(buffer)
    );

    if (result < 0)
        return pyos_user_error(
            "gethostname"
        );

    return PyUnicode_FromString(
        buffer
    );
}

static PyObject *
pyos_user_getdomainname(
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

    result = pyos_native_getdomainname(
        buffer,
        sizeof(buffer)
    );

    if (result < 0)
        return pyos_user_error(
            "getdomainname"
        );

    return PyUnicode_FromString(
        buffer
    );
}

static PyMethodDef pyos_user_methods[] = {
    {
        "getuid",
        pyos_user_getuid,
        METH_NOARGS,
        NULL
    },
    {
        "geteuid",
        pyos_user_geteuid,
        METH_NOARGS,
        NULL
    },
    {
        "getgid",
        pyos_user_getgid,
        METH_NOARGS,
        NULL
    },
    {
        "getegid",
        pyos_user_getegid,
        METH_NOARGS,
        NULL
    },
    {
        "getgroups",
        pyos_user_getgroups,
        METH_NOARGS,
        NULL
    },
    {
        "setuid",
        pyos_user_setuid,
        METH_VARARGS,
        NULL
    },
    {
        "setgid",
        pyos_user_setgid,
        METH_VARARGS,
        NULL
    },
    {
        "setgroups",
        pyos_user_setgroups,
        METH_VARARGS,
        NULL
    },
    {
        "getlogin",
        pyos_user_getlogin,
        METH_NOARGS,
        NULL
    },
    {
        "gethostname",
        pyos_user_gethostname,
        METH_NOARGS,
        NULL
    },
    {
        "getdomainname",
        pyos_user_getdomainname,
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
pyos_get_user_methods(void)
{
    return pyos_user_methods;
}
