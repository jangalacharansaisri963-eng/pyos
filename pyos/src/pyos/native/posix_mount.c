#define PY_SSIZE_T_CLEAN

#include <Python.h>

#include "posix_internal.h"

static PyObject *
pyos_mount_error(
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
        message = "mount operation failed";

    PyErr_Format(
        PyExc_OSError,
        "[%s] %s",
        operation,
        message
    );

    return NULL;
}

static PyObject *
pyos_mount_mount(
    PyObject *self,
    PyObject *args,
    PyObject *kwargs
)
{
    const char *source;
    const char *target;
    const char *filesystem;
    const char *options;
    unsigned long flags;

    static char *keywords[] = {
        "source",
        "target",
        "filesystem",
        "flags",
        "options",
        NULL
    };

    (void)self;

    source = NULL;
    filesystem = NULL;
    options = NULL;
    flags = 0;

    if (
        !PyArg_ParseTupleAndKeywords(
            args,
            kwargs,
            "ss|skz:mount",
            keywords,
            &source,
            &target,
            &filesystem,
            &flags,
            &options
        )
    )
        return NULL;

    if (
        pyos_native_mount(
            source,
            target,
            filesystem,
            flags,
            options
        ) < 0
    )
        return pyos_mount_error(
            "mount"
        );

    Py_RETURN_NONE;
}

static PyObject *
pyos_mount_umount(
    PyObject *self,
    PyObject *args
)
{
    const char *target;
    unsigned long flags;

    (void)self;

    flags = 0;

    if (
        !PyArg_ParseTuple(
            args,
            "s|k:umount",
            &target,
            &flags
        )
    )
        return NULL;

    if (
        pyos_native_umount(
            target,
            flags
        ) < 0
    )
        return pyos_mount_error(
            "umount"
        );

    Py_RETURN_NONE;
}

static PyObject *
pyos_mount_remount(
    PyObject *self,
    PyObject *args,
    PyObject *kwargs
)
{
    const char *target;
    unsigned long flags;
    const char *options;

    static char *keywords[] = {
        "target",
        "flags",
        "options",
        NULL
    };

    (void)self;

    flags = 0;
    options = NULL;

    if (
        !PyArg_ParseTupleAndKeywords(
            args,
            kwargs,
            "s|kz:remount",
            keywords,
            &target,
            &flags,
            &options
        )
    )
        return NULL;

    if (
        pyos_native_remount(
            target,
            flags,
            options
        ) < 0
    )
        return pyos_mount_error(
            "remount"
        );

    Py_RETURN_NONE;
}

static PyObject *
pyos_mount_bind(
    PyObject *self,
    PyObject *args
)
{
    const char *source;
    const char *target;
    unsigned long flags;

    (void)self;

    flags = 0;

    if (
        !PyArg_ParseTuple(
            args,
            "ss|k:bind",
            &source,
            &target,
            &flags
        )
    )
        return NULL;

    if (
        pyos_native_bind_mount(
            source,
            target,
            flags
        ) < 0
    )
        return pyos_mount_error(
            "bind_mount"
        );

    Py_RETURN_NONE;
}

static PyObject *
pyos_mount_move(
    PyObject *self,
    PyObject *args
)
{
    const char *source;
    const char *target;
    unsigned long flags;

    (void)self;

    flags = 0;

    if (
        !PyArg_ParseTuple(
            args,
            "ss|k:move_mount",
            &source,
            &target,
            &flags
        )
    )
        return NULL;

    if (
        pyos_native_move_mount(
            source,
            target,
            flags
        ) < 0
    )
        return pyos_mount_error(
            "move_mount"
        );

    Py_RETURN_NONE;
}

static PyObject *
pyos_mount_readonly(
    PyObject *self,
    PyObject *args
)
{
    const char *target;
    unsigned long flags;

    (void)self;

    flags = 0;

    if (
        !PyArg_ParseTuple(
            args,
            "s|k:readonly",
            &target,
            &flags
        )
    )
        return NULL;

    if (
        pyos_native_mount_readonly(
            target,
            flags
        ) < 0
    )
        return pyos_mount_error(
            "readonly"
        );

    Py_RETURN_NONE;
}

static PyObject *
pyos_mount_is_mounted(
    PyObject *self,
    PyObject *args
)
{
    const char *target;
    int result;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "s:is_mounted",
            &target
        )
    )
        return NULL;

    result = pyos_native_is_mounted(
        target
    );

    if (result < 0)
        return pyos_mount_error(
            "is_mounted"
        );

    return PyBool_FromLong(
        result != 0
    );
}

static PyObject *
pyos_mount_filesystem_type(
    PyObject *self,
    PyObject *args
)
{
    const char *target;
    char buffer[
        PYOS_PATH_MAX
    ];
    long long length;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "s:filesystem_type",
            &target
        )
    )
        return NULL;

    length = pyos_native_get_mount_type(
        target,
        buffer,
        sizeof(buffer)
    );

    if (length < 0)
        return pyos_mount_error(
            "filesystem_type"
        );

    if (
        (unsigned long long)length >=
        sizeof(buffer)
    )
        length = sizeof(buffer) - 1;

    buffer[length] = '\0';

    return PyUnicode_FromStringAndSize(
        buffer,
        (Py_ssize_t)length
    );
}

static PyMethodDef pyos_mount_methods[] = {
    {
        "mount",
        (PyCFunction)pyos_mount_mount,
        METH_VARARGS | METH_KEYWORDS,
        NULL
    },
    {
        "umount",
        pyos_mount_umount,
        METH_VARARGS,
        NULL
    },
    {
        "remount",
        (PyCFunction)pyos_mount_remount,
        METH_VARARGS | METH_KEYWORDS,
        NULL
    },
    {
        "bind_mount",
        pyos_mount_bind,
        METH_VARARGS,
        NULL
    },
    {
        "move_mount",
        pyos_mount_move,
        METH_VARARGS,
        NULL
    },
    {
        "readonly",
        pyos_mount_readonly,
        METH_VARARGS,
        NULL
    },
    {
        "is_mounted",
        pyos_mount_is_mounted,
        METH_VARARGS,
        NULL
    },
    {
        "filesystem_type",
        pyos_mount_filesystem_type,
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
pyos_get_mount_methods(void)
{
    return pyos_mount_methods;
}
