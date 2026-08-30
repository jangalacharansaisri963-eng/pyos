#define PY_SSIZE_T_CLEAN

#include <Python.h>

#include "posix_internal.h"

static PyObject *
pyos_exec_error(const char *operation)
{
    int error_code;
    const char *message;

    error_code = pyos_get_errno();
    message = NULL;

    if (pyos_native_strerror != NULL) {
        message = pyos_native_strerror(error_code);
    }

    if (message == NULL) {
        message = "process execution failed";
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
pyos_exec_build_argv(
    PyObject *sequence
)
{
    Py_ssize_t count;
    Py_ssize_t index;
    PyObject *tuple;

    if (!PySequence_Check(sequence)) {
        PyErr_SetString(
            PyExc_TypeError,
            "argument list must be a sequence"
        );

        return NULL;
    }

    count = PySequence_Size(sequence);

    if (count < 0) {
        return NULL;
    }

    tuple = PyTuple_New(count);

    if (tuple == NULL) {
        return NULL;
    }

    for (index = 0; index < count; index++) {
        PyObject *item;
        PyObject *text;

        item = PySequence_GetItem(
            sequence,
            index
        );

        if (item == NULL) {
            Py_DECREF(tuple);
            return NULL;
        }

        if (
            PyUnicode_Check(item) ||
            PyBytes_Check(item)
        ) {
            text = item;
        } else {
            PyErr_SetString(
                PyExc_TypeError,
                "exec arguments must be strings"
            );

            Py_DECREF(item);
            Py_DECREF(tuple);

            return NULL;
        }

        PyTuple_SET_ITEM(
            tuple,
            index,
            text
        );
    }

    return tuple;
}

static int
pyos_exec_sequence_to_argv(
    PyObject *sequence,
    char ***argv_out,
    Py_ssize_t *count_out
)
{
    Py_ssize_t count;
    Py_ssize_t index;
    char **argv;

    count = PySequence_Size(sequence);

    if (count < 0) {
        return -1;
    }

    argv = PyMem_Calloc(
        (size_t)count + 1,
        sizeof(char *)
    );

    if (argv == NULL) {
        PyErr_NoMemory();
        return -1;
    }

    for (index = 0; index < count; index++) {
        PyObject *item;
        PyObject *bytes;

        item = PySequence_GetItem(
            sequence,
            index
        );

        if (item == NULL) {
            PyMem_Free(argv);
            return -1;
        }

        if (PyBytes_Check(item)) {
            bytes = item;
        } else if (PyUnicode_Check(item)) {
            bytes = PyUnicode_EncodeLocale(
                item,
                "surrogateescape"
            );

            Py_DECREF(item);

            if (bytes == NULL) {
                PyMem_Free(argv);
                return -1;
            }
        } else {
            PyErr_SetString(
                PyExc_TypeError,
                "exec arguments must be strings"
            );

            Py_DECREF(item);
            PyMem_Free(argv);

            return -1;
        }

        argv[index] = PyMem_Malloc(
            (size_t)PyBytes_GET_SIZE(bytes) + 1
        );

        if (argv[index] == NULL) {
            Py_DECREF(bytes);

            while (index > 0) {
                index--;
                PyMem_Free(argv[index]);
            }

            PyMem_Free(argv);

            PyErr_NoMemory();

            return -1;
        }

        Py_MEMCPY(
            argv[index],
            PyBytes_AS_STRING(bytes),
            PyBytes_GET_SIZE(bytes)
        );

        argv[index][
            PyBytes_GET_SIZE(bytes)
        ] = '\0';

        Py_DECREF(bytes);
    }

    argv[count] = NULL;

    *argv_out = argv;
    *count_out = count;

    return 0;
}

static void
pyos_exec_free_argv(
    char **argv,
    Py_ssize_t count
)
{
    Py_ssize_t index;

    if (argv == NULL) {
        return;
    }

    for (index = 0; index < count; index++) {
        PyMem_Free(argv[index]);
    }

    PyMem_Free(argv);
}

static int
pyos_exec_sequence_to_envp(
    PyObject *mapping,
    char ***envp_out,
    Py_ssize_t *count_out
)
{
    PyObject *items;
    Py_ssize_t count;
    Py_ssize_t index;
    char **envp;

    if (!PyMapping_Check(mapping)) {
        PyErr_SetString(
            PyExc_TypeError,
            "environment must be a mapping"
        );

        return -1;
    }

    items = PyMapping_Items(mapping);

    if (items == NULL) {
        return -1;
    }

    count = PyList_Size(items);

    envp = PyMem_Calloc(
        (size_t)count + 1,
        sizeof(char *)
    );

    if (envp == NULL) {
        Py_DECREF(items);
        PyErr_NoMemory();
        return -1;
    }

    for (index = 0; index < count; index++) {
        PyObject *pair;
        PyObject *key;
        PyObject *value;
        PyObject *key_bytes;
        PyObject *value_bytes;
        Py_ssize_t key_length;
        Py_ssize_t value_length;

        pair = PyList_GetItem(
            items,
            index
        );

        key = PyTuple_GetItem(
            pair,
            0
        );

        value = PyTuple_GetItem(
            pair,
            1
        );

        if (
            !PyUnicode_Check(key) &&
            !PyBytes_Check(key)
        ) {
            PyErr_SetString(
                PyExc_TypeError,
                "environment keys must be strings"
            );

            goto error;
        }

        if (
            !PyUnicode_Check(value) &&
            !PyBytes_Check(value)
        ) {
            PyErr_SetString(
                PyExc_TypeError,
                "environment values must be strings"
            );

            goto error;
        }

        if (PyUnicode_Check(key)) {
            key_bytes = PyUnicode_EncodeLocale(
                key,
                "surrogateescape"
            );
        } else {
            key_bytes = key;
            Py_INCREF(key_bytes);
        }

        if (key_bytes == NULL) {
            goto error;
        }

        if (PyUnicode_Check(value)) {
            value_bytes = PyUnicode_EncodeLocale(
                value,
                "surrogateescape"
            );
        } else {
            value_bytes = value;
            Py_INCREF(value_bytes);
        }

        if (value_bytes == NULL) {
            Py_DECREF(key_bytes);
            goto error;
        }

        key_length = PyBytes_GET_SIZE(
            key_bytes
        );

        value_length = PyBytes_GET_SIZE(
            value_bytes
        );

        envp[index] = PyMem_Malloc(
            (size_t)key_length +
            (size_t)value_length +
            2
        );

        if (envp[index] == NULL) {
            Py_DECREF(key_bytes);
            Py_DECREF(value_bytes);

            PyErr_NoMemory();
            goto error;
        }

        Py_MEMCPY(
            envp[index],
            PyBytes_AS_STRING(key_bytes),
            key_length
        );

        envp[index][key_length] = '=';

        Py_MEMCPY(
            envp[index] + key_length + 1,
            PyBytes_AS_STRING(value_bytes),
            value_length
        );

        envp[index][
            key_length + value_length + 1
        ] = '\0';

        Py_DECREF(key_bytes);
        Py_DECREF(value_bytes);
    }

    envp[count] = NULL;

    Py_DECREF(items);

    *envp_out = envp;
    *count_out = count;

    return 0;

error:

    for (index = 0; index < count; index++) {
        PyMem_Free(envp[index]);
    }

    PyMem_Free(envp);
    Py_DECREF(items);

    return -1;
}

static void
pyos_exec_free_envp(
    char **envp,
    Py_ssize_t count
)
{
    Py_ssize_t index;

    if (envp == NULL) {
        return;
    }

    for (index = 0; index < count; index++) {
        PyMem_Free(envp[index]);
    }

    PyMem_Free(envp);
}

static PyObject *
pyos_execv(
    PyObject *self,
    PyObject *args
)
{
    const char *path;
    PyObject *sequence;
    char **argv;
    Py_ssize_t count;
    int result;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "sO:execv",
            &path,
            &sequence
        )
    ) {
        return NULL;
    }

    if (
        pyos_exec_sequence_to_argv(
            sequence,
            &argv,
            &count
        ) < 0
    ) {
        return NULL;
    }

    result = pyos_native_execv(
        path,
        argv
    );

    pyos_exec_free_argv(
        argv,
        count
    );

    if (result < 0) {
        return pyos_exec_error(
            "execv"
        );
    }

    Py_RETURN_NONE;
}

static PyObject *
pyos_execvp(
    PyObject *self,
    PyObject *args
)
{
    const char *file;
    PyObject *sequence;
    char **argv;
    Py_ssize_t count;
    int result;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "sO:execvp",
            &file,
            &sequence
        )
    ) {
        return NULL;
    }

    if (
        pyos_exec_sequence_to_argv(
            sequence,
            &argv,
            &count
        ) < 0
    ) {
        return NULL;
    }

    result = pyos_native_execvp(
        file,
        argv
    );

    pyos_exec_free_argv(
        argv,
        count
    );

    if (result < 0) {
        return pyos_exec_error(
            "execvp"
        );
    }

    Py_RETURN_NONE;
}

static PyObject *
pyos_execve(
    PyObject *self,
    PyObject *args
)
{
    const char *path;
    PyObject *sequence;
    PyObject *environment;
    char **argv;
    char **envp;
    Py_ssize_t argc;
    Py_ssize_t envc;
    int result;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "sOO:execve",
            &path,
            &sequence,
            &environment
        )
    ) {
        return NULL;
    }

    if (
        pyos_exec_sequence_to_argv(
            sequence,
            &argv,
            &argc
        ) < 0
    ) {
        return NULL;
    }

    if (
        pyos_exec_sequence_to_envp(
            environment,
            &envp,
            &envc
        ) < 0
    ) {
        pyos_exec_free_argv(
            argv,
            argc
        );

        return NULL;
    }

    result = pyos_native_execve(
        path,
        argv,
        envp
    );

    pyos_exec_free_argv(
        argv,
        argc
    );

    pyos_exec_free_envp(
        envp,
        envc
    );

    if (result < 0) {
        return pyos_exec_error(
            "execve"
        );
    }

    Py_RETURN_NONE;
}

static PyObject *
pyos_execvpe(
    PyObject *self,
    PyObject *args
)
{
    const char *file;
    PyObject *sequence;
    PyObject *environment;
    char **argv;
    char **envp;
    Py_ssize_t argc;
    Py_ssize_t envc;
    int result;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "sOO:execvpe",
            &file,
            &sequence,
            &environment
        )
    ) {
        return NULL;
    }

    if (
        pyos_exec_sequence_to_argv(
            sequence,
            &argv,
            &argc
        ) < 0
    ) {
        return NULL;
    }

    if (
        pyos_exec_sequence_to_envp(
            environment,
            &envp,
            &envc
        ) < 0
    ) {
        pyos_exec_free_argv(
            argv,
            argc
        );

        return NULL;
    }

    result = pyos_native_execvpe(
        file,
        argv,
        envp
    );

    pyos_exec_free_argv(
        argv,
        argc
    );

    pyos_exec_free_envp(
        envp,
        envc
    );

    if (result < 0) {
        return pyos_exec_error(
            "execvpe"
        );
    }

    Py_RETURN_NONE;
}

static PyObject *
pyos_exec_get_exec_path(
    PyObject *self,
    PyObject *args
)
{
    PyObject *environment;
    PyObject *path_value;
    PyObject *result;

    (void)self;

    environment = Py_None;

    if (
        !PyArg_ParseTuple(
            args,
            "|O:get_exec_path",
            &environment
        )
    ) {
        return NULL;
    }

    if (environment == Py_None) {
        return pyos_native_get_exec_path(
            NULL
        );
    }

    if (!PyMapping_Check(environment)) {
        PyErr_SetString(
            PyExc_TypeError,
            "environment must be a mapping"
        );

        return NULL;
    }

    path_value = PyMapping_GetItemString(
        environment,
        "PATH"
    );

    if (path_value == NULL) {
        PyErr_Clear();

        result = PyList_New(1);

        if (result == NULL) {
            return NULL;
        }

        Py_INCREF(PyUnicode_GetDefaultEncoding());

        PyList_SET_ITEM(
            result,
            0,
            PyUnicode_FromString(".")
        );

        return result;
    }

    result = pyos_native_split_exec_path(
        path_value
    );

    Py_DECREF(path_value);

    return result;
}

static PyMethodDef pyos_exec_methods[] = {
    {
        "execv",
        pyos_execv,
        METH_VARARGS,
        NULL
    },
    {
        "execvp",
        pyos_execvp,
        METH_VARARGS,
        NULL
    },
    {
        "execve",
        pyos_execve,
        METH_VARARGS,
        NULL
    },
    {
        "execvpe",
        pyos_execvpe,
        METH_VARARGS,
        NULL
    },
    {
        "get_exec_path",
        pyos_exec_get_exec_path,
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
pyos_get_exec_methods(void)
{
    return pyos_exec_methods;
}
