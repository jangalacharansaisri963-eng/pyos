#define PY_SSIZE_T_CLEAN

#include <Python.h>

#include "posix_internal.h"

static PyObject *
pyos_environment_error(
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
        message = "native environment operation failed";
    }

    PyErr_Format(
        PyExc_OSError,
        "[%s] %s",
        operation,
        message
    );

    return NULL;
}

static int
pyos_environment_key(
    PyObject *object,
    const char **key
)
{
    if (PyUnicode_Check(object)) {
        *key = PyUnicode_AsUTF8(object);

        if (*key == NULL) {
            return -1;
        }

        return 0;
    }

    if (PyBytes_Check(object)) {
        *key = PyBytes_AsString(object);

        if (*key == NULL) {
            return -1;
        }

        return 0;
    }

    PyErr_SetString(
        PyExc_TypeError,
        "environment key must be str or bytes"
    );

    return -1;
}

static int
pyos_environment_value(
    PyObject *object,
    const char **value
)
{
    if (PyUnicode_Check(object)) {
        *value = PyUnicode_AsUTF8(object);

        if (*value == NULL) {
            return -1;
        }

        return 0;
    }

    if (PyBytes_Check(object)) {
        *value = PyBytes_AsString(object);

        if (*value == NULL) {
            return -1;
        }

        return 0;
    }

    PyErr_SetString(
        PyExc_TypeError,
        "environment value must be str or bytes"
    );

    return -1;
}

static PyObject *
pyos_environment_getenv(
    PyObject *self,
    PyObject *args
)
{
    PyObject *key_object;
    const char *key;
    const char *value;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "O:getenv",
            &key_object
        )
    ) {
        return NULL;
    }

    if (
        pyos_environment_key(
            key_object,
            &key
        ) < 0
    ) {
        return NULL;
    }

    value = pyos_native_getenv(key);

    if (value == NULL) {
        Py_RETURN_NONE;
    }

    if (PyBytes_Check(key_object)) {
        return PyBytes_FromString(value);
    }

    return PyUnicode_FromString(value);
}

static PyObject *
pyos_environment_getenvb(
    PyObject *self,
    PyObject *args
)
{
    PyObject *key_object;
    const char *key;
    const char *value;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "O:getenvb",
            &key_object
        )
    ) {
        return NULL;
    }

    if (
        !PyBytes_Check(key_object)
    ) {
        PyErr_SetString(
            PyExc_TypeError,
            "getenvb() argument must be bytes"
        );

        return NULL;
    }

    key = PyBytes_AsString(
        key_object
    );

    if (key == NULL) {
        return NULL;
    }

    value = pyos_native_getenv(key);

    if (value == NULL) {
        Py_RETURN_NONE;
    }

    return PyBytes_FromString(value);
}

static PyObject *
pyos_environment_setenv(
    PyObject *self,
    PyObject *args
)
{
    PyObject *key_object;
    PyObject *value_object;
    int overwrite;
    const char *key;
    const char *value;

    (void)self;

    overwrite = 1;

    if (
        !PyArg_ParseTuple(
            args,
            "OO|p:setenv",
            &key_object,
            &value_object,
            &overwrite
        )
    ) {
        return NULL;
    }

    if (
        pyos_environment_key(
            key_object,
            &key
        ) < 0
    ) {
        return NULL;
    }

    if (
        pyos_environment_value(
            value_object,
            &value
        ) < 0
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
        return pyos_environment_error(
            "setenv"
        );
    }

    Py_RETURN_NONE;
}

static PyObject *
pyos_environment_unsetenv(
    PyObject *self,
    PyObject *args
)
{
    PyObject *key_object;
    const char *key;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "O:unsetenv",
            &key_object
        )
    ) {
        return NULL;
    }

    if (
        pyos_environment_key(
            key_object,
            &key
        ) < 0
    ) {
        return NULL;
    }

    if (
        pyos_native_unsetenv(key) < 0
    ) {
        return pyos_environment_error(
            "unsetenv"
        );
    }

    Py_RETURN_NONE;
}

static PyObject *
pyos_environment_putenv(
    PyObject *self,
    PyObject *args
)
{
    PyObject *value_object;
    const char *value;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "O:putenv",
            &value_object
        )
    ) {
        return NULL;
    }

    if (
        pyos_environment_value(
            value_object,
            &value
        ) < 0
    ) {
        return NULL;
    }

    if (
        pyos_native_putenv(value) < 0
    ) {
        return pyos_environment_error(
            "putenv"
        );
    }

    Py_RETURN_NONE;
}

static PyObject *
pyos_environment_clearenv(
    PyObject *self,
    PyObject *args
)
{
    (void)self;
    (void)args;

    if (
        pyos_native_clearenv() < 0
    ) {
        return pyos_environment_error(
            "clearenv"
        );
    }

    Py_RETURN_NONE;
}

static PyObject *
pyos_environment_hasenv(
    PyObject *self,
    PyObject *args
)
{
    PyObject *key_object;
    const char *key;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "O:hasenv",
            &key_object
        )
    ) {
        return NULL;
    }

    if (
        pyos_environment_key(
            key_object,
            &key
        ) < 0
    ) {
        return NULL;
    }

    return PyBool_FromLong(
        pyos_native_getenv(key) != NULL
    );
}

static PyObject *
pyos_environment_getenv_default(
    PyObject *self,
    PyObject *args
)
{
    PyObject *key_object;
    PyObject *default_object;
    const char *key;
    const char *value;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "OO:getenv_default",
            &key_object,
            &default_object
        )
    ) {
        return NULL;
    }

    if (
        pyos_environment_key(
            key_object,
            &key
        ) < 0
    ) {
        return NULL;
    }

    value = pyos_native_getenv(key);

    if (value == NULL) {
        Py_INCREF(default_object);
        return default_object;
    }

    if (PyBytes_Check(key_object)) {
        return PyBytes_FromString(value);
    }

    return PyUnicode_FromString(value);
}

static PyObject *
pyos_environment_getcwd(
    PyObject *self,
    PyObject *args
)
{
    char buffer[PYOS_PATH_MAX];
    int result;

    (void)self;
    (void)args;

    result = pyos_native_getcwd(
        buffer,
        sizeof(buffer)
    );

    if (result < 0) {
        return pyos_environment_error(
            "getcwd"
        );
    }

    return PyUnicode_FromString(
        buffer
    );
}

static PyObject *
pyos_environment_chdir(
    PyObject *self,
    PyObject *args
)
{
    PyObject *path_object;
    pyos_path_buffer path;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "O:chdir",
            &path_object
        )
    ) {
        return NULL;
    }

    path.data = NULL;
    path.length = 0;
    path.owns_data = 0;

    if (
        pyos_path_from_object(
            path_object,
            &path
        ) < 0
    ) {
        return NULL;
    }

    if (
        pyos_native_chdir(
            path.data
        ) < 0
    ) {
        pyos_path_release(&path);

        return pyos_environment_error(
            "chdir"
        );
    }

    pyos_path_release(&path);

    Py_RETURN_NONE;
}

static PyObject *
pyos_environment_umask(
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
pyos_environment_hostname(
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
        return pyos_environment_error(
            "gethostname"
        );
    }

    return PyUnicode_FromString(
        buffer
    );
}

static PyObject *
pyos_environment_domainname(
    PyObject *self,
    PyObject *args
)
{
    char buffer[PYOS_PATH_MAX];
    int result;

    (void)self;
    (void)args;

    result = pyos_native_getdomainname(
        buffer,
        sizeof(buffer)
    );

    if (result < 0) {
        return pyos_environment_error(
            "getdomainname"
        );
    }

    return PyUnicode_FromString(
        buffer
    );
}

static PyMethodDef pyos_environment_methods[] = {
    {
        "getenv",
        pyos_environment_getenv,
        METH_VARARGS,
        NULL
    },
    {
        "getenvb",
        pyos_environment_getenvb,
        METH_VARARGS,
        NULL
    },
    {
        "setenv",
        pyos_environment_setenv,
        METH_VARARGS,
        NULL
    },
    {
        "unsetenv",
        pyos_environment_unsetenv,
        METH_VARARGS,
        NULL
    },
    {
        "putenv",
        pyos_environment_putenv,
        METH_VARARGS,
        NULL
    },
    {
        "clearenv",
        pyos_environment_clearenv,
        METH_NOARGS,
        NULL
    },
    {
        "hasenv",
        pyos_environment_hasenv,
        METH_VARARGS,
        NULL
    },
    {
        "getenv_default",
        pyos_environment_getenv_default,
        METH_VARARGS,
        NULL
    },
    {
        "getcwd",
        pyos_environment_getcwd,
        METH_NOARGS,
        NULL
    },
    {
        "chdir",
        pyos_environment_chdir,
        METH_VARARGS,
        NULL
    },
    {
        "umask",
        pyos_environment_umask,
        METH_VARARGS,
        NULL
    },
    {
        "gethostname",
        pyos_environment_hostname,
        METH_NOARGS,
        NULL
    },
    {
        "getdomainname",
        pyos_environment_domainname,
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
pyos_get_environment_methods(void)
{
    return pyos_environment_methods;
}
