#define PY_SSIZE_T_CLEAN

#include <Python.h>

#include "posix_internal.h"

static PyObject *
pyos_time_error(const char *operation)
{
    int error_code;
    const char *message;

    error_code = pyos_get_errno();
    message = NULL;

    if (pyos_native_strerror != NULL) {
        message = pyos_native_strerror(error_code);
    }

    if (message == NULL) {
        message = "native time operation failed";
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
pyos_time_clock_gettime(
    PyObject *self,
    PyObject *args
)
{
    int clock_id;
    pyos_timespec value;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "i:clock_gettime",
            &clock_id
        )
    ) {
        return NULL;
    }

    if (
        pyos_native_clock_gettime(
            clock_id,
            &value
        ) < 0
    ) {
        return pyos_time_error(
            "clock_gettime"
        );
    }

    return PyFloat_FromDouble(
        (double)value.tv_sec +
        ((double)value.tv_nsec / 1000000000.0)
    );
}

static PyObject *
pyos_time_clock_gettime_ns(
    PyObject *self,
    PyObject *args
)
{
    int clock_id;
    pyos_timespec value;
    long long result;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "i:clock_gettime_ns",
            &clock_id
        )
    ) {
        return NULL;
    }

    if (
        pyos_native_clock_gettime(
            clock_id,
            &value
        ) < 0
    ) {
        return pyos_time_error(
            "clock_gettime_ns"
        );
    }

    result =
        ((long long)value.tv_sec *
         1000000000LL) +
        (long long)value.tv_nsec;

    return PyLong_FromLongLong(
        result
    );
}

static PyObject *
pyos_time_time(
    PyObject *self,
    PyObject *args
)
{
    pyos_timespec value;

    (void)self;
    (void)args;

    if (
        pyos_native_realtime(
            &value
        ) < 0
    ) {
        return pyos_time_error(
            "time"
        );
    }

    return PyFloat_FromDouble(
        (double)value.tv_sec +
        ((double)value.tv_nsec / 1000000000.0)
    );
}

static PyObject *
pyos_time_time_ns(
    PyObject *self,
    PyObject *args
)
{
    pyos_timespec value;
    long long result;

    (void)self;
    (void)args;

    if (
        pyos_native_realtime(
            &value
        ) < 0
    ) {
        return pyos_time_error(
            "time_ns"
        );
    }

    result =
        ((long long)value.tv_sec *
         1000000000LL) +
        (long long)value.tv_nsec;

    return PyLong_FromLongLong(
        result
    );
}

static PyObject *
pyos_time_monotonic(
    PyObject *self,
    PyObject *args
)
{
    pyos_timespec value;

    (void)self;
    (void)args;

    if (
        pyos_native_monotonic(
            &value
        ) < 0
    ) {
        return pyos_time_error(
            "monotonic"
        );
    }

    return PyFloat_FromDouble(
        (double)value.tv_sec +
        ((double)value.tv_nsec / 1000000000.0)
    );
}

static PyObject *
pyos_time_monotonic_ns(
    PyObject *self,
    PyObject *args
)
{
    pyos_timespec value;
    long long result;

    (void)self;
    (void)args;

    if (
        pyos_native_monotonic(
            &value
        ) < 0
    ) {
        return pyos_time_error(
            "monotonic_ns"
        );
    }

    result =
        ((long long)value.tv_sec *
         1000000000LL) +
        (long long)value.tv_nsec;

    return PyLong_FromLongLong(
        result
    );
}

static PyObject *
pyos_time_process_time(
    PyObject *self,
    PyObject *args
)
{
    pyos_timespec value;

    (void)self;
    (void)args;

    if (
        pyos_native_process_time(
            &value
        ) < 0
    ) {
        return pyos_time_error(
            "process_time"
        );
    }

    return PyFloat_FromDouble(
        (double)value.tv_sec +
        ((double)value.tv_nsec / 1000000000.0)
    );
}

static PyObject *
pyos_time_process_time_ns(
    PyObject *self,
    PyObject *args
)
{
    pyos_timespec value;
    long long result;

    (void)self;
    (void)args;

    if (
        pyos_native_process_time(
            &value
        ) < 0
    ) {
        return pyos_time_error(
            "process_time_ns"
        );
    }

    result =
        ((long long)value.tv_sec *
         1000000000LL) +
        (long long)value.tv_nsec;

    return PyLong_FromLongLong(
        result
    );
}

static PyObject *
pyos_time_thread_time(
    PyObject *self,
    PyObject *args
)
{
    pyos_timespec value;

    (void)self;
    (void)args;

    if (
        pyos_native_thread_time(
            &value
        ) < 0
    ) {
        return pyos_time_error(
            "thread_time"
        );
    }

    return PyFloat_FromDouble(
        (double)value.tv_sec +
        ((double)value.tv_nsec / 1000000000.0)
    );
}

static PyObject *
pyos_time_thread_time_ns(
    PyObject *self,
    PyObject *args
)
{
    pyos_timespec value;
    long long result;

    (void)self;
    (void)args;

    if (
        pyos_native_thread_time(
            &value
        ) < 0
    ) {
        return pyos_time_error(
            "thread_time_ns"
        );
    }

    result =
        ((long long)value.tv_sec *
         1000000000LL) +
        (long long)value.tv_nsec;

    return PyLong_FromLongLong(
        result
    );
}

static PyObject *
pyos_time_sleep(
    PyObject *self,
    PyObject *args
)
{
    double seconds;
    long long whole_seconds;
    long nanoseconds;
    pyos_timespec request;
    pyos_timespec remaining;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "d:sleep",
            &seconds
        )
    ) {
        return NULL;
    }

    if (seconds < 0.0) {
        PyErr_SetString(
            PyExc_ValueError,
            "sleep length must be non-negative"
        );

        return NULL;
    }

    whole_seconds =
        (long long)seconds;

    nanoseconds =
        (long)(
            (seconds -
             (double)whole_seconds) *
            1000000000.0
        );

    if (nanoseconds < 0) {
        nanoseconds = 0;
    }

    if (nanoseconds >= 1000000000L) {
        whole_seconds++;
        nanoseconds -= 1000000000L;
    }

    request.tv_sec =
        (long)whole_seconds;

    request.tv_nsec =
        nanoseconds;

    remaining.tv_sec = 0;
    remaining.tv_nsec = 0;

    {
        int status;
        Py_BEGIN_ALLOW_THREADS
        status = pyos_native_nanosleep(
            &request,
            &remaining
        );
        Py_END_ALLOW_THREADS
        if (status < 0) {
            return pyos_time_error("sleep");
        }
    }

    Py_RETURN_NONE;
}

static PyObject *
pyos_time_nanosleep(
    PyObject *self,
    PyObject *args
)
{
    long long nanoseconds;
    pyos_timespec request;
    pyos_timespec remaining;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "L:nanosleep",
            &nanoseconds
        )
    ) {
        return NULL;
    }

    if (nanoseconds < 0) {
        PyErr_SetString(
            PyExc_ValueError,
            "sleep length must be non-negative"
        );

        return NULL;
    }

    request.tv_sec =
        (long)(
            nanoseconds /
            1000000000LL
        );

    request.tv_nsec =
        (long)(
            nanoseconds %
            1000000000LL
        );

    remaining.tv_sec = 0;
    remaining.tv_nsec = 0;

    {
        int status;
        Py_BEGIN_ALLOW_THREADS
        status = pyos_native_nanosleep(
            &request,
            &remaining
        );
        Py_END_ALLOW_THREADS
        if (status < 0) {
            return pyos_time_error("nanosleep");
        }
    }

    return PyLong_FromLongLong(
        ((long long)remaining.tv_sec *
         1000000000LL) +
        (long long)remaining.tv_nsec
    );
}

static PyObject *
pyos_time_clock_resolution(
    PyObject *self,
    PyObject *args
)
{
    int clock_id;
    pyos_timespec value;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "i:clock_getres",
            &clock_id
        )
    ) {
        return NULL;
    }

    if (
        pyos_native_clock_getres(
            clock_id,
            &value
        ) < 0
    ) {
        return pyos_time_error(
            "clock_getres"
        );
    }

    return PyFloat_FromDouble(
        (double)value.tv_sec +
        ((double)value.tv_nsec / 1000000000.0)
    );
}

static PyObject *
pyos_time_clock_resolution_ns(
    PyObject *self,
    PyObject *args
)
{
    int clock_id;
    pyos_timespec value;
    long long result;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "i:clock_getres_ns",
            &clock_id
        )
    ) {
        return NULL;
    }

    if (
        pyos_native_clock_getres(
            clock_id,
            &value
        ) < 0
    ) {
        return pyos_time_error(
            "clock_getres_ns"
        );
    }

    result =
        ((long long)value.tv_sec *
         1000000000LL) +
        (long long)value.tv_nsec;

    return PyLong_FromLongLong(
        result
    );
}

static PyMethodDef pyos_time_methods[] = {
    {
        "clock_gettime",
        pyos_time_clock_gettime,
        METH_VARARGS,
        NULL
    },
    {
        "clock_gettime_ns",
        pyos_time_clock_gettime_ns,
        METH_VARARGS,
        NULL
    },
    {
        "clock_getres",
        pyos_time_clock_resolution,
        METH_VARARGS,
        NULL
    },
    {
        "clock_getres_ns",
        pyos_time_clock_resolution_ns,
        METH_VARARGS,
        NULL
    },
    {
        "time",
        pyos_time_time,
        METH_NOARGS,
        NULL
    },
    {
        "time_ns",
        pyos_time_time_ns,
        METH_NOARGS,
        NULL
    },
    {
        "monotonic",
        pyos_time_monotonic,
        METH_NOARGS,
        NULL
    },
    {
        "monotonic_ns",
        pyos_time_monotonic_ns,
        METH_NOARGS,
        NULL
    },
    {
        "process_time",
        pyos_time_process_time,
        METH_NOARGS,
        NULL
    },
    {
        "process_time_ns",
        pyos_time_process_time_ns,
        METH_NOARGS,
        NULL
    },
    {
        "thread_time",
        pyos_time_thread_time,
        METH_NOARGS,
        NULL
    },
    {
        "thread_time_ns",
        pyos_time_thread_time_ns,
        METH_NOARGS,
        NULL
    },
    {
        "sleep",
        pyos_time_sleep,
        METH_VARARGS,
        NULL
    },
    {
        "nanosleep",
        pyos_time_nanosleep,
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
pyos_get_time_methods(void)
{
    return pyos_time_methods;
}
