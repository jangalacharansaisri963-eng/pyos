#define PY_SSIZE_T_CLEAN

#include <Python.h>

#include "posix_internal.h"

static PyObject *
pyos_poll_error(
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
        message = "poll operation failed";

    PyErr_Format(
        PyExc_OSError,
        "[%s] %s",
        operation,
        message
    );

    return NULL;
}

static PyObject *
pyos_poll_poll(
    PyObject *self,
    PyObject *args
)
{
    PyObject *sequence;
    int timeout;
    Py_ssize_t count;
    Py_ssize_t index;
    pyos_poll_result *items;
    PyObject *result;

    (void)self;

    timeout = -1;

    if (
        !PyArg_ParseTuple(
            args,
            "O|i:poll",
            &sequence,
            &timeout
        )
    )
        return NULL;

    sequence = PySequence_Fast(
        sequence,
        "poll descriptors must be a sequence"
    );

    if (sequence == NULL)
        return NULL;

    count = PySequence_Fast_GET_SIZE(
        sequence
    );

    if (count == 0) {
        Py_DECREF(sequence);

        if (
            pyos_native_poll(
                NULL,
                0,
                timeout
            ) < 0
        )
            return pyos_poll_error("poll");

        return PyList_New(0);
    }

    items = PyMem_Calloc(
        (size_t)count,
        sizeof(pyos_poll_result)
    );

    if (items == NULL) {
        Py_DECREF(sequence);
        return PyErr_NoMemory();
    }

    for (
        index = 0;
        index < count;
        index++
    ) {
        PyObject *item;
        PyObject *fd_object;
        PyObject *events_object;
        long long fd;
        long long events;

        item = PySequence_Fast_GET_ITEM(
            sequence,
            index
        );

        if (
            !PyTuple_Check(item) &&
            !PyList_Check(item)
        ) {
            PyErr_SetString(
                PyExc_TypeError,
                "each poll item must be a "
                "(fd, events) pair"
            );

            PyMem_Free(items);
            Py_DECREF(sequence);

            return NULL;
        }

        if (
            PySequence_Size(item) != 2
        ) {
            PyErr_SetString(
                PyExc_ValueError,
                "each poll item must contain "
                "exactly two values"
            );

            PyMem_Free(items);
            Py_DECREF(sequence);

            return NULL;
        }

        fd_object = PySequence_GetItem(
            item,
            0
        );

        events_object = PySequence_GetItem(
            item,
            1
        );

        if (
            fd_object == NULL ||
            events_object == NULL
        ) {
            Py_XDECREF(fd_object);
            Py_XDECREF(events_object);

            PyMem_Free(items);
            Py_DECREF(sequence);

            return NULL;
        }

        fd = PyLong_AsLongLong(
            fd_object
        );

        events = PyLong_AsLongLong(
            events_object
        );

        Py_DECREF(fd_object);
        Py_DECREF(events_object);

        if (
            PyErr_Occurred()
        ) {
            PyMem_Free(items);
            Py_DECREF(sequence);

            return NULL;
        }

        if (
            fd < 0 ||
            fd > 2147483647LL
        ) {
            PyErr_SetString(
                PyExc_ValueError,
                "invalid file descriptor"
            );

            PyMem_Free(items);
            Py_DECREF(sequence);

            return NULL;
        }

        items[index].fd = (int)fd;
        items[index].events = (short)events;
        items[index].revents = 0;
    }

    Py_DECREF(sequence);

    if (
        pyos_native_poll(
            items,
            count,
            timeout
        ) < 0
    ) {
        PyMem_Free(items);

        return pyos_poll_error(
            "poll"
        );
    }

    result = PyList_New(0);

    if (result == NULL) {
        PyMem_Free(items);
        return NULL;
    }

    for (
        index = 0;
        index < count;
        index++
    ) {
        PyObject *entry;

        if (
            items[index].revents == 0
        )
            continue;

        entry = Py_BuildValue(
            "(ih)",
            items[index].fd,
            items[index].revents
        );

        if (entry == NULL) {
            Py_DECREF(result);
            PyMem_Free(items);

            return NULL;
        }

        if (
            PyList_Append(
                result,
                entry
            ) < 0
        ) {
            Py_DECREF(entry);
            Py_DECREF(result);
            PyMem_Free(items);

            return NULL;
        }

        Py_DECREF(entry);
    }

    PyMem_Free(items);

    return result;
}

static PyObject *
pyos_poll_select(
    PyObject *self,
    PyObject *args,
    PyObject *kwargs
)
{
    PyObject *read_objects;
    PyObject *write_objects;
    PyObject *error_objects;
    double timeout;
    static char *keywords[] = {
        "read",
        "write",
        "error",
        "timeout",
        NULL
    };

    PyObject *result;

    (void)self;

    read_objects = Py_None;
    write_objects = Py_None;
    error_objects = Py_None;
    timeout = -1.0;

    if (
        !PyArg_ParseTupleAndKeywords(
            args,
            kwargs,
            "OOO|d:select",
            keywords,
            &read_objects,
            &write_objects,
            &error_objects,
            &timeout
        )
    )
        return NULL;

    if (timeout < -1.0) {
        PyErr_SetString(
            PyExc_ValueError,
            "timeout must be >= -1"
        );

        return NULL;
    }

    result = pyos_native_select(
        read_objects,
        write_objects,
        error_objects,
        timeout
    );

    if (result == NULL)
        return NULL;

    return result;
}

static PyObject *
pyos_poll_epoll_create(
    PyObject *self,
    PyObject *args
)
{
    int flags;
    int fd;

    (void)self;

    flags = 0;

    if (
        !PyArg_ParseTuple(
            args,
            "|i:epoll_create",
            &flags
        )
    )
        return NULL;

    fd = pyos_native_epoll_create(
        flags
    );

    if (fd < 0)
        return pyos_poll_error(
            "epoll_create"
        );

    return PyLong_FromLong(fd);
}

static PyObject *
pyos_poll_epoll_ctl(
    PyObject *self,
    PyObject *args
)
{
    int epfd;
    int operation;
    int fd;
    unsigned int events;
    unsigned long long data;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "iiiIK:epoll_ctl",
            &epfd,
            &operation,
            &fd,
            &events,
            &data
        )
    )
        return NULL;

    if (
        pyos_native_epoll_ctl(
            epfd,
            operation,
            fd,
            events,
            data
        ) < 0
    )
        return pyos_poll_error(
            "epoll_ctl"
        );

    Py_RETURN_NONE;
}

static PyObject *
pyos_poll_epoll_wait(
    PyObject *self,
    PyObject *args
)
{
    int epfd;
    int maxevents;
    int timeout;
    pyos_epoll_event *events;
    int count;
    int index;
    PyObject *result;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "ii|i:epoll_wait",
            &epfd,
            &maxevents,
            &timeout
        )
    )
        return NULL;

    if (maxevents <= 0) {
        PyErr_SetString(
            PyExc_ValueError,
            "maxevents must be positive"
        );

        return NULL;
    }

    events = PyMem_Calloc(
        (size_t)maxevents,
        sizeof(pyos_epoll_event)
    );

    if (events == NULL)
        return PyErr_NoMemory();

    count = pyos_native_epoll_wait(
        epfd,
        events,
        maxevents,
        timeout
    );

    if (count < 0) {
        PyMem_Free(events);

        return pyos_poll_error(
            "epoll_wait"
        );
    }

    result = PyList_New(count);

    if (result == NULL) {
        PyMem_Free(events);
        return NULL;
    }

    for (
        index = 0;
        index < count;
        index++
    ) {
        PyObject *entry;

        entry = Py_BuildValue(
            "(IK)",
            events[index].events,
            events[index].data
        );

        if (entry == NULL) {
            Py_DECREF(result);
            PyMem_Free(events);

            return NULL;
        }

        PyList_SET_ITEM(
            result,
            index,
            entry
        );
    }

    PyMem_Free(events);

    return result;
}

static PyMethodDef pyos_poll_methods[] = {
    {
        "poll",
        pyos_poll_poll,
        METH_VARARGS,
        NULL
    },
    {
        "select",
        (PyCFunction)pyos_poll_select,
        METH_VARARGS | METH_KEYWORDS,
        NULL
    },
    {
        "epoll_create",
        pyos_poll_epoll_create,
        METH_VARARGS,
        NULL
    },
    {
        "epoll_ctl",
        pyos_poll_epoll_ctl,
        METH_VARARGS,
        NULL
    },
    {
        "epoll_wait",
        pyos_poll_epoll_wait,
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
pyos_get_poll_methods(void)
{
    return pyos_poll_methods;
}
