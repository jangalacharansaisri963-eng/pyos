#define PY_SSIZE_T_CLEAN

#include <Python.h>

#include <errno.h>
#include <limits.h>
#include <string.h>

#include "posix_internal.h"

PyObject *pyos_posix_error = NULL;
PyObject *pyos_native_error = NULL;

static int pyos_initialized = 0;


int
pyos_get_errno(void)
{
    return errno;
}


const char *
pyos_native_strerror(
    int error_code
)
{
    const char *message;

    message = strerror(error_code);

    if (message == NULL) {
        return "Unknown POSIX error";
    }

    return message;
}


PyObject *
pyos_raise_errno(void)
{
    int error_number;

    error_number = pyos_get_errno();

    if (error_number <= 0) {
        PyErr_SetString(
            PyExc_OSError,
            "POSIX operation failed"
        );

        return NULL;
    }

    PyErr_SetFromErrno(
        PyExc_OSError
    );

    return NULL;
}


PyObject *
pyos_raise_errno_with_path(
    const char *path
)
{
    int error_number;

    error_number = pyos_get_errno();

    if (error_number <= 0) {
        if (path != NULL) {
            PyErr_Format(
                PyExc_OSError,
                "POSIX operation failed: %s",
                path
            );
        } else {
            PyErr_SetString(
                PyExc_OSError,
                "POSIX operation failed"
            );
        }

        return NULL;
    }

    if (path != NULL) {
        PyErr_SetFromErrnoWithFilename(
            PyExc_OSError,
            path
        );
    } else {
        PyErr_SetFromErrno(
            PyExc_OSError
        );
    }

    return NULL;
}


PyObject *
pyos_raise_native_error(
    const char *message
)
{
    if (message == NULL) {
        message = "Native POSIX operation failed";
    }

    if (pyos_native_error != NULL) {
        PyErr_SetString(
            pyos_native_error,
            message
        );
    } else {
        PyErr_SetString(
            PyExc_RuntimeError,
            message
        );
    }

    return NULL;
}


PyObject *
pyos_bool_result(
    int value
)
{
    if (value) {
        Py_RETURN_TRUE;
    }

    Py_RETURN_FALSE;
}


static int
pyos_copy_path_bytes(
    PyObject *object,
    pyos_path_buffer *buffer
)
{
    char *data;
    Py_ssize_t length;

    data = PyBytes_AsString(object);

    if (data == NULL) {
        return -1;
    }

    length = PyBytes_GET_SIZE(object);

    buffer->data = data;
    buffer->length = (long long)length;
    buffer->owns_data = 0;
    buffer->owner = object;

    Py_INCREF(object);

    return 0;
}


static int
pyos_copy_path_unicode(
    PyObject *object,
    pyos_path_buffer *buffer
)
{
    PyObject *encoded;
    char *data;
    Py_ssize_t length;

    encoded = PyUnicode_EncodeFSDefault(object);

    if (encoded == NULL) {
        return -1;
    }

    data = PyBytes_AsString(encoded);

    if (data == NULL) {
        Py_DECREF(encoded);
        return -1;
    }

    length = PyBytes_GET_SIZE(encoded);

    buffer->data = data;
    buffer->length = (long long)length;
    buffer->owns_data = 1;
    buffer->owner = encoded;

    return 0;
}


int
pyos_path_from_object(
    PyObject *object,
    pyos_path_buffer *buffer
)
{
    PyObject *path_object;
    int result;

    if (buffer == NULL) {
        PyErr_SetString(
            PyExc_SystemError,
            "NULL path buffer"
        );

        return -1;
    }

    buffer->data = NULL;
    buffer->length = 0;
    buffer->owns_data = 0;
    buffer->owner = NULL;

    if (object == NULL) {
        PyErr_SetString(
            PyExc_TypeError,
            "path must not be NULL"
        );

        return -1;
    }

    if (PyBytes_Check(object)) {
        return pyos_copy_path_bytes(
            object,
            buffer
        );
    }

    if (PyUnicode_Check(object)) {
        return pyos_copy_path_unicode(
            object,
            buffer
        );
    }

    path_object = PyObject_CallMethod(
        object,
        "__fspath__",
        NULL
    );

    if (path_object == NULL) {
        PyErr_Format(
            PyExc_TypeError,
            "expected str, bytes or os.PathLike object, "
            "not %.200s",
            Py_TYPE(object)->tp_name
        );

        return -1;
    }

    if (
        !PyBytes_Check(path_object) &&
        !PyUnicode_Check(path_object)
    ) {
        PyErr_Format(
            PyExc_TypeError,
            "__fspath__() returned non-string "
            "of type '%.200s'",
            Py_TYPE(path_object)->tp_name
        );

        Py_DECREF(path_object);

        return -1;
    }

    if (PyBytes_Check(path_object)) {
        result = pyos_copy_path_bytes(
            path_object,
            buffer
        );
    } else {
        result = pyos_copy_path_unicode(
            path_object,
            buffer
        );
    }

    Py_DECREF(path_object);

    return result;
}


void
pyos_path_release(
    pyos_path_buffer *buffer
)
{
    if (buffer == NULL) {
        return;
    }

    Py_XDECREF(buffer->owner);

    buffer->data = NULL;
    buffer->length = 0;
    buffer->owns_data = 0;
    buffer->owner = NULL;
}


int
pyos_path_validate(
    const pyos_path_buffer *buffer
)
{
    if (buffer == NULL) {
        PyErr_SetString(
            PyExc_SystemError,
            "NULL path buffer"
        );

        return -1;
    }

    if (buffer->data == NULL) {
        PyErr_SetString(
            PyExc_SystemError,
            "NULL path data"
        );

        return -1;
    }

    if (buffer->length < 0) {
        PyErr_SetString(
            PyExc_ValueError,
            "invalid path length"
        );

        return -1;
    }

    if (buffer->length == 0) {
        PyErr_SetString(
            PyExc_FileNotFoundError,
            "empty path"
        );

        return -1;
    }

    if (
        buffer->length >= PYOS_PATH_MAX
    ) {
        PyErr_SetString(
            PyExc_OSError,
            "path is too long"
        );

        return -1;
    }

    return 0;
}


PyObject *
pyos_bytes_from_buffer(
    const char *data,
    long long length
)
{
    if (length < 0) {
        PyErr_SetString(
            PyExc_ValueError,
            "negative buffer length"
        );

        return NULL;
    }

    if (length > PY_SSIZE_T_MAX) {
        PyErr_SetString(
            PyExc_OverflowError,
            "buffer is too large"
        );

        return NULL;
    }

    if (data == NULL && length != 0) {
        PyErr_SetString(
            PyExc_SystemError,
            "NULL buffer with non-zero length"
        );

        return NULL;
    }

    return PyBytes_FromStringAndSize(
        data,
        (Py_ssize_t)length
    );
}


PyObject *
pyos_unicode_from_buffer(
    const char *data,
    long long length
)
{
    if (length < 0) {
        PyErr_SetString(
            PyExc_ValueError,
            "negative buffer length"
        );

        return NULL;
    }

    if (length > PY_SSIZE_T_MAX) {
        PyErr_SetString(
            PyExc_OverflowError,
            "buffer is too large"
        );

        return NULL;
    }

    if (data == NULL && length != 0) {
        PyErr_SetString(
            PyExc_SystemError,
            "NULL buffer with non-zero length"
        );

        return NULL;
    }

    return PyUnicode_DecodeFSDefaultAndSize(
        data,
        (Py_ssize_t)length
    );
}


PyObject *
pyos_path_result(
    PyObject *original,
    const char *data,
    long long length
)
{
    if (original == NULL) {
        PyErr_SetString(
            PyExc_SystemError,
            "NULL original path"
        );

        return NULL;
    }

    if (PyBytes_Check(original)) {
        return pyos_bytes_from_buffer(
            data,
            length
        );
    }

    return pyos_unicode_from_buffer(
        data,
        length
    );
}


int
pyos_require_integer(
    PyObject *object,
    const char *name,
    long long *result
)
{
    PyObject *value;
    long long converted;

    if (object == NULL) {
        PyErr_Format(
            PyExc_TypeError,
            "%s must be an integer",
            name
        );

        return -1;
    }

    value = PyNumber_Index(object);

    if (value == NULL) {
        PyErr_Format(
            PyExc_TypeError,
            "%s must be an integer",
            name
        );

        return -1;
    }

    converted = PyLong_AsLongLong(value);

    Py_DECREF(value);

    if (PyErr_Occurred()) {
        return -1;
    }

    if (result == NULL) {
        PyErr_SetString(
            PyExc_SystemError,
            "NULL integer result"
        );

        return -1;
    }

    *result = converted;

    return 0;
}


int
pyos_require_fd(
    PyObject *object,
    pyos_fd_t *fd
)
{
    long long value;

    if (fd == NULL) {
        PyErr_SetString(
            PyExc_SystemError,
            "NULL file descriptor result"
        );

        return -1;
    }

    if (
        pyos_require_integer(
            object,
            "file descriptor",
            &value
        ) < 0
    ) {
        return -1;
    }

    if (
        value < 0 ||
        value > INT_MAX
    ) {
        PyErr_SetString(
            PyExc_ValueError,
            "invalid file descriptor"
        );

        return -1;
    }

    *fd = (pyos_fd_t)value;

    return 0;
}


int
pyos_mode_is_regular(
    pyos_mode_t mode
)
{
    return (
        (mode & PYOS_S_IFMT) ==
        PYOS_S_IFREG
    );
}


int
pyos_mode_is_directory(
    pyos_mode_t mode
)
{
    return (
        (mode & PYOS_S_IFMT) ==
        PYOS_S_IFDIR
    );
}


int
pyos_mode_is_symlink(
    pyos_mode_t mode
)
{
    return (
        (mode & PYOS_S_IFMT) ==
        PYOS_S_IFLNK
    );
}


int
pyos_mode_is_character_device(
    pyos_mode_t mode
)
{
    return (
        (mode & PYOS_S_IFMT) ==
        PYOS_S_IFCHR
    );
}


int
pyos_mode_is_block_device(
    pyos_mode_t mode
)
{
    return (
        (mode & PYOS_S_IFMT) ==
        PYOS_S_IFBLK
    );
}


int
pyos_mode_is_fifo(
    pyos_mode_t mode
)
{
    return (
        (mode & PYOS_S_IFMT) ==
        PYOS_S_IFIFO
    );
}


int
pyos_mode_is_socket(
    pyos_mode_t mode
)
{
    return (
        (mode & PYOS_S_IFMT) ==
        PYOS_S_IFSOCK
    );
}


PyObject *
pyos_make_stat_dict(
    const pyos_stat_result *info
)
{
    PyObject *result;

    if (info == NULL) {
        return pyos_raise_native_error(
            "NULL stat result"
        );
    }

    result = PyDict_New();

    if (result == NULL) {
        return NULL;
    }

#define PYOS_SET_STAT(name, value)                                  \
    do {                                                            \
        PyObject *item;                                             \
                                                                    \
        item = PyLong_FromLongLong(                                 \
            (long long)(value)                                      \
        );                                                          \
                                                                    \
        if (item == NULL) {                                         \
            Py_DECREF(result);                                      \
            return NULL;                                            \
        }                                                           \
                                                                    \
        if (PyDict_SetItemString(                                   \
                result,                                             \
                name,                                               \
                item                                                \
            ) < 0)                                                  \
        {                                                           \
            Py_DECREF(item);                                        \
            Py_DECREF(result);                                      \
            return NULL;                                            \
        }                                                           \
                                                                    \
        Py_DECREF(item);                                            \
    } while (0)

    PYOS_SET_STAT("st_mode", info->st_mode);
    PYOS_SET_STAT("st_ino", info->st_ino);
    PYOS_SET_STAT("st_dev", info->st_dev);
    PYOS_SET_STAT("st_nlink", info->st_nlink);
    PYOS_SET_STAT("st_uid", info->st_uid);
    PYOS_SET_STAT("st_gid", info->st_gid);
    PYOS_SET_STAT("st_size", info->st_size);
    PYOS_SET_STAT("st_atime", info->st_atime);
    PYOS_SET_STAT("st_mtime", info->st_mtime);
    PYOS_SET_STAT("st_ctime", info->st_ctime);

#undef PYOS_SET_STAT

    return result;
}


int
pyos_initialize_native(void)
{
    if (pyos_initialized) {
        return 0;
    }

    pyos_posix_error = PyErr_NewException(
        "pyos.native.POSIXError",
        PyExc_OSError,
        NULL
    );

    if (pyos_posix_error == NULL) {
        return -1;
    }

    pyos_native_error = PyErr_NewException(
        "pyos.native.NativeError",
        PyExc_RuntimeError,
        NULL
    );

    if (pyos_native_error == NULL) {
        Py_CLEAR(pyos_posix_error);
        return -1;
    }

    pyos_initialized = 1;

    return 0;
}


void
pyos_finalize_native(void)
{
    Py_CLEAR(pyos_posix_error);
    Py_CLEAR(pyos_native_error);

    pyos_initialized = 0;
}

PyMethodDef *
pyos_get_core_methods(void)
{
    return NULL;
}
