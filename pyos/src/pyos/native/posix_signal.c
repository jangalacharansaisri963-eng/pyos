#define PY_SSIZE_T_CLEAN

#include <Python.h>

#include "posix_internal.h"

static PyObject *
pyos_signal_error(
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
        message = "native signal operation failed";
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
pyos_signal_send(
    PyObject *self,
    PyObject *args
)
{
    long long pid;
    int signal_number;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "Li:kill",
            &pid,
            &signal_number
        )
    ) {
        return NULL;
    }

    if (
        pyos_native_kill(
            pid,
            signal_number
        ) < 0
    ) {
        return pyos_signal_error(
            "kill"
        );
    }

    Py_RETURN_NONE;
}

static PyObject *
pyos_signal_raise(
    PyObject *self,
    PyObject *args
)
{
    int signal_number;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "i:raise_signal",
            &signal_number
        )
    ) {
        return NULL;
    }

    if (
        pyos_native_raise(
            signal_number
        ) < 0
    ) {
        return pyos_signal_error(
            "raise"
        );
    }

    Py_RETURN_NONE;
}

static PyObject *
pyos_signal_getmask(
    PyObject *self,
    PyObject *args
)
{
    unsigned long long mask;

    (void)self;
    (void)args;

    if (
        pyos_native_get_signal_mask(
            &mask
        ) < 0
    ) {
        return pyos_signal_error(
            "sigprocmask"
        );
    }

    return PyLong_FromUnsignedLongLong(
        mask
    );
}

static PyObject *
pyos_signal_block(
    PyObject *self,
    PyObject *args
)
{
    unsigned long long mask;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "K:block",
            &mask
        )
    ) {
        return NULL;
    }

    if (
        pyos_native_block_signals(
            mask
        ) < 0
    ) {
        return pyos_signal_error(
            "sigprocmask(SIG_BLOCK)"
        );
    }

    Py_RETURN_NONE;
}

static PyObject *
pyos_signal_unblock(
    PyObject *self,
    PyObject *args
)
{
    unsigned long long mask;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "K:unblock",
            &mask
        )
    ) {
        return NULL;
    }

    if (
        pyos_native_unblock_signals(
            mask
        ) < 0
    ) {
        return pyos_signal_error(
            "sigprocmask(SIG_UNBLOCK)"
        );
    }

    Py_RETURN_NONE;
}

static PyObject *
pyos_signal_setmask(
    PyObject *self,
    PyObject *args
)
{
    unsigned long long mask;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "K:setmask",
            &mask
        )
    ) {
        return NULL;
    }

    if (
        pyos_native_set_signal_mask(
            mask
        ) < 0
    ) {
        return pyos_signal_error(
            "sigprocmask(SIG_SETMASK)"
        );
    }

    Py_RETURN_NONE;
}

static PyObject *
pyos_signal_pending(
    PyObject *self,
    PyObject *args
)
{
    unsigned long long mask;

    (void)self;
    (void)args;

    if (
        pyos_native_pending_signals(
            &mask
        ) < 0
    ) {
        return pyos_signal_error(
            "sigpending"
        );
    }

    return PyLong_FromUnsignedLongLong(
        mask
    );
}

static PyObject *
pyos_signal_wait(
    PyObject *self,
    PyObject *args
)
{
    unsigned long long mask;
    int signal_number;
    int result;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "K:wait_signal",
            &mask
        )
    ) {
        return NULL;
    }

    Py_BEGIN_ALLOW_THREADS

    result =
        pyos_native_wait_signal(
            mask,
            &signal_number
        );

    Py_END_ALLOW_THREADS

    if (result < 0) {
        return pyos_signal_error(
            "sigsuspend/sigwait"
        );
    }

    return PyLong_FromLong(
        signal_number
    );
}

static PyObject *
pyos_signal_wait_timeout(
    PyObject *self,
    PyObject *args
)
{
    unsigned long long mask;
    long long seconds;
    long nanoseconds;
    int signal_number;
    int result;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "KLl:wait_signal_timeout",
            &mask,
            &seconds,
            &nanoseconds
        )
    ) {
        return NULL;
    }

    if (
        seconds < 0 ||
        nanoseconds < 0 ||
        nanoseconds >= 1000000000L
    ) {
        PyErr_SetString(
            PyExc_ValueError,
            "invalid timeout"
        );

        return NULL;
    }

    Py_BEGIN_ALLOW_THREADS

    result =
        pyos_native_wait_signal_timeout(
            mask,
            seconds,
            nanoseconds,
            &signal_number
        );

    Py_END_ALLOW_THREADS

    if (result < 0) {
        return pyos_signal_error(
            "sigtimedwait"
        );
    }

    if (result == 0) {
        Py_RETURN_NONE;
    }

    return PyLong_FromLong(
        signal_number
    );
}

static PyObject *
pyos_signal_queue(
    PyObject *self,
    PyObject *args
)
{
    long long pid;
    int signal_number;
    int value;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "Li|i:queue_signal",
            &pid,
            &signal_number,
            &value
        )
    ) {
        return NULL;
    }

    if (
        pyos_native_queue_signal(
            pid,
            signal_number,
            value
        ) < 0
    ) {
        return pyos_signal_error(
            "sigqueue"
        );
    }

    Py_RETURN_NONE;
}

static PyObject *
pyos_signal_alarm(
    PyObject *self,
    PyObject *args
)
{
    unsigned long seconds;
    unsigned long previous;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "k:alarm",
            &seconds
        )
    ) {
        return NULL;
    }

    previous =
        pyos_native_alarm(
            seconds
        );

    return PyLong_FromUnsignedLong(
        previous
    );
}

static PyObject *
pyos_signal_getitimer(
    PyObject *self,
    PyObject *args
)
{
    int timer;
    long long interval_seconds;
    long interval_microseconds;
    long long value_seconds;
    long value_microseconds;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "i:getitimer",
            &timer
        )
    ) {
        return NULL;
    }

    if (
        pyos_native_getitimer(
            timer,
            &interval_seconds,
            &interval_microseconds,
            &value_seconds,
            &value_microseconds
        ) < 0
    ) {
        return pyos_signal_error(
            "getitimer"
        );
    }

    return Py_BuildValue(
        "((Ll)(Ll))",
        interval_seconds,
        interval_microseconds,
        value_seconds,
        value_microseconds
    );
}

static PyObject *
pyos_signal_setitimer(
    PyObject *self,
    PyObject *args
)
{
    int timer;
    double value;
    double interval;
    double previous_value;
    double previous_interval;

    (void)self;

    interval = 0.0;

    if (
        !PyArg_ParseTuple(
            args,
            "id|d:setitimer",
            &timer,
            &value,
            &interval
        )
    ) {
        return NULL;
    }

    if (
        value < 0.0 ||
        interval < 0.0
    ) {
        PyErr_SetString(
            PyExc_ValueError,
            "timer values cannot be negative"
        );

        return NULL;
    }

    if (
        pyos_native_setitimer(
            timer,
            value,
            interval,
            &previous_value,
            &previous_interval
        ) < 0
    ) {
        return pyos_signal_error(
            "setitimer"
        );
    }

    return Py_BuildValue(
        "(dd)",
        previous_value,
        previous_interval
    );
}

static PyObject *
pyos_signal_getpgrp(
    PyObject *self,
    PyObject *args
)
{
    long long pgid;

    (void)self;
    (void)args;

    pgid = pyos_native_getpgrp();

    if (pgid < 0) {
        return pyos_signal_error(
            "getpgrp"
        );
    }

    return PyLong_FromLongLong(
        pgid
    );
}

static PyObject *
pyos_signal_send_group(
    PyObject *self,
    PyObject *args
)
{
    long long pgid;
    int signal_number;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "Li:killpg",
            &pgid,
            &signal_number
        )
    ) {
        return NULL;
    }

    if (
        pyos_native_killpg(
            pgid,
            signal_number
        ) < 0
    ) {
        return pyos_signal_error(
            "killpg"
        );
    }

    Py_RETURN_NONE;
}

static PyObject *
pyos_signal_set_handler(
    PyObject *self,
    PyObject *args
)
{
    int signal_number;
    PyObject *handler;
    PyObject *previous;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "iO:set_handler",
            &signal_number,
            &handler
        )
    ) {
        return NULL;
    }

    previous =
        pyos_native_signal_set_handler(
            signal_number,
            handler
        );

    if (previous == NULL) {
        return NULL;
    }

    return previous;
}

static PyObject *
pyos_signal_ignore(
    PyObject *self,
    PyObject *args
)
{
    int signal_number;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "i:ignore",
            &signal_number
        )
    ) {
        return NULL;
    }

    if (
        pyos_native_signal_ignore(
            signal_number
        ) < 0
    ) {
        return pyos_signal_error(
            "signal(SIG_IGN)"
        );
    }

    Py_RETURN_NONE;
}

static PyObject *
pyos_signal_default(
    PyObject *self,
    PyObject *args
)
{
    int signal_number;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "i:default",
            &signal_number
        )
    ) {
        return NULL;
    }

    if (
        pyos_native_signal_default(
            signal_number
        ) < 0
    ) {
        return pyos_signal_error(
            "signal(SIG_DFL)"
        );
    }

    Py_RETURN_NONE;
}

static PyObject *
pyos_signal_sigaction(
    PyObject *self,
    PyObject *args
)
{
    int signal_number;
    PyObject *handler;
    PyObject *previous;
    unsigned long long flags;
    unsigned long long mask;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "iOOK:sigaction",
            &signal_number,
            &handler,
            &flags,
            &mask
        )
    ) {
        return NULL;
    }

    previous =
        pyos_native_sigaction(
            signal_number,
            handler,
            flags,
            mask
        );

    if (previous == NULL) {
        return NULL;
    }

    return previous;
}

static PyMethodDef pyos_signal_methods[] = {
    {
        "kill",
        pyos_signal_send,
        METH_VARARGS,
        NULL
    },
    {
        "raise_signal",
        pyos_signal_raise,
        METH_VARARGS,
        NULL
    },
    {
        "getmask",
        pyos_signal_getmask,
        METH_NOARGS,
        NULL
    },
    {
        "block",
        pyos_signal_block,
        METH_VARARGS,
        NULL
    },
    {
        "unblock",
        pyos_signal_unblock,
        METH_VARARGS,
        NULL
    },
    {
        "setmask",
        pyos_signal_setmask,
        METH_VARARGS,
        NULL
    },
    {
        "pending",
        pyos_signal_pending,
        METH_NOARGS,
        NULL
    },
    {
        "wait_signal",
        pyos_signal_wait,
        METH_VARARGS,
        NULL
    },
    {
        "wait_signal_timeout",
        pyos_signal_wait_timeout,
        METH_VARARGS,
        NULL
    },
    {
        "queue_signal",
        pyos_signal_queue,
        METH_VARARGS,
        NULL
    },
    {
        "alarm",
        pyos_signal_alarm,
        METH_VARARGS,
        NULL
    },
    {
        "getitimer",
        pyos_signal_getitimer,
        METH_VARARGS,
        NULL
    },
    {
        "setitimer",
        pyos_signal_setitimer,
        METH_VARARGS,
        NULL
    },
    {
        "getpgrp",
        pyos_signal_getpgrp,
        METH_NOARGS,
        NULL
    },
    {
        "killpg",
        pyos_signal_send_group,
        METH_VARARGS,
        NULL
    },
    {
        "set_handler",
        pyos_signal_set_handler,
        METH_VARARGS,
        NULL
    },
    {
        "ignore",
        pyos_signal_ignore,
        METH_VARARGS,
        NULL
    },
    {
        "default",
        pyos_signal_default,
        METH_VARARGS,
        NULL
    },
    {
        "sigaction",
        pyos_signal_sigaction,
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
pyos_get_signal_methods(void)
{
    return pyos_signal_methods;
}
