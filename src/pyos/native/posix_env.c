#define PY_SSIZE_T_CLEAN

#include <Python.h>

#include "posix_internal.h"

static PyObject *
pyos_env_error(const char *operation)
{
    int error_code;
    const char *message;

    error_code = pyos_get_errno();
    message = NULL;

    if (pyos_native_strerror != NULL) {
        message = pyos_native_strerror(error_code);
    }

    if (message == NULL) {
        message = "native environment operation failed";
    }

    PyErr_Format(
        PyExc_OSError,
        "%s: %s",
        operation,
        message
    );

    return NULL;
}

static PyObject *
pyos_env_get(
    PyObject *self,
    PyObject *args
)
{
    const char *key;
    const char *value;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "s:getenv",
            &key
        )
    ) {
        return NULL;
    }

    value = pyos_native_getenv(key);

    if (value == NULL) {
        Py_RETURN_NONE;
    }

    return PyUnicode_FromString(value);
}

static PyObject *
pyos_env_get_bytes(
    PyObject *self,
    PyObject *args
)
{
    const char *key;
    const char *value;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "s:getenvb",
            &key
        )
    ) {
        return NULL;
    }

    value = pyos_native_getenv(key);

    if (value == NULL) {
        Py_RETURN_NONE;
    }

    return PyBytes_FromString(value);
}

static PyObject *
pyos_env_set(
    PyObject *self,
    PyObject *args
)
{
    const char *key;
    const char *value;
    int overwrite;

    (void)self;

    overwrite = 1;

    if (
        !PyArg_ParseTuple(
            args,
            "ss|p:setenv",
            &key,
            &value,
            &overwrite
        )
    ) {
        return NULL;
    }

    if (
        pyos_native_setenv(
            key,
            value,
            overwrite
        ) < 0
    ) {
        return pyos_env_error(
            "setenv"
        );
    }

    Py_RETURN_NONE;
}

static PyObject *
pyos_env_unset(
    PyObject *self,
    PyObject *args
)
{
    const char *key;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "s:unsetenv",
            &key
        )
    ) {
        return NULL;
    }

    if (
        pyos_native_unsetenv(key) < 0
    ) {
        return pyos_env_error(
            "unsetenv"
        );
    }

    Py_RETURN_NONE;
}

static PyObject *
pyos_env_has(
    PyObject *self,
    PyObject *args
)
{
    const char *key;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "s:hasenv",
            &key
        )
    ) {
        return NULL;
    }

    if (
        pyos_native_getenv(key) == NULL
    ) {
        Py_RETURN_FALSE;
    }

    Py_RETURN_TRUE;
}

static PyObject *
pyos_env_clear(
    PyObject *self,
    PyObject *args
)
{
    (void)self;
    (void)args;

    if (
        pyos_native_clearenv() < 0
    ) {
        return pyos_env_error(
            "clearenv"
        );
    }

    Py_RETURN_NONE;
}

static PyObject *
pyos_env_snapshot(
    PyObject *self,
    PyObject *args
)
{
    PyObject *result;
    Py_ssize_t index;
    Py_ssize_t count;
    char **environment;

    (void)self;
    (void)args;

    environment = pyos_native_environ();

    if (environment == NULL) {
        return PyDict_New();
    }

    result = PyDict_New();

    if (result == NULL) {
        return NULL;
    }

    count = 0;

    while (environment[count] != NULL) {
        const char *entry;
        const char *separator;
        PyObject *key;
        PyObject *value;

        entry = environment[count];

        separator = entry;

        while (
            *separator != '\0' &&
            *separator != '='
        ) {
            separator++;
        }

        if (*separator == '=') {
            Py_ssize_t key_length;

            key_length =
                (Py_ssize_t)(
                    separator - entry
                );

            key = PyUnicode_DecodeLocale(
                entry,
                "surrogateescape"
            );

            if (key == NULL) {
                Py_DECREF(result);
                return NULL;
            }

            value = PyUnicode_DecodeLocale(
                separator + 1,
                "surrogateescape"
            );

            if (value == NULL) {
                Py_DECREF(key);
                Py_DECREF(result);
                return NULL;
            }

            if (
                PyUnicode_GET_LENGTH(key) !=
                key_length
            ) {
                Py_DECREF(key);
                Py_DECREF(value);
                Py_DECREF(result);

                PyErr_SetString(
                    PyExc_ValueError,
                    "invalid environment entry"
                );

                return NULL;
            }

            if (
                PyDict_SetItem(
                    result,
                    key,
                    value
                ) < 0
            ) {
                Py_DECREF(key);
                Py_DECREF(value);
                Py_DECREF(result);

                return NULL;
            }

            Py_DECREF(key);
            Py_DECREF(value);
        }

        count++;
    }

    index = count;

    if (index < 0) {
        Py_DECREF(result);
        return NULL;
    }

    return result;
}

static PyObject *
pyos_env_keys(
    PyObject *self,
    PyObject *args
)
{
    PyObject *snapshot;
    PyObject *keys;

    (void)self;
    (void)args;

    snapshot = pyos_env_snapshot(
        NULL,
        NULL
    );

    if (snapshot == NULL) {
        return NULL;
    }

    keys = PyDict_Keys(snapshot);

    Py_DECREF(snapshot);

    return keys;
}

static PyObject *
pyos_env_values(
    PyObject *self,
    PyObject *args
)
{
    PyObject *snapshot;
    PyObject *values;

    (void)self;
    (void)args;

    snapshot = pyos_env_snapshot(
        NULL,
        NULL
    );

    if (snapshot == NULL) {
        return NULL;
    }

    values = PyDict_Values(snapshot);

    Py_DECREF(snapshot);

    return values;
}

static PyObject *
pyos_env_items(
    PyObject *self,
    PyObject *args
)
{
    PyObject *snapshot;
    PyObject *items;

    (void)self;
    (void)args;

    snapshot = pyos_env_snapshot(
        NULL,
        NULL
    );

    if (snapshot == NULL) {
        return NULL;
    }

    items = PyDict_Items(snapshot);

    Py_DECREF(snapshot);

    return items;
}

static PyObject *
pyos_env_replace(
    PyObject *self,
    PyObject *args
)
{
    const char *key;
    const char *value;
    const char *old_value;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "ss:replaceenv",
            &key,
            &value
        )
    ) {
        return NULL;
    }

    old_value = pyos_native_getenv(key);

    if (old_value == NULL) {
        Py_RETURN_FALSE;
    }

    if (
        pyos_native_setenv(
            key,
            value,
            1
        ) < 0
    ) {
        return pyos_env_error(
            "replaceenv"
        );
    }

    Py_RETURN_TRUE;
}

static PyObject *
pyos_env_getpid(
    PyObject *self,
    PyObject *args
)
{
    long long pid;

    (void)self;
    (void)args;

    pid = pyos_native_getpid();

    if (pid < 0) {
        return pyos_env_error(
            "getpid"
        );
    }

    return PyLong_FromLongLong(pid);
}

static PyObject *
pyos_env_gethostname(
    PyObject *self,
    PyObject *args
)
{
    char buffer[PYOS_PATH_MAX];
    int result;

    (void)self;
    (void)args;

    result = pyos_native_gethostname(
        buffer,
        sizeof(buffer)
    );

    if (result < 0) {
        return pyos_env_error(
            "gethostname"
        );
    }

    return PyUnicode_FromString(
        buffer
    );
}

static PyMethodDef pyos_env_methods[] = {
    {
        "getenv",
        pyos_env_get,
        METH_VARARGS,
        NULL
    },
    {
        "getenvb",
        pyos_env_get_bytes,
        METH_VARARGS,
        NULL
    },
    {
        "setenv",
        pyos_env_set,
        METH_VARARGS,
        NULL
    },
    {
        "unsetenv",
        pyos_env_unset,
        METH_VARARGS,
        NULL
    },
    {
        "hasenv",
        pyos_env_has,
        METH_VARARGS,
        NULL
    },
    {
        "clearenv",
        pyos_env_clear,
        METH_NOARGS,
        NULL
    },
    {
        "snapshot",
        pyos_env_snapshot,
        METH_NOARGS,
        NULL
    },
    {
        "keys",
        pyos_env_keys,
        METH_NOARGS,
        NULL
    },
    {
        "values",
        pyos_env_values,
        METH_NOARGS,
        NULL
    },
    {
        "items",
        pyos_env_items,
        METH_NOARGS,
        NULL
    },
    {
        "replaceenv",
        pyos_env_replace,
        METH_VARARGS,
        NULL
    },
    {
        "getpid",
        pyos_env_getpid,
        METH_NOARGS,
        NULL
    },
    {
        "gethostname",
        pyos_env_gethostname,
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
pyos_get_env_methods(void)
{
    return pyos_env_methods;
}
