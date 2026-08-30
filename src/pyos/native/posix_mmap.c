#define PY_SSIZE_T_CLEAN

#include <Python.h>

#include "posix_internal.h"

typedef long long pyos_mmap_intptr;

#define PYOS_PROT_NONE 0x0
#define PYOS_PROT_READ 0x1
#define PYOS_PROT_WRITE 0x2
#define PYOS_PROT_EXEC 0x4

#define PYOS_MAP_SHARED 0x01
#define PYOS_MAP_PRIVATE 0x02
#define PYOS_MAP_FIXED 0x10
#define PYOS_MAP_ANONYMOUS 0x20
#define PYOS_MAP_ANON PYOS_MAP_ANONYMOUS
#define PYOS_MAP_GROWSDOWN 0x0100
#define PYOS_MAP_DENYWRITE 0x0800
#define PYOS_MAP_EXECUTABLE 0x1000
#define PYOS_MAP_LOCKED 0x2000
#define PYOS_MAP_NORESERVE 0x4000
#define PYOS_MAP_POPULATE 0x8000
#define PYOS_MAP_NONBLOCK 0x10000
#define PYOS_MAP_STACK 0x20000
#define PYOS_MAP_HUGETLB 0x40000
#define PYOS_MAP_SYNC 0x80000
#define PYOS_MAP_FIXED_NOREPLACE 0x100000

#define PYOS_MADV_NORMAL 0
#define PYOS_MADV_RANDOM 1
#define PYOS_MADV_SEQUENTIAL 2
#define PYOS_MADV_WILLNEED 3
#define PYOS_MADV_DONTNEED 4
#define PYOS_MADV_FREE 8
#define PYOS_MADV_REMOVE 9
#define PYOS_MADV_DONTFORK 10
#define PYOS_MADV_DOFORK 11
#define PYOS_MADV_MERGEABLE 12
#define PYOS_MADV_UNMERGEABLE 13
#define PYOS_MADV_HUGEPAGE 14
#define PYOS_MADV_NOHUGEPAGE 15
#define PYOS_MADV_DONTDUMP 16
#define PYOS_MADV_DODUMP 17
#define PYOS_MADV_WIPEONFORK 18
#define PYOS_MADV_KEEPONFORK 19

#define PYOS_MS_ASYNC 1
#define PYOS_MS_INVALIDATE 2
#define PYOS_MS_SYNC 4

#define PYOS_MREMAP_MAYMOVE 1
#define PYOS_MREMAP_FIXED 2

typedef struct {
    void *address;
    unsigned long long length;
    int protection;
    int flags;
    int fd;
    long long offset;
    int active;
} pyos_mapping;

static PyObject *
pyos_mmap_error(
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
        message = "memory mapping operation failed";

    PyErr_Format(
        PyExc_OSError,
        "[%s] %s",
        operation,
        message
    );

    return NULL;
}

static int
pyos_mmap_require_size(
    PyObject *object,
    unsigned long long *value
)
{
    unsigned long long converted;

    if (!PyLong_Check(object)) {
        PyErr_SetString(
            PyExc_TypeError,
            "size must be an integer"
        );

        return -1;
    }

    converted = PyLong_AsUnsignedLongLong(object);

    if (
        converted ==
        (unsigned long long)-1 &&
        PyErr_Occurred()
    )
        return -1;

    *value = converted;

    return 0;
}

static PyObject *
pyos_mmap_map(
    PyObject *self,
    PyObject *args,
    PyObject *kwargs
)
{
    unsigned long long length;
    int protection;
    int flags;
    int fd;
    long long offset;
    void *address;

    static char *keywords[] = {
        "length",
        "protection",
        "flags",
        "fd",
        "offset",
        NULL
    };

    (void)self;

    protection = PYOS_PROT_READ | PYOS_PROT_WRITE;
    flags = PYOS_MAP_PRIVATE | PYOS_MAP_ANONYMOUS;
    fd = -1;
    offset = 0;

    if (
        !PyArg_ParseTupleAndKeywords(
            args,
            kwargs,
            "K|iiLi:mmap",
            keywords,
            &length,
            &protection,
            &flags,
            &fd,
            &offset
        )
    )
        return NULL;

    if (length == 0) {
        PyErr_SetString(
            PyExc_ValueError,
            "mmap length must be greater than zero"
        );

        return NULL;
    }

    address = pyos_native_mmap(
        NULL,
        length,
        protection,
        flags,
        fd,
        offset
    );

    if (address == (void *)-1)
        return pyos_mmap_error("mmap");

    return PyLong_FromUnsignedLongLong(
        (unsigned long long)(
            pyos_mmap_intptr)address
    );
}

static PyObject *
pyos_mmap_map_at(
    PyObject *self,
    PyObject *args,
    PyObject *kwargs
)
{
    unsigned long long address_value;
    unsigned long long length;
    int protection;
    int flags;
    int fd;
    long long offset;
    void *address;
    void *result;

    static char *keywords[] = {
        "address",
        "length",
        "protection",
        "flags",
        "fd",
        "offset",
        NULL
    };

    (void)self;

    protection = PYOS_PROT_READ | PYOS_PROT_WRITE;
    flags = PYOS_MAP_PRIVATE | PYOS_MAP_ANONYMOUS;
    fd = -1;
    offset = 0;

    if (
        !PyArg_ParseTupleAndKeywords(
            args,
            kwargs,
            "KK|iiLi:mmap_at",
            keywords,
            &address_value,
            &length,
            &protection,
            &flags,
            &fd,
            &offset
        )
    )
        return NULL;

    if (length == 0) {
        PyErr_SetString(
            PyExc_ValueError,
            "mmap length must be greater than zero"
        );

        return NULL;
    }

    address = (void *)(pyos_mmap_intptr)address_value;

    result = pyos_native_mmap(
        address,
        length,
        protection,
        flags,
        fd,
        offset
    );

    if (result == (void *)-1)
        return pyos_mmap_error("mmap");

    return PyLong_FromUnsignedLongLong(
        (unsigned long long)(
            (pyos_mmap_intptr)result
        )
    );
}

static PyObject *
pyos_mmap_unmap(
    PyObject *self,
    PyObject *args
)
{
    unsigned long long address_value;
    unsigned long long length;
    void *address;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "KK:munmap",
            &address_value,
            &length
        )
    )
        return NULL;

    if (length == 0) {
        PyErr_SetString(
            PyExc_ValueError,
            "munmap length must be greater than zero"
        );

        return NULL;
    }

    address = (void *)(pyos_mmap_intptr)address_value;

    if (
        pyos_native_munmap(
            address,
            length
        ) < 0
    )
        return pyos_mmap_error("munmap");

    Py_RETURN_NONE;
}

static PyObject *
pyos_mmap_protect(
    PyObject *self,
    PyObject *args
)
{
    unsigned long long address_value;
    unsigned long long length;
    int protection;
    void *address;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "KKi:mprotect",
            &address_value,
            &length,
            &protection
        )
    )
        return NULL;

    if (length == 0) {
        PyErr_SetString(
            PyExc_ValueError,
            "mprotect length must be greater than zero"
        );

        return NULL;
    }

    address = (void *)(pyos_mmap_intptr)address_value;

    if (
        pyos_native_mprotect(
            address,
            length,
            protection
        ) < 0
    )
        return pyos_mmap_error("mprotect");

    Py_RETURN_NONE;
}

static PyObject *
pyos_mmap_advise(
    PyObject *self,
    PyObject *args
)
{
    unsigned long long address_value;
    unsigned long long length;
    int advice;
    void *address;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "KKi:madvise",
            &address_value,
            &length,
            &advice
        )
    )
        return NULL;

    if (length == 0) {
        PyErr_SetString(
            PyExc_ValueError,
            "madvise length must be greater than zero"
        );

        return NULL;
    }

    address = (void *)(pyos_mmap_intptr)address_value;

    if (
        pyos_native_madvise(
            address,
            length,
            advice
        ) < 0
    )
        return pyos_mmap_error("madvise");

    Py_RETURN_NONE;
}

static PyObject *
pyos_mmap_sync(
    PyObject *self,
    PyObject *args
)
{
    unsigned long long address_value;
    unsigned long long length;
    int flags;
    void *address;

    (void)self;

    flags = PYOS_MS_ASYNC;

    if (
        !PyArg_ParseTuple(
            args,
            "KK|i:msync",
            &address_value,
            &length,
            &flags
        )
    )
        return NULL;

    if (length == 0) {
        PyErr_SetString(
            PyExc_ValueError,
            "msync length must be greater than zero"
        );

        return NULL;
    }

    address = (void *)(pyos_mmap_intptr)address_value;

    if (
        pyos_native_msync(
            address,
            length,
            flags
        ) < 0
    )
        return pyos_mmap_error("msync");

    Py_RETURN_NONE;
}

static PyObject *
pyos_mmap_lock(
    PyObject *self,
    PyObject *args
)
{
    unsigned long long address_value;
    unsigned long long length;
    void *address;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "KK:mlock",
            &address_value,
            &length
        )
    )
        return NULL;

    address = (void *)(pyos_mmap_intptr)address_value;

    if (
        pyos_native_mlock(
            address,
            length
        ) < 0
    )
        return pyos_mmap_error("mlock");

    Py_RETURN_NONE;
}

static PyObject *
pyos_mmap_unlock(
    PyObject *self,
    PyObject *args
)
{
    unsigned long long address_value;
    unsigned long long length;
    void *address;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "KK:munlock",
            &address_value,
            &length
        )
    )
        return NULL;

    address = (void *)(pyos_mmap_intptr)address_value;

    if (
        pyos_native_munlock(
            address,
            length
        ) < 0
    )
        return pyos_mmap_error("munlock");

    Py_RETURN_NONE;
}

static PyObject *
pyos_mmap_constants(
    PyObject *self,
    PyObject *args
)
{
    PyObject *dict;

    (void)self;
    (void)args;

    dict = PyDict_New();

    if (dict == NULL)
        return NULL;

    if (
        PyDict_SetItemString(
            dict,
            "PROT_NONE",
            PyLong_FromLong(PYOS_PROT_NONE)
        ) < 0
    )
        goto failure;

    if (
        PyDict_SetItemString(
            dict,
            "PROT_READ",
            PyLong_FromLong(PYOS_PROT_READ)
        ) < 0
    )
        goto failure;

    if (
        PyDict_SetItemString(
            dict,
            "PROT_WRITE",
            PyLong_FromLong(PYOS_PROT_WRITE)
        ) < 0
    )
        goto failure;

    if (
        PyDict_SetItemString(
            dict,
            "PROT_EXEC",
            PyLong_FromLong(PYOS_PROT_EXEC)
        ) < 0
    )
        goto failure;

    if (
        PyDict_SetItemString(
            dict,
            "MAP_SHARED",
            PyLong_FromLong(PYOS_MAP_SHARED)
        ) < 0
    )
        goto failure;

    if (
        PyDict_SetItemString(
            dict,
            "MAP_PRIVATE",
            PyLong_FromLong(PYOS_MAP_PRIVATE)
        ) < 0
    )
        goto failure;

    if (
        PyDict_SetItemString(
            dict,
            "MAP_FIXED",
            PyLong_FromLong(PYOS_MAP_FIXED)
        ) < 0
    )
        goto failure;

    if (
        PyDict_SetItemString(
            dict,
            "MAP_ANONYMOUS",
            PyLong_FromLong(PYOS_MAP_ANONYMOUS)
        ) < 0
    )
        goto failure;

    if (
        PyDict_SetItemString(
            dict,
            "MAP_FIXED_NOREPLACE",
            PyLong_FromLong(PYOS_MAP_FIXED_NOREPLACE)
        ) < 0
    )
        goto failure;

    return dict;

failure:
    Py_DECREF(dict);
    return NULL;
}

static PyMethodDef pyos_mmap_methods[] = {
    {
        "mmap",
        (PyCFunction)pyos_mmap_map,
        METH_VARARGS | METH_KEYWORDS,
        NULL
    },
    {
        "mmap_at",
        (PyCFunction)pyos_mmap_map_at,
        METH_VARARGS | METH_KEYWORDS,
        NULL
    },
    {
        "munmap",
        pyos_mmap_unmap,
        METH_VARARGS,
        NULL
    },
    {
        "mprotect",
        pyos_mmap_protect,
        METH_VARARGS,
        NULL
    },
    {
        "madvise",
        pyos_mmap_advise,
        METH_VARARGS,
        NULL
    },
    {
        "msync",
        pyos_mmap_sync,
        METH_VARARGS,
        NULL
    },
    {
        "mlock",
        pyos_mmap_lock,
        METH_VARARGS,
        NULL
    },
    {
        "munlock",
        pyos_mmap_unlock,
        METH_VARARGS,
        NULL
    },
    {
        "constants",
        pyos_mmap_constants,
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
pyos_get_mmap_methods(void)
{
    return pyos_mmap_methods;
}
