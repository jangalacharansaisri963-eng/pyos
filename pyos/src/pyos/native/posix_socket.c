#define PY_SSIZE_T_CLEAN

#include <Python.h>

#include "posix_internal.h"

static PyObject *
pyos_socket_error(
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
        message = "socket operation failed";

    PyErr_Format(
        PyExc_OSError,
        "[%s] %s",
        operation,
        message
    );

    return NULL;
}

static PyObject *
pyos_socket_create(
    PyObject *self,
    PyObject *args
)
{
    int domain;
    int type;
    int protocol;
    int fd;

    (void)self;

    protocol = 0;

    if (
        !PyArg_ParseTuple(
            args,
            "ii|i:socket",
            &domain,
            &type,
            &protocol
        )
    )
        return NULL;

    fd = pyos_native_socket(
        domain,
        type,
        protocol
    );

    if (fd < 0)
        return pyos_socket_error(
            "socket"
        );

    return PyLong_FromLong(fd);
}

static PyObject *
pyos_socket_bind(
    PyObject *self,
    PyObject *args
)
{
    int fd;
    Py_buffer address;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "iy*:bind",
            &fd,
            &address
        )
    )
        return NULL;

    if (
        pyos_native_bind(
            fd,
            address.buf,
            address.len
        ) < 0
    ) {
        PyBuffer_Release(&address);

        return pyos_socket_error(
            "bind"
        );
    }

    PyBuffer_Release(&address);

    Py_RETURN_NONE;
}

static PyObject *
pyos_socket_connect(
    PyObject *self,
    PyObject *args
)
{
    int fd;
    Py_buffer address;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "iy*:connect",
            &fd,
            &address
        )
    )
        return NULL;

    if (
        pyos_native_connect(
            fd,
            address.buf,
            address.len
        ) < 0
    ) {
        PyBuffer_Release(&address);

        return pyos_socket_error(
            "connect"
        );
    }

    PyBuffer_Release(&address);

    Py_RETURN_NONE;
}

static PyObject *
pyos_socket_listen(
    PyObject *self,
    PyObject *args
)
{
    int fd;
    int backlog;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "ii:listen",
            &fd,
            &backlog
        )
    )
        return NULL;

    if (
        pyos_native_listen(
            fd,
            backlog
        ) < 0
    )
        return pyos_socket_error(
            "listen"
        );

    Py_RETURN_NONE;
}

static PyObject *
pyos_socket_accept(
    PyObject *self,
    PyObject *args
)
{
    int fd;
    int result;
    pyos_socket_address address;
    PyObject *python_address;

    (void)self;
    (void)args;

    if (
        !PyArg_ParseTuple(
            args,
            "i:accept",
            &fd
        )
    )
        return NULL;

    result = pyos_native_accept(
        fd,
        &address
    );

    if (result < 0)
        return pyos_socket_error(
            "accept"
        );

    python_address =
        pyos_socket_address_to_object(
            &address
        );

    if (python_address == NULL) {
        pyos_native_close(result);
        return NULL;
    }

    return Py_BuildValue(
        "(iN)",
        result,
        python_address
    );
}

static PyObject *
pyos_socket_send(
    PyObject *self,
    PyObject *args
)
{
    int fd;
    Py_buffer buffer;
    int flags;
    long long result;

    (void)self;

    flags = 0;

    if (
        !PyArg_ParseTuple(
            args,
            "iy*|i:send",
            &fd,
            &buffer,
            &flags
        )
    )
        return NULL;

    result = pyos_native_send(
        fd,
        buffer.buf,
        (unsigned long)buffer.len,
        flags
    );

    PyBuffer_Release(&buffer);

    if (result < 0)
        return pyos_socket_error(
            "send"
        );

    return PyLong_FromLongLong(
        result
    );
}

static PyObject *
pyos_socket_recv(
    PyObject *self,
    PyObject *args
)
{
    int fd;
    Py_ssize_t size;
    int flags;
    char *buffer;
    long long result;
    PyObject *value;

    (void)self;

    flags = 0;

    if (
        !PyArg_ParseTuple(
            args,
            "in|i:recv",
            &fd,
            &size,
            &flags
        )
    )
        return NULL;

    if (size < 0) {
        PyErr_SetString(
            PyExc_ValueError,
            "size must be non-negative"
        );

        return NULL;
    }

    buffer = PyMem_Malloc(
        size == 0 ? 1 : (size_t)size
    );

    if (buffer == NULL)
        return PyErr_NoMemory();

    result = pyos_native_recv(
        fd,
        buffer,
        (unsigned long)size,
        flags
    );

    if (result < 0) {
        PyMem_Free(buffer);

        return pyos_socket_error(
            "recv"
        );
    }

    value = PyBytes_FromStringAndSize(
        buffer,
        (Py_ssize_t)result
    );

    PyMem_Free(buffer);

    return value;
}

static PyObject *
pyos_socket_shutdown(
    PyObject *self,
    PyObject *args
)
{
    int fd;
    int how;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "ii:shutdown",
            &fd,
            &how
        )
    )
        return NULL;

    if (
        pyos_native_shutdown(
            fd,
            how
        ) < 0
    )
        return pyos_socket_error(
            "shutdown"
        );

    Py_RETURN_NONE;
}

static PyObject *
pyos_socket_getsockname(
    PyObject *self,
    PyObject *args
)
{
    int fd;
    pyos_socket_address address;
    PyObject *result;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "i:getsockname",
            &fd
        )
    )
        return NULL;

    if (
        pyos_native_getsockname(
            fd,
            &address
        ) < 0
    )
        return pyos_socket_error(
            "getsockname"
        );

    result = pyos_socket_address_to_object(
        &address
    );

    return result;
}

static PyObject *
pyos_socket_getpeername(
    PyObject *self,
    PyObject *args
)
{
    int fd;
    pyos_socket_address address;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "i:getpeername",
            &fd
        )
    )
        return NULL;

    if (
        pyos_native_getpeername(
            fd,
            &address
        ) < 0
    )
        return pyos_socket_error(
            "getpeername"
        );

    return pyos_socket_address_to_object(
        &address
    );
}

static PyObject *
pyos_socket_setsockopt(
    PyObject *self,
    PyObject *args
)
{
    int fd;
    int level;
    int option;
    Py_buffer value;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "iiiy*:setsockopt",
            &fd,
            &level,
            &option,
            &value
        )
    )
        return NULL;

    if (
        pyos_native_setsockopt(
            fd,
            level,
            option,
            value.buf,
            value.len
        ) < 0
    ) {
        PyBuffer_Release(&value);

        return pyos_socket_error(
            "setsockopt"
        );
    }

    PyBuffer_Release(&value);

    Py_RETURN_NONE;
}

static PyObject *
pyos_socket_getsockopt(
    PyObject *self,
    PyObject *args
)
{
    int fd;
    int level;
    int option;
    Py_ssize_t size;
    char *buffer;
    long long result;
    PyObject *value;

    (void)self;

    size = sizeof(long);

    if (
        !PyArg_ParseTuple(
            args,
            "iii|n:getsockopt",
            &fd,
            &level,
            &option,
            &size
        )
    )
        return NULL;

    if (size <= 0) {
        PyErr_SetString(
            PyExc_ValueError,
            "buffer size must be positive"
        );

        return NULL;
    }

    buffer = PyMem_Malloc(
        (size_t)size
    );

    if (buffer == NULL)
        return PyErr_NoMemory();

    result = pyos_native_getsockopt(
        fd,
        level,
        option,
        buffer,
        (unsigned long)size
    );

    if (result < 0) {
        PyMem_Free(buffer);

        return pyos_socket_error(
            "getsockopt"
        );
    }

    value = PyBytes_FromStringAndSize(
        buffer,
        (Py_ssize_t)result
    );

    PyMem_Free(buffer);

    return value;
}

static PyMethodDef pyos_socket_methods[] = {
    {
        "socket",
        pyos_socket_create,
        METH_VARARGS,
        NULL
    },
    {
        "bind",
        pyos_socket_bind,
        METH_VARARGS,
        NULL
    },
    {
        "connect",
        pyos_socket_connect,
        METH_VARARGS,
        NULL
    },
    {
        "listen",
        pyos_socket_listen,
        METH_VARARGS,
        NULL
    },
    {
        "accept",
        pyos_socket_accept,
        METH_VARARGS,
        NULL
    },
    {
        "send",
        pyos_socket_send,
        METH_VARARGS,
        NULL
    },
    {
        "recv",
        pyos_socket_recv,
        METH_VARARGS,
        NULL
    },
    {
        "shutdown",
        pyos_socket_shutdown,
        METH_VARARGS,
        NULL
    },
    {
        "getsockname",
        pyos_socket_getsockname,
        METH_VARARGS,
        NULL
    },
    {
        "getpeername",
        pyos_socket_getpeername,
        METH_VARARGS,
        NULL
    },
    {
        "setsockopt",
        pyos_socket_setsockopt,
        METH_VARARGS,
        NULL
    },
    {
        "getsockopt",
        pyos_socket_getsockopt,
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
pyos_get_socket_methods(void)
{
    return pyos_socket_methods;
}
