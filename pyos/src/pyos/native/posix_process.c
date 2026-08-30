#define PY_SSIZE_T_CLEAN

#include <Python.h>

#include "posix_internal.h"

static PyObject *
pyos_process_error(
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
        message = "native process operation failed";
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
pyos_process_getpid(
    PyObject *self,
    PyObject *args
)
{
    long long pid;

    (void)self;
    (void)args;

    pid = pyos_native_getpid();

    if (pid < 0) {
        return pyos_process_error("getpid");
    }

    return PyLong_FromLongLong(pid);
}

static PyObject *
pyos_process_getppid(
    PyObject *self,
    PyObject *args
)
{
    long long pid;

    (void)self;
    (void)args;

    pid = pyos_native_getppid();

    if (pid < 0) {
        return pyos_process_error("getppid");
    }

    return PyLong_FromLongLong(pid);
}

static PyObject *
pyos_process_getuid(
    PyObject *self,
    PyObject *args
)
{
    unsigned long uid;

    (void)self;
    (void)args;

    uid = pyos_native_getuid();

    return PyLong_FromUnsignedLong(uid);
}

static PyObject *
pyos_process_geteuid(
    PyObject *self,
    PyObject *args
)
{
    unsigned long uid;

    (void)self;
    (void)args;

    uid = pyos_native_geteuid();

    return PyLong_FromUnsignedLong(uid);
}

static PyObject *
pyos_process_getgid(
    PyObject *self,
    PyObject *args
)
{
    unsigned long gid;

    (void)self;
    (void)args;

    gid = pyos_native_getgid();

    return PyLong_FromUnsignedLong(gid);
}

static PyObject *
pyos_process_getegid(
    PyObject *self,
    PyObject *args
)
{
    unsigned long gid;

    (void)self;
    (void)args;

    gid = pyos_native_getegid();

    return PyLong_FromUnsignedLong(gid);
}

static PyObject *
pyos_process_getgroups(
    PyObject *self,
    PyObject *args
)
{
    unsigned long *groups;
    long count;
    PyObject *result;
    long index;

    (void)self;
    (void)args;

    count = pyos_native_getgroups(
        0,
        NULL
    );

    if (count < 0) {
        return pyos_process_error(
            "getgroups"
        );
    }

    if (count == 0) {
        return PyList_New(0);
    }

    groups = PyMem_Malloc(
        sizeof(unsigned long) *
        (size_t)count
    );

    if (groups == NULL) {
        return PyErr_NoMemory();
    }

    count = pyos_native_getgroups(
        count,
        groups
    );

    if (count < 0) {
        PyMem_Free(groups);

        return pyos_process_error(
            "getgroups"
        );
    }

    result = PyList_New(count);

    if (result == NULL) {
        PyMem_Free(groups);
        return NULL;
    }

    for (
        index = 0;
        index < count;
        index++
    ) {
        PyObject *value;

        value = PyLong_FromUnsignedLong(
            groups[index]
        );

        if (value == NULL) {
            Py_DECREF(result);
            PyMem_Free(groups);
            return NULL;
        }

        PyList_SET_ITEM(
            result,
            index,
            value
        );
    }

    PyMem_Free(groups);

    return result;
}

static PyObject *
pyos_process_setuid(
    PyObject *self,
    PyObject *args
)
{
    unsigned long uid;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "k:setuid",
            &uid
        )
    ) {
        return NULL;
    }

    if (
        pyos_native_setuid(uid) < 0
    ) {
        return pyos_process_error(
            "setuid"
        );
    }

    Py_RETURN_NONE;
}

static PyObject *
pyos_process_setgid(
    PyObject *self,
    PyObject *args
)
{
    unsigned long gid;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "k:setgid",
            &gid
        )
    ) {
        return NULL;
    }

    if (
        pyos_native_setgid(gid) < 0
    ) {
        return pyos_process_error(
            "setgid"
        );
    }

    Py_RETURN_NONE;
}

static PyObject *
pyos_process_setpgid(
    PyObject *self,
    PyObject *args
)
{
    long long pid;
    long long pgid;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "LL:setpgid",
            &pid,
            &pgid
        )
    ) {
        return NULL;
    }

    if (
        pyos_native_setpgid(
            pid,
            pgid
        ) < 0
    ) {
        return pyos_process_error(
            "setpgid"
        );
    }

    Py_RETURN_NONE;
}

static PyObject *
pyos_process_getpgid(
    PyObject *self,
    PyObject *args
)
{
    long long pid;
    long long pgid;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "L:getpgid",
            &pid
        )
    ) {
        return NULL;
    }

    pgid = pyos_native_getpgid(pid);

    if (pgid < 0) {
        return pyos_process_error(
            "getpgid"
        );
    }

    return PyLong_FromLongLong(pgid);
}

static PyObject *
pyos_process_getpgrp(
    PyObject *self,
    PyObject *args
)
{
    long long pgid;

    (void)self;
    (void)args;

    pgid = pyos_native_getpgrp();

    if (pgid < 0) {
        return pyos_process_error(
            "getpgrp"
        );
    }

    return PyLong_FromLongLong(pgid);
}

static PyObject *
pyos_process_setsid(
    PyObject *self,
    PyObject *args
)
{
    long long sid;

    (void)self;
    (void)args;

    sid = pyos_native_setsid();

    if (sid < 0) {
        return pyos_process_error(
            "setsid"
        );
    }

    return PyLong_FromLongLong(sid);
}

static PyObject *
pyos_process_getsid(
    PyObject *self,
    PyObject *args
)
{
    long long pid;
    long long sid;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "L:getsid",
            &pid
        )
    ) {
        return NULL;
    }

    sid = pyos_native_getsid(pid);

    if (sid < 0) {
        return pyos_process_error(
            "getsid"
        );
    }

    return PyLong_FromLongLong(sid);
}

static PyObject *
pyos_process_fork(
    PyObject *self,
    PyObject *args
)
{
    long long pid;

    (void)self;
    (void)args;

    pid = pyos_native_fork();

    if (pid < 0) {
        return pyos_process_error(
            "fork"
        );
    }

    return PyLong_FromLongLong(pid);
}

static PyObject *
pyos_process_vfork(
    PyObject *self,
    PyObject *args
)
{
    long long pid;

    (void)self;
    (void)args;

    pid = pyos_native_vfork();

    if (pid < 0) {
        return pyos_process_error(
            "vfork"
        );
    }

    return PyLong_FromLongLong(pid);
}

static PyObject *
pyos_process_waitpid(
    PyObject *self,
    PyObject *args
)
{
    long long pid;
    int options;
    pyos_wait_result result;

    (void)self;

    options = 0;

    if (
        !PyArg_ParseTuple(
            args,
            "L|i:waitpid",
            &pid,
            &options
        )
    ) {
        return NULL;
    }

    if (
        pyos_native_waitpid(
            pid,
            options,
            &result
        ) < 0
    ) {
        return pyos_process_error(
            "waitpid"
        );
    }

    return Py_BuildValue(
        "{s:L,s:i,s:i,s:i,s:i,s:i,s:i,s:i}",
        "pid",
        result.pid,
        "status",
        result.status,
        "exited",
        result.exited,
        "signaled",
        result.signaled,
        "stopped",
        result.stopped,
        "continued",
        result.continued,
        "exit_code",
        result.exit_code,
        "signal_number",
        result.signal_number
    );
}

static PyObject *
pyos_process_exit(
    PyObject *self,
    PyObject *args
)
{
    int status;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "i:_exit",
            &status
        )
    ) {
        return NULL;
    }

    pyos_native_exit(status);

    Py_RETURN_NONE;
}

static PyObject *
pyos_process_kill(
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
        return pyos_process_error(
            "kill"
        );
    }

    Py_RETURN_NONE;
}

static PyObject *
pyos_process_pause(
    PyObject *self,
    PyObject *args
)
{
    (void)self;
    (void)args;

    if (
        pyos_native_pause() < 0
    ) {
        return pyos_process_error(
            "pause"
        );
    }

    Py_RETURN_NONE;
}

static PyObject *
pyos_process_sleep(
    PyObject *self,
    PyObject *args
)
{
    unsigned long seconds;
    unsigned long remaining;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "k:sleep",
            &seconds
        )
    ) {
        return NULL;
    }

    remaining = pyos_native_sleep(
        seconds
    );

    return PyLong_FromUnsignedLong(
        remaining
    );
}

static PyObject *
pyos_process_usleep(
    PyObject *self,
    PyObject *args
)
{
    unsigned long microseconds;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "k:usleep",
            &microseconds
        )
    ) {
        return NULL;
    }

    if (
        pyos_native_usleep(
            microseconds
        ) < 0
    ) {
        return pyos_process_error(
            "usleep"
        );
    }

    Py_RETURN_NONE;
}

static PyObject *
pyos_process_gethostname(
    PyObject *self,
    PyObject *args
)
{
    char buffer[4096];
    int result;

    (void)self;
    (void)args;

    result = pyos_native_gethostname(
        buffer,
        sizeof(buffer)
    );

    if (result < 0) {
        return pyos_process_error(
            "gethostname"
        );
    }

    return PyUnicode_FromString(
        buffer
    );
}

static PyObject *
pyos_process_getlogin(
    PyObject *self,
    PyObject *args
)
{
    const char *name;

    (void)self;
    (void)args;

    name = pyos_native_getlogin();

    if (name == NULL) {
        return pyos_process_error(
            "getlogin"
        );
    }

    return PyUnicode_FromString(name);
}

static PyMethodDef pyos_process_methods[] = {
    {
        "getpid",
        pyos_process_getpid,
        METH_NOARGS,
        NULL
    },
    {
        "getppid",
        pyos_process_getppid,
        METH_NOARGS,
        NULL
    },
    {
        "getuid",
        pyos_process_getuid,
        METH_NOARGS,
        NULL
    },
    {
        "geteuid",
        pyos_process_geteuid,
        METH_NOARGS,
        NULL
    },
    {
        "getgid",
        pyos_process_getgid,
        METH_NOARGS,
        NULL
    },
    {
        "getegid",
        pyos_process_getegid,
        METH_NOARGS,
        NULL
    },
    {
        "getgroups",
        pyos_process_getgroups,
        METH_NOARGS,
        NULL
    },
    {
        "setuid",
        pyos_process_setuid,
        METH_VARARGS,
        NULL
    },
    {
        "setgid",
        pyos_process_setgid,
        METH_VARARGS,
        NULL
    },
    {
        "setpgid",
        pyos_process_setpgid,
        METH_VARARGS,
        NULL
    },
    {
        "getpgid",
        pyos_process_getpgid,
        METH_VARARGS,
        NULL
    },
    {
        "getpgrp",
        pyos_process_getpgrp,
        METH_NOARGS,
        NULL
    },
    {
        "setsid",
        pyos_process_setsid,
        METH_NOARGS,
        NULL
    },
    {
        "getsid",
        pyos_process_getsid,
        METH_VARARGS,
        NULL
    },
    {
        "fork",
        pyos_process_fork,
        METH_NOARGS,
        NULL
    },
    {
        "vfork",
        pyos_process_vfork,
        METH_NOARGS,
        NULL
    },
    {
        "waitpid",
        pyos_process_waitpid,
        METH_VARARGS,
        NULL
    },
    {
        "_exit",
        pyos_process_exit,
        METH_VARARGS,
        NULL
    },
    {
        "kill",
        pyos_process_kill,
        METH_VARARGS,
        NULL
    },
    {
        "pause",
        pyos_process_pause,
        METH_NOARGS,
        NULL
    },
    {
        "sleep",
        pyos_process_sleep,
        METH_VARARGS,
        NULL
    },
    {
        "usleep",
        pyos_process_usleep,
        METH_VARARGS,
        NULL
    },
    {
        "gethostname",
        pyos_process_gethostname,
        METH_NOARGS,
        NULL
    },
    {
        "getlogin",
        pyos_process_getlogin,
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
pyos_get_process_methods(void)
{
    return pyos_process_methods;
}
