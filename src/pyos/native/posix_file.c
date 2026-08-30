#define PY_SSIZE_T_CLEAN

#include <Python.h>

#include "posix_internal.h"

typedef struct {
    int fd;
    int flags;
    int mode;
    int valid;
} pyos_file_state;

static PyObject *
pyos_file_raise_error(
    const char *operation,
    const char *path
)
{
    int error_code;
    const char *message;

    error_code = pyos_get_errno();
    message = NULL;

    if (pyos_native_strerror != NULL) {
        message = pyos_native_strerror(error_code);
    }

    if (message == NULL) {
        message = "native file operation failed";
    }

    if (path != NULL) {
        PyErr_Format(
            PyExc_OSError,
            "[%s] %s: %s",
            operation,
            path,
            message
        );
    } else {
        PyErr_Format(
            PyExc_OSError,
            "[%s] %s",
            operation,
            message
        );
    }

    return NULL;
}

static int
pyos_file_require_fd(
    PyObject *object,
    int *fd
)
{
    long long value;

    if (
        pyos_require_integer(
            object,
            "fd",
            &value
        ) < 0
    ) {
        return -1;
    }

    if (
        value < 0 ||
        value > 2147483647LL
    ) {
        PyErr_SetString(
            PyExc_ValueError,
            "invalid file descriptor"
        );

        return -1;
    }

    *fd = (int)value;

    return 0;
}

static PyObject *
pyos_file_open(
    PyObject *self,
    PyObject *args,
    PyObject *kwargs
)
{
    const char *path;
    int flags;
    unsigned long mode;
    int fd;

    static char *keywords[] = {
        "path",
        "flags",
        "mode",
        NULL
    };

    (void)self;

    flags = PYOS_O_RDONLY;
    mode = 0666;

    if (
        !PyArg_ParseTupleAndKeywords(
            args,
            kwargs,
            "s|ik:open",
            keywords,
            &path,
            &flags,
            &mode
        )
    ) {
        return NULL;
    }

    fd = pyos_native_open(
        path,
        flags,
        (pyos_mode_t)mode
    );

    if (fd < 0) {
        return pyos_file_raise_error(
            "open",
            path
        );
    }

    return PyLong_FromLong(fd);
}

static PyObject *
pyos_file_close(
    PyObject *self,
    PyObject *args
)
{
    int fd;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "i:close",
            &fd
        )
    ) {
        return NULL;
    }

    if (
        pyos_native_close(fd) < 0
    ) {
        return pyos_file_raise_error(
            "close",
            NULL
        );
    }

    Py_RETURN_NONE;
}

static PyObject *
pyos_file_read(
    PyObject *self,
    PyObject *args,
    PyObject *kwargs
)
{
    int fd;
    Py_ssize_t size;
    char *buffer;
    long long count;

    static char *keywords[] = {
        "fd",
        "size",
        NULL
    };

    (void)self;

    if (
        !PyArg_ParseTupleAndKeywords(
            args,
            kwargs,
            "iL:read",
            keywords,
            &fd,
            &size
        )
    ) {
        return NULL;
    }

    if (size < 0) {
        PyErr_SetString(
            PyExc_ValueError,
            "read size must not be negative"
        );

        return NULL;
    }

    if (
        size == 0
    ) {
        return PyBytes_FromStringAndSize(
            "",
            0
        );
    }

    buffer = PyMem_Malloc(
        (size_t)size
    );

    if (buffer == NULL) {
        return PyErr_NoMemory();
    }

    count = pyos_native_read(
        fd,
        buffer,
        (unsigned long)size
    );

    if (count < 0) {
        PyMem_Free(buffer);

        return pyos_file_raise_error(
            "read",
            NULL
        );
    }

    {
        PyObject *result;

        result = PyBytes_FromStringAndSize(
            buffer,
            (Py_ssize_t)count
        );

        PyMem_Free(buffer);

        return result;
    }
}

static PyObject *
pyos_file_write(
    PyObject *self,
    PyObject *args
)
{
    int fd;
    const char *data;
    Py_ssize_t size;
    Py_ssize_t offset;
    Py_ssize_t total;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "iy#:write",
            &fd,
            &data,
            &size
        )
    ) {
        return NULL;
    }

    total = 0;
    offset = 0;

    while (
        offset < size
    ) {
        long long count;

        count = pyos_native_write(
            fd,
            data + offset,
            (unsigned long)(
                size - offset
            )
        );

        if (count < 0) {
            if (total > 0) {
                return PyLong_FromSsize_t(
                    total
                );
            }

            return pyos_file_raise_error(
                "write",
                NULL
            );
        }

        if (count == 0) {
            break;
        }

        offset += (Py_ssize_t)count;
        total += (Py_ssize_t)count;
    }

    return PyLong_FromSsize_t(total);
}

static PyObject *
pyos_file_pread(
    PyObject *self,
    PyObject *args
)
{
    int fd;
    Py_ssize_t size;
    long long offset;
    pyos_off_t original;
    pyos_off_t position;
    PyObject *result;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "iLn:pread",
            &fd,
            &size,
            &offset
        )
    ) {
        return NULL;
    }

    if (size < 0) {
        PyErr_SetString(
            PyExc_ValueError,
            "read size must not be negative"
        );

        return NULL;
    }

    original = pyos_native_lseek(
        fd,
        0,
        PYOS_SEEK_CUR
    );

    if (original < 0) {
        return pyos_file_raise_error(
            "pread",
            NULL
        );
    }

    position = pyos_native_lseek(
        fd,
        (pyos_off_t)offset,
        PYOS_SEEK_SET
    );

    if (position < 0) {
        return pyos_file_raise_error(
            "pread",
            NULL
        );
    }

    {
        PyObject *read_args;
        PyObject *fd_object;
        PyObject *size_object;

        fd_object = PyLong_FromLong(fd);

        if (fd_object == NULL) {
            pyos_native_lseek(
                fd,
                original,
                PYOS_SEEK_SET
            );

            return NULL;
        }

        size_object = PyLong_FromSsize_t(size);

        if (size_object == NULL) {
            Py_DECREF(fd_object);

            pyos_native_lseek(
                fd,
                original,
                PYOS_SEEK_SET
            );

            return NULL;
        }

        read_args = PyTuple_Pack(
            2,
            fd_object,
            size_object
        );

        Py_DECREF(fd_object);
        Py_DECREF(size_object);

        if (read_args == NULL) {
            pyos_native_lseek(
                fd,
                original,
                PYOS_SEEK_SET
            );

            return NULL;
        }

        result = pyos_file_read(
            NULL,
            read_args,
            NULL
        );

        Py_DECREF(read_args);
    }

    pyos_native_lseek(
        fd,
        original,
        PYOS_SEEK_SET
    );

    return result;
}

static PyObject *
pyos_file_pwrite(
    PyObject *self,
    PyObject *args
)
{
    int fd;
    const char *data;
    Py_ssize_t size;
    long long offset;
    pyos_off_t original;
    pyos_off_t position;
    Py_ssize_t total;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "iy#L:pwrite",
            &fd,
            &data,
            &size,
            &offset
        )
    ) {
        return NULL;
    }

    original = pyos_native_lseek(
        fd,
        0,
        PYOS_SEEK_CUR
    );

    if (original < 0) {
        return pyos_file_raise_error(
            "pwrite",
            NULL
        );
    }

    position = pyos_native_lseek(
        fd,
        (pyos_off_t)offset,
        PYOS_SEEK_SET
    );

    if (position < 0) {
        return pyos_file_raise_error(
            "pwrite",
            NULL
        );
    }

    total = 0;

    while (
        total < size
    ) {
        long long count;

        count = pyos_native_write(
            fd,
            data + total,
            (unsigned long)(
                size - total
            )
        );

        if (count < 0) {
            pyos_native_lseek(
                fd,
                original,
                PYOS_SEEK_SET
            );

            if (total > 0) {
                return PyLong_FromSsize_t(
                    total
                );
            }

            return pyos_file_raise_error(
                "pwrite",
                NULL
            );
        }

        if (count == 0) {
            break;
        }

        total += (Py_ssize_t)count;
    }

    pyos_native_lseek(
        fd,
        original,
        PYOS_SEEK_SET
    );

    return PyLong_FromSsize_t(total);
}

static PyObject *
pyos_file_seek(
    PyObject *self,
    PyObject *args
)
{
    int fd;
    long long offset;
    int whence;
    pyos_off_t position;

    (void)self;

    whence = PYOS_SEEK_SET;

    if (
        !PyArg_ParseTuple(
            args,
            "iL|i:seek",
            &fd,
            &offset,
            &whence
        )
    ) {
        return NULL;
    }

    if (
        whence != PYOS_SEEK_SET &&
        whence != PYOS_SEEK_CUR &&
        whence != PYOS_SEEK_END
    ) {
        PyErr_SetString(
            PyExc_ValueError,
            "invalid seek mode"
        );

        return NULL;
    }

    position = pyos_native_lseek(
        fd,
        (pyos_off_t)offset,
        whence
    );

    if (position < 0) {
        return pyos_file_raise_error(
            "seek",
            NULL
        );
    }

    return PyLong_FromLongLong(
        (long long)position
    );
}

static PyObject *
pyos_file_tell(
    PyObject *self,
    PyObject *args
)
{
    int fd;
    pyos_off_t position;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "i:tell",
            &fd
        )
    ) {
        return NULL;
    }

    position = pyos_native_lseek(
        fd,
        0,
        PYOS_SEEK_CUR
    );

    if (position < 0) {
        return pyos_file_raise_error(
            "tell",
            NULL
        );
    }

    return PyLong_FromLongLong(
        (long long)position
    );
}

static PyObject *
pyos_file_truncate(
    PyObject *self,
    PyObject *args
)
{
    int fd;
    long long length;
    pyos_off_t current;
    pyos_off_t result;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "iL:truncate",
            &fd,
            &length
        )
    ) {
        return NULL;
    }

    if (length < 0) {
        PyErr_SetString(
            PyExc_ValueError,
            "truncate length must not be negative"
        );

        return NULL;
    }

    current = pyos_native_lseek(
        fd,
        0,
        PYOS_SEEK_CUR
    );

    if (current < 0) {
        current = 0;
    }

    result = pyos_native_lseek(
        fd,
        (pyos_off_t)length,
        PYOS_SEEK_SET
    );

    if (result < 0) {
        return pyos_file_raise_error(
            "truncate",
            NULL
        );
    }

    result = pyos_native_lseek(
        fd,
        current,
        PYOS_SEEK_SET
    );

    if (result < 0) {
        return pyos_file_raise_error(
            "truncate",
            NULL
        );
    }

    PyErr_SetString(
        PyExc_NotImplementedError,
        "fd-based truncate requires the native ftruncate operation"
    );

    return NULL;
}

static PyObject *
pyos_file_fsync(
    PyObject *self,
    PyObject *args
)
{
    int fd;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "i:fsync",
            &fd
        )
    ) {
        return NULL;
    }

    if (
        pyos_native_fsync(fd) < 0
    ) {
        return pyos_file_raise_error(
            "fsync",
            NULL
        );
    }

    Py_RETURN_NONE;
}

static PyObject *
pyos_file_fstat(
    PyObject *self,
    PyObject *args
)
{
    int fd;
    pyos_stat_result info;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "i:fstat",
            &fd
        )
    ) {
        return NULL;
    }

    if (
        pyos_native_fstat(
            fd,
            &info
        ) < 0
    ) {
        return pyos_file_raise_error(
            "fstat",
            NULL
        );
    }

    return pyos_make_stat_dict(
        &info
    );
}

static PyObject *
pyos_file_dup(
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
            "i:dup",
            &fd
        )
    ) {
        return NULL;
    }

    result = pyos_native_dup(fd);

    if (result < 0) {
        return pyos_file_raise_error(
            "dup",
            NULL
        );
    }

    return PyLong_FromLong(result);
}

static PyObject *
pyos_file_dup2(
    PyObject *self,
    PyObject *args
)
{
    int oldfd;
    int newfd;
    int result;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "ii:dup2",
            &oldfd,
            &newfd
        )
    ) {
        return NULL;
    }

    result = pyos_native_dup2(
        oldfd,
        newfd
    );

    if (result < 0) {
        return pyos_file_raise_error(
            "dup2",
            NULL
        );
    }

    return PyLong_FromLong(result);
}

static PyObject *
pyos_file_pipe(
    PyObject *self,
    PyObject *args
)
{
    int descriptors[2];

    (void)self;
    (void)args;

    if (
        pyos_native_pipe(
            descriptors
        ) < 0
    ) {
        return pyos_file_raise_error(
            "pipe",
            NULL
        );
    }

    return Py_BuildValue(
        "(ii)",
        descriptors[0],
        descriptors[1]
    );
}

static PyObject *
pyos_file_isatty(
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
        PyErr_Clear();
        Py_RETURN_FALSE;
    }

    return pyos_bool_result(result);
}

static PyObject *
pyos_file_readall(
    PyObject *self,
    PyObject *args
)
{
    int fd;
    unsigned long chunk;
    PyObject *result;
    PyObject *parts;
    unsigned long total;
    unsigned long capacity;

    (void)self;

    chunk = 65536;

    if (
        !PyArg_ParseTuple(
            args,
            "i|k:readall",
            &fd,
            &chunk
        )
    ) {
        return NULL;
    }

    if (chunk == 0) {
        chunk = 65536;
    }

    parts = PyList_New(0);

    if (parts == NULL) {
        return NULL;
    }

    total = 0;
    capacity = 0;

    for (;;) {
        char *buffer;
        long long count;
        PyObject *piece;

        buffer = PyMem_Malloc(chunk);

        if (buffer == NULL) {
            Py_DECREF(parts);
            return PyErr_NoMemory();
        }

        count = pyos_native_read(
            fd,
            buffer,
            chunk
        );

        if (count < 0) {
            PyMem_Free(buffer);
            Py_DECREF(parts);

            return pyos_file_raise_error(
                "readall",
                NULL
            );
        }

        if (count == 0) {
            PyMem_Free(buffer);
            break;
        }

        piece = PyBytes_FromStringAndSize(
            buffer,
            (Py_ssize_t)count
        );

        PyMem_Free(buffer);

        if (piece == NULL) {
            Py_DECREF(parts);
            return NULL;
        }

        if (
            PyList_Append(
                parts,
                piece
            ) < 0
        ) {
            Py_DECREF(piece);
            Py_DECREF(parts);
            return NULL;
        }

        Py_DECREF(piece);

        total += (unsigned long)count;

        if (total > capacity) {
            capacity = total;
        }
    }

    result = PyBytes_FromStringAndSize(
        NULL,
        (Py_ssize_t)total
    );

    if (result == NULL) {
        Py_DECREF(parts);
        return NULL;
    }

    {
        Py_ssize_t output_offset;
        Py_ssize_t index;
        Py_ssize_t count;

        output_offset = 0;
        count = PyList_GET_SIZE(parts);

        for (
            index = 0;
            index < count;
            index++
        ) {
            PyObject *piece;
            Py_ssize_t piece_size;
            char *destination;

            piece = PyList_GET_ITEM(
                parts,
                index
            );

            piece_size = PyBytes_GET_SIZE(
                piece
            );

            destination =
                PyBytes_AS_STRING(result)
                + output_offset;

            if (piece_size > 0) {
                Py_MEMCPY(
                    destination,
                    PyBytes_AS_STRING(piece),
                    piece_size
                );
            }

            output_offset += piece_size;
        }
    }

    Py_DECREF(parts);

    return result;
}

static PyMethodDef pyos_file_methods[] = {
    {
        "open",
        (PyCFunction)pyos_file_open,
        METH_VARARGS | METH_KEYWORDS,
        NULL
    },
    {
        "close",
        pyos_file_close,
        METH_VARARGS,
        NULL
    },
    {
        "read",
        (PyCFunction)pyos_file_read,
        METH_VARARGS | METH_KEYWORDS,
        NULL
    },
    {
        "write",
        pyos_file_write,
        METH_VARARGS,
        NULL
    },
    {
        "pread",
        pyos_file_pread,
        METH_VARARGS,
        NULL
    },
    {
        "pwrite",
        pyos_file_pwrite,
        METH_VARARGS,
        NULL
    },
    {
        "seek",
        pyos_file_seek,
        METH_VARARGS,
        NULL
    },
    {
        "tell",
        pyos_file_tell,
        METH_VARARGS,
        NULL
    },
    {
        "truncate",
        pyos_file_truncate,
        METH_VARARGS,
        NULL
    },
    {
        "fsync",
        pyos_file_fsync,
        METH_VARARGS,
        NULL
    },
    {
        "fstat",
        pyos_file_fstat,
        METH_VARARGS,
        NULL
    },
    {
        "dup",
        pyos_file_dup,
        METH_VARARGS,
        NULL
    },
    {
        "dup2",
        pyos_file_dup2,
        METH_VARARGS,
        NULL
    },
    {
        "pipe",
        pyos_file_pipe,
        METH_NOARGS,
        NULL
    },
    {
        "isatty",
        pyos_file_isatty,
        METH_VARARGS,
        NULL
    },
    {
        "readall",
        pyos_file_readall,
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

PyMethodDef *pyos_get_file_methods(void)
{
    return pyos_file_methods;
}
