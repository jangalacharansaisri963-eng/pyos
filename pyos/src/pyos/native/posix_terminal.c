#define PY_SSIZE_T_CLEAN

#include <Python.h>

#include "posix_internal.h"

static PyObject *
pyos_terminal_error(
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
        message = "native terminal operation failed";
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
pyos_terminal_isatty(
    PyObject *self,
    PyObject *args
)
{
    int fd;
    int result;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "i:isatty",
            &fd
        )
    ) {
        return NULL;
    }

    result = pyos_native_isatty(fd);

    if (result < 0) {
        return pyos_terminal_error(
            "isatty"
        );
    }

    return PyBool_FromLong(
        result != 0
    );
}

static PyObject *
pyos_terminal_get_winsize(
    PyObject *self,
    PyObject *args
)
{
    int fd;
    unsigned long rows;
    unsigned long columns;
    unsigned long x_pixels;
    unsigned long y_pixels;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "i:get_winsize",
            &fd
        )
    ) {
        return NULL;
    }

    if (
        pyos_native_terminal_size(
            fd,
            &rows,
            &columns,
            &x_pixels,
            &y_pixels
        ) < 0
    ) {
        return pyos_terminal_error(
            "terminal_size"
        );
    }

    return Py_BuildValue(
        "{s:K,s:K,s:K,s:K}",
        "rows",
        rows,
        "columns",
        columns,
        "x_pixels",
        x_pixels,
        "y_pixels",
        y_pixels
    );
}

static PyObject *
pyos_terminal_set_winsize(
    PyObject *self,
    PyObject *args
)
{
    int fd;
    unsigned long rows;
    unsigned long columns;
    unsigned long x_pixels;
    unsigned long y_pixels;

    (void)self;

    x_pixels = 0;
    y_pixels = 0;

    if (
        !PyArg_ParseTuple(
            args,
            "iKK|KK:set_winsize",
            &fd,
            &rows,
            &columns,
            &x_pixels,
            &y_pixels
        )
    ) {
        return NULL;
    }

    if (
        pyos_native_set_terminal_size(
            fd,
            rows,
            columns,
            x_pixels,
            y_pixels
        ) < 0
    ) {
        return pyos_terminal_error(
            "set_terminal_size"
        );
    }

    Py_RETURN_NONE;
}

static PyObject *
pyos_terminal_get_foreground_pgrp(
    PyObject *self,
    PyObject *args
)
{
    int fd;
    long long pgid;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "i:get_foreground_pgrp",
            &fd
        )
    ) {
        return NULL;
    }

    pgid =
        pyos_native_tcgetpgrp(fd);

    if (pgid < 0) {
        return pyos_terminal_error(
            "tcgetpgrp"
        );
    }

    return PyLong_FromLongLong(
        pgid
    );
}

static PyObject *
pyos_terminal_set_foreground_pgrp(
    PyObject *self,
    PyObject *args
)
{
    int fd;
    long long pgid;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "iL:set_foreground_pgrp",
            &fd,
            &pgid
        )
    ) {
        return NULL;
    }

    if (
        pyos_native_tcsetpgrp(
            fd,
            pgid
        ) < 0
    ) {
        return pyos_terminal_error(
            "tcsetpgrp"
        );
    }

    Py_RETURN_NONE;
}

static PyObject *
pyos_terminal_get_process_group(
    PyObject *self,
    PyObject *args
)
{
    int fd;
    long long pgid;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "i:get_process_group",
            &fd
        )
    ) {
        return NULL;
    }

    pgid =
        pyos_native_tcgetpgrp(fd);

    if (pgid < 0) {
        return pyos_terminal_error(
            "tcgetpgrp"
        );
    }

    return PyLong_FromLongLong(
        pgid
    );
}

static PyObject *
pyos_terminal_get_attributes(
    PyObject *self,
    PyObject *args
)
{
    int fd;
    PyObject *attributes;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "i:get_attributes",
            &fd
        )
    ) {
        return NULL;
    }

    attributes =
        pyos_native_terminal_get_attributes(
            fd
        );

    if (attributes == NULL) {
        return pyos_terminal_error(
            "tcgetattr"
        );
    }

    return attributes;
}

static PyObject *
pyos_terminal_set_attributes(
    PyObject *self,
    PyObject *args
)
{
    int fd;
    PyObject *attributes;
    int action;

    (void)self;

    action = 0;

    if (
        !PyArg_ParseTuple(
            args,
            "iO|i:set_attributes",
            &fd,
            &attributes,
            &action
        )
    ) {
        return NULL;
    }

    if (
        pyos_native_terminal_set_attributes(
            fd,
            attributes,
            action
        ) < 0
    ) {
        return pyos_terminal_error(
            "tcsetattr"
        );
    }

    Py_RETURN_NONE;
}

static PyObject *
pyos_terminal_drain(
    PyObject *self,
    PyObject *args
)
{
    int fd;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "i:drain",
            &fd
        )
    ) {
        return NULL;
    }

    if (
        pyos_native_tcdrain(fd) < 0
    ) {
        return pyos_terminal_error(
            "tcdrain"
        );
    }

    Py_RETURN_NONE;
}

static PyObject *
pyos_terminal_flush(
    PyObject *self,
    PyObject *args
)
{
    int fd;
    int selector;

    (void)self;

    selector = 2;

    if (
        !PyArg_ParseTuple(
            args,
            "i|i:flush",
            &fd,
            &selector
        )
    ) {
        return NULL;
    }

    if (
        pyos_native_tcflush(
            fd,
            selector
        ) < 0
    ) {
        return pyos_terminal_error(
            "tcflush"
        );
    }

    Py_RETURN_NONE;
}

static PyObject *
pyos_terminal_flow(
    PyObject *self,
    PyObject *args
)
{
    int fd;
    int action;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "ii:flow",
            &fd,
            &action
        )
    ) {
        return NULL;
    }

    if (
        pyos_native_tcflow(
            fd,
            action
        ) < 0
    ) {
        return pyos_terminal_error(
            "tcflow"
        );
    }

    Py_RETURN_NONE;
}

static PyObject *
pyos_terminal_break(
    PyObject *self,
    PyObject *args
)
{
    int fd;
    int duration;

    (void)self;

    duration = 0;

    if (
        !PyArg_ParseTuple(
            args,
            "i|i:send_break",
            &fd,
            &duration
        )
    ) {
        return NULL;
    }

    if (
        pyos_native_tcsendbreak(
            fd,
            duration
        ) < 0
    ) {
        return pyos_terminal_error(
            "tcsendbreak"
        );
    }

    Py_RETURN_NONE;
}

static PyObject *
pyos_terminal_set_control(
    PyObject *self,
    PyObject *args
)
{
    int fd;
    int command;
    int value;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "iii:set_control",
            &fd,
            &command,
            &value
        )
    ) {
        return NULL;
    }

    if (
        pyos_native_terminal_control(
            fd,
            command,
            value
        ) < 0
    ) {
        return pyos_terminal_error(
            "terminal_control"
        );
    }

    Py_RETURN_NONE;
}

static PyObject *
pyos_terminal_open_device(
    PyObject *self,
    PyObject *args
)
{
    const char *path;
    int flags;
    int fd;

    (void)self;

    flags = 2;

    if (
        !PyArg_ParseTuple(
            args,
            "s|i:open_device",
            &path,
            &flags
        )
    ) {
        return NULL;
    }

    fd =
        pyos_native_open_terminal(
            path,
            flags
        );

    if (fd < 0) {
        return pyos_terminal_error(
            "open_terminal"
        );
    }

    return PyLong_FromLong(fd);
}

static PyObject *
pyos_terminal_close_device(
    PyObject *self,
    PyObject *args
)
{
    int fd;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "i:close_device",
            &fd
        )
    ) {
        return NULL;
    }

    if (
        pyos_native_close(fd) < 0
    ) {
        return pyos_terminal_error(
            "close_terminal"
        );
    }

    Py_RETURN_NONE;
}

static PyObject *
pyos_terminal_is_device(
    PyObject *self,
    PyObject *args
)
{
    int fd;
    int result;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "i:is_device",
            &fd
        )
    ) {
        return NULL;
    }

    result =
        pyos_native_is_terminal_device(fd);

    if (result < 0) {
        return pyos_terminal_error(
            "is_terminal_device"
        );
    }

    return PyBool_FromLong(
        result != 0
    );
}

static PyMethodDef pyos_terminal_methods[] = {
    {
        "isatty",
        pyos_terminal_isatty,
        METH_VARARGS,
        NULL
    },
    {
        "get_winsize",
        pyos_terminal_get_winsize,
        METH_VARARGS,
        NULL
    },
    {
        "set_winsize",
        pyos_terminal_set_winsize,
        METH_VARARGS,
        NULL
    },
    {
        "get_foreground_pgrp",
        pyos_terminal_get_foreground_pgrp,
        METH_VARARGS,
        NULL
    },
    {
        "set_foreground_pgrp",
        pyos_terminal_set_foreground_pgrp,
        METH_VARARGS,
        NULL
    },
    {
        "get_process_group",
        pyos_terminal_get_process_group,
        METH_VARARGS,
        NULL
    },
    {
        "get_attributes",
        pyos_terminal_get_attributes,
        METH_VARARGS,
        NULL
    },
    {
        "set_attributes",
        pyos_terminal_set_attributes,
        METH_VARARGS,
        NULL
    },
    {
        "drain",
        pyos_terminal_drain,
        METH_VARARGS,
        NULL
    },
    {
        "flush",
        pyos_terminal_flush,
        METH_VARARGS,
        NULL
    },
    {
        "flow",
        pyos_terminal_flow,
        METH_VARARGS,
        NULL
    },
    {
        "send_break",
        pyos_terminal_break,
        METH_VARARGS,
        NULL
    },
    {
        "set_control",
        pyos_terminal_set_control,
        METH_VARARGS,
        NULL
    },
    {
        "open_device",
        pyos_terminal_open_device,
        METH_VARARGS,
        NULL
    },
    {
        "close_device",
        pyos_terminal_close_device,
        METH_VARARGS,
        NULL
    },
    {
        "is_device",
        pyos_terminal_is_device,
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
pyos_get_terminal_methods(void)
{
    return pyos_terminal_methods;
}
