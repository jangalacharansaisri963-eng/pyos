#define PY_SSIZE_T_CLEAN

#include <Python.h>
#include "posix_internal.h"

#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

static PyObject *
pyos_fs_getcwd(
    PyObject *self,
    PyObject *args
)
{
    char buffer[PYOS_PATH_MAX];
    char *result;

    (void)self;
    (void)args;

    result = getcwd(
        buffer,
        sizeof(buffer)
    );

    if (result == NULL) {
        return pyos_raise_errno();
    }

    return PyUnicode_DecodeFSDefault(
        result
    );
}

static PyObject *
pyos_fs_chdir(
    PyObject *self,
    PyObject *args
)
{
    PyObject *path_object;
    pyos_path_buffer path;

    (void)self;

    if (!PyArg_ParseTuple(
        args,
        "O:chdir",
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
        pyos_path_validate(&path) < 0
    ) {
        pyos_path_release(&path);
        return NULL;
    }

    if (
        chdir(path.data) < 0
    ) {
        pyos_path_release(&path);
        return pyos_raise_errno_with_path(
            path.data
        );
    }

    pyos_path_release(&path);

    Py_RETURN_NONE;
}

static PyObject *
pyos_fs_mkdir(
    PyObject *self,
    PyObject *args,
    PyObject *kwargs
)
{
    PyObject *path_object;
    PyObject *mode_object = NULL;
    long long mode = 0777;
    pyos_path_buffer path;

    static char *keywords[] = {
        "path",
        "mode",
        NULL
    };

    (void)self;

    if (!PyArg_ParseTupleAndKeywords(
        args,
        kwargs,
        "O|O:mkdir",
        keywords,
        &path_object,
        &mode_object
    )) {
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

    if (
        mkdir(
            path.data,
            (unsigned int)mode
        ) < 0
    ) {
        PyObject *error;

        error = pyos_raise_errno_with_path(
            path.data
        );

        pyos_path_release(&path);

        return error;
    }

    pyos_path_release(&path);

    Py_RETURN_NONE;
}

static PyObject *
pyos_fs_rmdir(
    PyObject *self,
    PyObject *args
)
{
    PyObject *path_object;
    pyos_path_buffer path;

    (void)self;

    if (!PyArg_ParseTuple(
        args,
        "O:rmdir",
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
        pyos_path_validate(&path) < 0
    ) {
        pyos_path_release(&path);
        return NULL;
    }

    if (
        rmdir(path.data) < 0
    ) {
        PyObject *error;

        error = pyos_raise_errno_with_path(
            path.data
        );

        pyos_path_release(&path);

        return error;
    }

    pyos_path_release(&path);

    Py_RETURN_NONE;
}

static PyObject *
pyos_fs_remove(
    PyObject *self,
    PyObject *args
)
{
    PyObject *path_object;
    pyos_path_buffer path;

    (void)self;

    if (!PyArg_ParseTuple(
        args,
        "O:remove",
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
        pyos_path_validate(&path) < 0
    ) {
        pyos_path_release(&path);
        return NULL;
    }

    if (
        unlink(path.data) < 0
    ) {
        PyObject *error;

        error = pyos_raise_errno_with_path(
            path.data
        );

        pyos_path_release(&path);

        return error;
    }

    pyos_path_release(&path);

    Py_RETURN_NONE;
}

static PyObject *
pyos_fs_unlink(
    PyObject *self,
    PyObject *args
)
{
    return pyos_fs_remove(
        self,
        args
    );
}

static PyObject *
pyos_fs_rename(
    PyObject *self,
    PyObject *args
)
{
    PyObject *old_object;
    PyObject *new_object;
    pyos_path_buffer old_path;
    pyos_path_buffer new_path;

    (void)self;

    if (!PyArg_ParseTuple(
        args,
        "OO:rename",
        &old_object,
        &new_object
    )) {
        return NULL;
    }

    if (
        pyos_path_from_object(
            old_object,
            &old_path
        ) < 0
    ) {
        return NULL;
    }

    if (
        pyos_path_from_object(
            new_object,
            &new_path
        ) < 0
    ) {
        pyos_path_release(&old_path);
        return NULL;
    }

    if (
        pyos_path_validate(&old_path) < 0 ||
        pyos_path_validate(&new_path) < 0
    ) {
        pyos_path_release(&old_path);
        pyos_path_release(&new_path);
        return NULL;
    }

    if (
        rename(
            old_path.data,
            new_path.data
        ) < 0
    ) {
        PyObject *error;

        error = pyos_raise_errno_with_path(
            old_path.data
        );

        pyos_path_release(&old_path);
        pyos_path_release(&new_path);

        return error;
    }

    pyos_path_release(&old_path);
    pyos_path_release(&new_path);

    Py_RETURN_NONE;
}

static PyObject *
pyos_fs_replace(
    PyObject *self,
    PyObject *args
)
{
    return pyos_fs_rename(
        self,
        args
    );
}

static PyObject *
pyos_fs_chmod(
    PyObject *self,
    PyObject *args
)
{
    PyObject *path_object;
    PyObject *mode_object;
    pyos_path_buffer path;
    long long mode;

    (void)self;

    if (!PyArg_ParseTuple(
        args,
        "OO:chmod",
        &path_object,
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

    if (
        chmod(
            path.data,
            (unsigned int)mode
        ) < 0
    ) {
        PyObject *error;

        error = pyos_raise_errno_with_path(
            path.data
        );

        pyos_path_release(&path);

        return error;
    }

    pyos_path_release(&path);

    Py_RETURN_NONE;
}

static PyObject *
pyos_fs_access(
    PyObject *self,
    PyObject *args
)
{
    PyObject *path_object;
    PyObject *mode_object;
    pyos_path_buffer path;
    long long mode;
    int result;

    (void)self;

    if (!PyArg_ParseTuple(
        args,
        "OO:access",
        &path_object,
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

    result = access(
        path.data,
        (int)mode
    );

    pyos_path_release(&path);

    return pyos_bool_result(
        result == 0
    );
}

static PyObject *
pyos_fs_truncate(
    PyObject *self,
    PyObject *args
)
{
    PyObject *path_object;
    PyObject *length_object;
    pyos_path_buffer path;
    long long length;

    (void)self;

    if (!PyArg_ParseTuple(
        args,
        "OO:truncate",
        &path_object,
        &length_object
    )) {
        return NULL;
    }

    if (
        pyos_require_integer(
            length_object,
            "length",
            &length
        ) < 0
    ) {
        return NULL;
    }

    if (length < 0) {
        PyErr_SetString(
            PyExc_ValueError,
            "negative length"
        );

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
        pyos_path_validate(&path) < 0
    ) {
        pyos_path_release(&path);
        return NULL;
    }

    if (
        truncate(
            path.data,
            length
        ) < 0
    ) {
        PyObject *error;

        error = pyos_raise_errno_with_path(
            path.data
        );

        pyos_path_release(&path);

        return error;
    }

    pyos_path_release(&path);

    Py_RETURN_NONE;
}

static PyObject *
pyos_fs_symlink(
    PyObject *self,
    PyObject *args
)
{
    PyObject *source_object;
    PyObject *destination_object;
    pyos_path_buffer source;
    pyos_path_buffer destination;

    (void)self;

    if (!PyArg_ParseTuple(
        args,
        "OO:symlink",
        &source_object,
        &destination_object
    )) {
        return NULL;
    }

    if (
        pyos_path_from_object(
            source_object,
            &source
        ) < 0
    ) {
        return NULL;
    }

    if (
        pyos_path_from_object(
            destination_object,
            &destination
        ) < 0
    ) {
        pyos_path_release(&source);
        return NULL;
    }

    if (
        pyos_path_validate(&source) < 0 ||
        pyos_path_validate(&destination) < 0
    ) {
        pyos_path_release(&source);
        pyos_path_release(&destination);
        return NULL;
    }

    if (
        symlink(
            source.data,
            destination.data
        ) < 0
    ) {
        PyObject *error;

        error = pyos_raise_errno_with_path(
            destination.data
        );

        pyos_path_release(&source);
        pyos_path_release(&destination);

        return error;
    }

    pyos_path_release(&source);
    pyos_path_release(&destination);

    Py_RETURN_NONE;
}

static PyObject *
pyos_fs_link(
    PyObject *self,
    PyObject *args
)
{
    PyObject *source_object;
    PyObject *destination_object;
    pyos_path_buffer source;
    pyos_path_buffer destination;

    (void)self;

    if (!PyArg_ParseTuple(
        args,
        "OO:link",
        &source_object,
        &destination_object
    )) {
        return NULL;
    }

    if (
        pyos_path_from_object(
            source_object,
            &source
        ) < 0
    ) {
        return NULL;
    }

    if (
        pyos_path_from_object(
            destination_object,
            &destination
        ) < 0
    ) {
        pyos_path_release(&source);
        return NULL;
    }

    if (
        pyos_path_validate(&source) < 0 ||
        pyos_path_validate(&destination) < 0
    ) {
        pyos_path_release(&source);
        pyos_path_release(&destination);
        return NULL;
    }

    if (
        link(
            source.data,
            destination.data
        ) < 0
    ) {
        PyObject *error;

        error = pyos_raise_errno_with_path(
            destination.data
        );

        pyos_path_release(&source);
        pyos_path_release(&destination);

        return error;
    }

    pyos_path_release(&source);
    pyos_path_release(&destination);

    Py_RETURN_NONE;
}

static PyObject *
pyos_fs_readlink(
    PyObject *self,
    PyObject *args
)
{
    PyObject *path_object;
    pyos_path_buffer path;
    char buffer[PYOS_PATH_MAX];
    long result;

    (void)self;

    if (!PyArg_ParseTuple(
        args,
        "O:readlink",
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
        pyos_path_validate(&path) < 0
    ) {
        pyos_path_release(&path);
        return NULL;
    }

    result = readlink(
        path.data,
        buffer,
        sizeof(buffer)
    );

    if (result < 0) {
        PyObject *error;

        error = pyos_raise_errno_with_path(
            path.data
        );

        pyos_path_release(&path);

        return error;
    }

    pyos_path_release(&path);

    return pyos_path_result(
        path_object,
        buffer,
        result
    );
}

PyMethodDef pyos_fs_methods[] = {
    {
        "getcwd",
        pyos_fs_getcwd,
        METH_NOARGS,
        NULL
    },
    {
        "chdir",
        pyos_fs_chdir,
        METH_VARARGS,
        NULL
    },
    {
        "mkdir",
        (PyCFunction)pyos_fs_mkdir,
        METH_VARARGS | METH_KEYWORDS,
        NULL
    },
    {
        "rmdir",
        pyos_fs_rmdir,
        METH_VARARGS,
        NULL
    },
    {
        "remove",
        pyos_fs_remove,
        METH_VARARGS,
        NULL
    },
    {
        "unlink",
        pyos_fs_unlink,
        METH_VARARGS,
        NULL
    },
    {
        "rename",
        pyos_fs_rename,
        METH_VARARGS,
        NULL
    },
    {
        "replace",
        pyos_fs_replace,
        METH_VARARGS,
        NULL
    },
    {
        "chmod",
        pyos_fs_chmod,
        METH_VARARGS,
        NULL
    },
    {
        "access",
        pyos_fs_access,
        METH_VARARGS,
        NULL
    },
    {
        "truncate",
        pyos_fs_truncate,
        METH_VARARGS,
        NULL
    },
    {
        "symlink",
        pyos_fs_symlink,
        METH_VARARGS,
        NULL
    },
    {
        "link",
        pyos_fs_link,
        METH_VARARGS,
        NULL
    },
    {
        "readlink",
        pyos_fs_readlink,
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
pyos_get_fs_methods(void)
{
    return pyos_fs_methods;
}
