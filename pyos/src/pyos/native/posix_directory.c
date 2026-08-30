#define PY_SSIZE_T_CLEAN

#include <Python.h>

#include "posix_internal.h"
#include <string.h>

typedef struct {
    long long inode;
    unsigned long long offset;
    unsigned short reclen;
    unsigned char type;
    const char *name;
} pyos_dirent_view;

typedef struct {
    long long handle;
    int valid;
    int closed;
    int owns_handle;
    char *path;
} pyos_directory_state;

typedef struct {
    PyObject_HEAD
    pyos_directory_state state;
} PyOSDirectoryObject;

extern long long pyos_native_opendir(
    const char *path
);

extern int pyos_native_closedir(
    long long handle
);

extern long long pyos_native_readdir(
    long long handle,
    void *buffer,
    unsigned long size
);

extern int pyos_native_mkdir(
    const char *path,
    pyos_mode_t mode
);

extern int pyos_native_rmdir(
    const char *path
);

extern int pyos_native_rename(
    const char *old_path,
    const char *new_path
);

extern int pyos_native_unlink(
    const char *path
);

extern int pyos_native_getdents(
    int fd,
    void *buffer,
    unsigned long size
);

extern int pyos_native_fstatat(
    int dirfd,
    const char *path,
    pyos_stat_result *result,
    int flags
);

extern int pyos_native_mkdirat(
    int dirfd,
    const char *path,
    pyos_mode_t mode
);

extern int pyos_native_unlinkat(
    int dirfd,
    const char *path,
    int flags
);

extern int pyos_native_renameat(
    int oldfd,
    const char *old_path,
    int newfd,
    const char *new_path
);

extern int pyos_native_chdir(
    const char *path
);

extern int pyos_native_fchdir(
    int fd
);

extern int pyos_native_dirfd(
    long long handle
);

extern int pyos_native_errno(
    void
);

extern const char *pyos_native_strerror(
    int error_code
);

static PyObject *
pyos_directory_raise_error(
    const char *operation,
    const char *path
)
{
    int error_code;
    const char *message;

    error_code = pyos_native_errno();

    message = pyos_native_strerror(
        error_code
    );

    if (message == NULL) {
        message = "native POSIX directory operation failed";
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
pyos_directory_copy_name(
    const char *source,
    unsigned long length,
    PyObject **result
)
{
    if (source == NULL) {
        PyErr_SetString(
            PyExc_ValueError,
            "directory entry name is NULL"
        );

        return -1;
    }

    if (length == 0) {
        *result = PyUnicode_FromString("");

        if (*result == NULL) {
            return -1;
        }

        return 0;
    }

    *result = PyUnicode_DecodeFSDefaultAndSize(
        source,
        (Py_ssize_t)length
    );

    if (*result == NULL) {
        return -1;
    }

    return 0;
}

static int
pyos_directory_entry_from_native(
    const pyos_dirent_view *entry,
    PyObject **result
)
{
    PyObject *dictionary;
    PyObject *name;
    PyObject *inode;
    PyObject *type;
    PyObject *offset;
    PyObject *kind;

    if (entry == NULL) {
        PyErr_SetString(
            PyExc_ValueError,
            "directory entry is NULL"
        );

        return -1;
    }

    dictionary = PyDict_New();

    if (dictionary == NULL) {
        return -1;
    }

    {
        unsigned long name_len = 0;
        if (entry->name != NULL) {
            name_len = (unsigned long)strlen(entry->name);
        }
        if (
        pyos_directory_copy_name(
            entry->name,
            name_len,
            &name
        ) < 0
    ) {
        Py_DECREF(dictionary);
        return -1;
    }
    }

    inode = PyLong_FromLongLong(
        entry->inode
    );

    if (inode == NULL) {
        Py_DECREF(name);
        Py_DECREF(dictionary);
        return -1;
    }

    type = PyLong_FromUnsignedLong(
        (unsigned long)entry->type
    );

    if (type == NULL) {
        Py_DECREF(inode);
        Py_DECREF(name);
        Py_DECREF(dictionary);
        return -1;
    }

    offset = PyLong_FromUnsignedLongLong(
        entry->offset
    );

    if (offset == NULL) {
        Py_DECREF(type);
        Py_DECREF(inode);
        Py_DECREF(name);
        Py_DECREF(dictionary);
        return -1;
    }

    if (
        PyDict_SetItemString(
            dictionary,
            "name",
            name
        ) < 0
    ) {
        Py_DECREF(offset);
        Py_DECREF(type);
        Py_DECREF(inode);
        Py_DECREF(name);
        Py_DECREF(dictionary);
        return -1;
    }

    if (
        PyDict_SetItemString(
            dictionary,
            "inode",
            inode
        ) < 0
    ) {
        Py_DECREF(offset);
        Py_DECREF(type);
        Py_DECREF(inode);
        Py_DECREF(name);
        Py_DECREF(dictionary);
        return -1;
    }

    if (
        PyDict_SetItemString(
            dictionary,
            "type",
            type
        ) < 0
    ) {
        Py_DECREF(offset);
        Py_DECREF(type);
        Py_DECREF(inode);
        Py_DECREF(name);
        Py_DECREF(dictionary);
        return -1;
    }

    if (
        PyDict_SetItemString(
            dictionary,
            "offset",
            offset
        ) < 0
    ) {
        Py_DECREF(offset);
        Py_DECREF(type);
        Py_DECREF(inode);
        Py_DECREF(name);
        Py_DECREF(dictionary);
        return -1;
    }

    kind = PyUnicode_FromString(
        "unknown"
    );

    if (kind == NULL) {
        Py_DECREF(offset);
        Py_DECREF(type);
        Py_DECREF(inode);
        Py_DECREF(name);
        Py_DECREF(dictionary);
        return -1;
    }

    switch (entry->type) {
        case PYOS_DT_REG:
            Py_DECREF(kind);
            kind = PyUnicode_FromString("file");
            break;

        case PYOS_DT_DIR:
            Py_DECREF(kind);
            kind = PyUnicode_FromString("directory");
            break;

        case PYOS_DT_LNK:
            Py_DECREF(kind);
            kind = PyUnicode_FromString("symlink");
            break;

        case PYOS_DT_FIFO:
            Py_DECREF(kind);
            kind = PyUnicode_FromString("fifo");
            break;

        case PYOS_DT_CHR:
            Py_DECREF(kind);
            kind = PyUnicode_FromString("character");
            break;

        case PYOS_DT_BLK:
            Py_DECREF(kind);
            kind = PyUnicode_FromString("block");
            break;

        case PYOS_DT_SOCK:
            Py_DECREF(kind);
            kind = PyUnicode_FromString("socket");
            break;

        default:
            break;
    }

    if (kind == NULL) {
        Py_DECREF(offset);
        Py_DECREF(type);
        Py_DECREF(inode);
        Py_DECREF(name);
        Py_DECREF(dictionary);
        return -1;
    }

    if (
        PyDict_SetItemString(
            dictionary,
            "kind",
            kind
        ) < 0
    ) {
        Py_DECREF(kind);
        Py_DECREF(offset);
        Py_DECREF(type);
        Py_DECREF(inode);
        Py_DECREF(name);
        Py_DECREF(dictionary);
        return -1;
    }

    Py_DECREF(kind);
    Py_DECREF(offset);
    Py_DECREF(type);
    Py_DECREF(inode);
    Py_DECREF(name);

    *result = dictionary;

    return 0;
}

static int
pyos_directory_decode_entries(
    const unsigned char *buffer,
    unsigned long length,
    PyObject *result
)
{
    unsigned long position;

    position = 0;

    while (
        position + 19 <= length
    ) {
        pyos_dirent_view entry;
        unsigned long name_offset;
        unsigned long name_length;
        unsigned long record_size;
        PyObject *object;

        entry.inode = 0;
        entry.offset = 0;
        entry.reclen = 0;
        entry.type = PYOS_DT_UNKNOWN;
        entry.name = NULL;

        if (
            sizeof(long long) <= length - position
        ) {
            const unsigned char *base;

            base = buffer + position;

            entry.inode =
                (long long)(
                    ((unsigned long long)base[0]) |
                    ((unsigned long long)base[1] << 8) |
                    ((unsigned long long)base[2] << 16) |
                    ((unsigned long long)base[3] << 24) |
                    ((unsigned long long)base[4] << 32) |
                    ((unsigned long long)base[5] << 40) |
                    ((unsigned long long)base[6] << 48) |
                    ((unsigned long long)base[7] << 56)
                );

            entry.offset =
                ((unsigned long long)base[8]) |
                ((unsigned long long)base[9] << 8) |
                ((unsigned long long)base[10] << 16) |
                ((unsigned long long)base[11] << 24) |
                ((unsigned long long)base[12] << 32) |
                ((unsigned long long)base[13] << 40) |
                ((unsigned long long)base[14] << 48) |
                ((unsigned long long)base[15] << 56);

            entry.reclen =
                (unsigned short)(
                    ((unsigned short)base[16]) |
                    ((unsigned short)base[17] << 8)
                );

            entry.type = base[18];

            name_offset = 19;

            if (
                entry.reclen < name_offset
            ) {
                PyErr_SetString(
                    PyExc_OSError,
                    "invalid native directory record"
                );

                return -1;
            }

            record_size = entry.reclen;

            name_length =
                record_size - name_offset;

            if (
                name_length > 0
            ) {
                const char *name;

                name = (const char *)(
                    buffer +
                    position +
                    name_offset
                );

                while (
                    name_length > 0 &&
                    name[name_length - 1] == '\0'
                ) {
                    name_length--;
                }

                entry.name = name;
            }
        } else {
            break;
        }

        if (
            entry.name == NULL
        ) {
            position += entry.reclen;

            continue;
        }

        if (
            pyos_directory_entry_from_native(
                &entry,
                &object
            ) < 0
        ) {
            return -1;
        }

        if (
            PyList_Append(
                result,
                object
            ) < 0
        ) {
            Py_DECREF(object);
            return -1;
        }

        Py_DECREF(object);

        position += entry.reclen;
    }

    return 0;
}

static PyObject *
pyos_directory_listdir(
    PyObject *self,
    PyObject *args,
    PyObject *kwargs
)
{
    const char *path;
    long long handle;
    unsigned char buffer[32768];
    PyObject *result;
    static char *keywords[] = {
        "path",
        NULL
    };

    (void)self;

    path = ".";

    if (
        !PyArg_ParseTupleAndKeywords(
            args,
            kwargs,
            "|s:listdir",
            keywords,
            &path
        )
    ) {
        return NULL;
    }

    handle = pyos_native_opendir(path);

    if (handle < 0) {
        return pyos_directory_raise_error(
            "opendir",
            path
        );
    }

    result = PyList_New(0);

    if (result == NULL) {
        pyos_native_closedir(handle);
        return NULL;
    }

    for (;;) {
        long long count;

        count = pyos_native_readdir(
            handle,
            buffer,
            sizeof(buffer)
        );

        if (count < 0) {
            Py_DECREF(result);
            pyos_native_closedir(handle);

            return pyos_directory_raise_error(
                "readdir",
                path
            );
        }

        if (count == 0) {
            break;
        }

        if (
            pyos_directory_decode_entries(
                buffer,
                (unsigned long)count,
                result
            ) < 0
        ) {
            Py_DECREF(result);
            pyos_native_closedir(handle);
            return NULL;
        }
    }

    if (
        pyos_native_closedir(handle) < 0
    ) {
        Py_DECREF(result);

        return pyos_directory_raise_error(
            "closedir",
            path
        );
    }

    return result;
}

static PyObject *
pyos_directory_mkdir(
    PyObject *self,
    PyObject *args,
    PyObject *kwargs
)
{
    const char *path;
    unsigned long mode;

    static char *keywords[] = {
        "path",
        "mode",
        NULL
    };

    (void)self;

    path = NULL;
    mode = 0777;

    if (
        !PyArg_ParseTupleAndKeywords(
            args,
            kwargs,
            "s|k:mkdir",
            keywords,
            &path,
            &mode
        )
    ) {
        return NULL;
    }

    if (
        pyos_native_mkdir(
            path,
            (pyos_mode_t)mode
        ) < 0
    ) {
        return pyos_directory_raise_error(
            "mkdir",
            path
        );
    }

    Py_RETURN_NONE;
}

static PyObject *
pyos_directory_mkdirat(
    PyObject *self,
    PyObject *args,
    PyObject *kwargs
)
{
    int dirfd;
    const char *path;
    unsigned long mode;

    static char *keywords[] = {
        "dirfd",
        "path",
        "mode",
        NULL
    };

    (void)self;

    mode = 0777;

    if (
        !PyArg_ParseTupleAndKeywords(
            args,
            kwargs,
            "is|k:mkdirat",
            keywords,
            &dirfd,
            &path,
            &mode
        )
    ) {
        return NULL;
    }

    if (
        pyos_native_mkdirat(
            dirfd,
            path,
            (pyos_mode_t)mode
        ) < 0
    ) {
        return pyos_directory_raise_error(
            "mkdirat",
            path
        );
    }

    Py_RETURN_NONE;
}

static PyObject *
pyos_directory_rmdir(
    PyObject *self,
    PyObject *args
)
{
    const char *path;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "s:rmdir",
            &path
        )
    ) {
        return NULL;
    }

    if (
        pyos_native_rmdir(path) < 0
    ) {
        return pyos_directory_raise_error(
            "rmdir",
            path
        );
    }

    Py_RETURN_NONE;
}

static PyObject *
pyos_directory_unlink(
    PyObject *self,
    PyObject *args
)
{
    const char *path;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "s:unlink",
            &path
        )
    ) {
        return NULL;
    }

    if (
        pyos_native_unlink(path) < 0
    ) {
        return pyos_directory_raise_error(
            "unlink",
            path
        );
    }

    Py_RETURN_NONE;
}

static PyObject *
pyos_directory_unlinkat(
    PyObject *self,
    PyObject *args
)
{
    int dirfd;
    const char *path;
    int flags;

    (void)self;

    flags = 0;

    if (
        !PyArg_ParseTuple(
            args,
            "is|i:unlinkat",
            &dirfd,
            &path,
            &flags
        )
    ) {
        return NULL;
    }

    if (
        pyos_native_unlinkat(
            dirfd,
            path,
            flags
        ) < 0
    ) {
        return pyos_directory_raise_error(
            "unlinkat",
            path
        );
    }

    Py_RETURN_NONE;
}

static PyObject *
pyos_directory_rename(
    PyObject *self,
    PyObject *args
)
{
    const char *old_path;
    const char *new_path;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "ss:rename",
            &old_path,
            &new_path
        )
    ) {
        return NULL;
    }

    if (
        pyos_native_rename(
            old_path,
            new_path
        ) < 0
    ) {
        return pyos_directory_raise_error(
            "rename",
            old_path
        );
    }

    Py_RETURN_NONE;
}

static PyObject *
pyos_directory_renameat(
    PyObject *self,
    PyObject *args
)
{
    int oldfd;
    int newfd;
    const char *old_path;
    const char *new_path;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "is is:renameat",
            &oldfd,
            &old_path,
            &newfd,
            &new_path
        )
    ) {
        return NULL;
    }

    if (
        pyos_native_renameat(
            oldfd,
            old_path,
            newfd,
            new_path
        ) < 0
    ) {
        return pyos_directory_raise_error(
            "renameat",
            old_path
        );
    }

    Py_RETURN_NONE;
}

static PyObject *
pyos_directory_stat(
    PyObject *self,
    PyObject *args
)
{
    const char *path;
    pyos_stat_result info;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "s:stat",
            &path
        )
    ) {
        return NULL;
    }

    if (
        pyos_native_stat(
            path,
            &info
        ) < 0
    ) {
        return pyos_directory_raise_error(
            "stat",
            path
        );
    }

    return pyos_make_stat_dict(
        &info
    );
}

static PyObject *
pyos_directory_lstat(
    PyObject *self,
    PyObject *args
)
{
    const char *path;
    pyos_stat_result info;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "s:lstat",
            &path
        )
    ) {
        return NULL;
    }

    if (
        pyos_native_lstat(
            path,
            &info
        ) < 0
    ) {
        return pyos_directory_raise_error(
            "lstat",
            path
        );
    }

    return pyos_make_stat_dict(
        &info
    );
}

static PyObject *
pyos_directory_fstatat(
    PyObject *self,
    PyObject *args
)
{
    int dirfd;
    const char *path;
    int flags;
    pyos_stat_result info;

    (void)self;

    flags = 0;

    if (
        !PyArg_ParseTuple(
            args,
            "is|i:fstatat",
            &dirfd,
            &path,
            &flags
        )
    ) {
        return NULL;
    }

    if (
        pyos_native_fstatat(
            dirfd,
            path,
            &info,
            flags
        ) < 0
    ) {
        return pyos_directory_raise_error(
            "fstatat",
            path
        );
    }

    return pyos_make_stat_dict(
        &info
    );
}

static PyObject *
pyos_directory_chdir(
    PyObject *self,
    PyObject *args
)
{
    const char *path;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "s:chdir",
            &path
        )
    ) {
        return NULL;
    }

    if (
        pyos_native_chdir(path) < 0
    ) {
        return pyos_directory_raise_error(
            "chdir",
            path
        );
    }

    Py_RETURN_NONE;
}

static PyObject *
pyos_directory_fchdir(
    PyObject *self,
    PyObject *args
)
{
    int fd;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "i:fchdir",
            &fd
        )
    ) {
        return NULL;
    }

    if (
        pyos_native_fchdir(fd) < 0
    ) {
        return pyos_directory_raise_error(
            "fchdir",
            NULL
        );
    }

    Py_RETURN_NONE;
}

static PyObject *
pyos_directory_dirfd(
    PyObject *self,
    PyObject *args
)
{
    long long handle;
    int fd;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "L:dirfd",
            &handle
        )
    ) {
        return NULL;
    }

    fd = pyos_native_dirfd(handle);

    if (fd < 0) {
        return pyos_directory_raise_error(
            "dirfd",
            NULL
        );
    }

    return PyLong_FromLong(fd);
}

static PyObject *
pyos_directory_exists(
    PyObject *self,
    PyObject *args
)
{
    const char *path;
    pyos_stat_result info;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "s:exists",
            &path
        )
    ) {
        return NULL;
    }

    if (
        pyos_native_stat(
            path,
            &info
        ) < 0
    ) {
        PyErr_Clear();
        Py_RETURN_FALSE;
    }

    Py_RETURN_TRUE;
}

static PyObject *
pyos_directory_isdir(
    PyObject *self,
    PyObject *args
)
{
    const char *path;
    pyos_stat_result info;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "s:isdir",
            &path
        )
    ) {
        return NULL;
    }

    if (
        pyos_native_stat(
            path,
            &info
        ) < 0
    ) {
        PyErr_Clear();
        Py_RETURN_FALSE;
    }

    return pyos_bool_result(
        pyos_mode_is_directory(
            info.st_mode
        )
    );
}

static PyObject *
pyos_directory_isfile(
    PyObject *self,
    PyObject *args
)
{
    const char *path;
    pyos_stat_result info;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "s:isfile",
            &path
        )
    ) {
        return NULL;
    }

    if (
        pyos_native_stat(
            path,
            &info
        ) < 0
    ) {
        PyErr_Clear();
        Py_RETURN_FALSE;
    }

    return pyos_bool_result(
        pyos_mode_is_regular(
            info.st_mode
        )
    );
}

static PyObject *
pyos_directory_islink(
    PyObject *self,
    PyObject *args
)
{
    const char *path;
    pyos_stat_result info;

    (void)self;

    if (
        !PyArg_ParseTuple(
            args,
            "s:islink",
            &path
        )
    ) {
        return NULL;
    }

    if (
        pyos_native_lstat(
            path,
            &info
        ) < 0
    ) {
        PyErr_Clear();
        Py_RETURN_FALSE;
    }

    return pyos_bool_result(
        pyos_mode_is_symlink(
            info.st_mode
        )
    );
}

PyMethodDef pyos_directory_methods[] = {
    {
        "listdir",
        (PyCFunction)pyos_directory_listdir,
        METH_VARARGS | METH_KEYWORDS,
        NULL
    },
    {
        "mkdir",
        (PyCFunction)pyos_directory_mkdir,
        METH_VARARGS | METH_KEYWORDS,
        NULL
    },
    {
        "mkdirat",
        (PyCFunction)pyos_directory_mkdirat,
        METH_VARARGS | METH_KEYWORDS,
        NULL
    },
    {
        "rmdir",
        pyos_directory_rmdir,
        METH_VARARGS,
        NULL
    },
    {
        "unlink",
        pyos_directory_unlink,
        METH_VARARGS,
        NULL
    },
    {
        "unlinkat",
        pyos_directory_unlinkat,
        METH_VARARGS,
        NULL
    },
    {
        "rename",
        pyos_directory_rename,
        METH_VARARGS,
        NULL
    },
    {
        "renameat",
        pyos_directory_renameat,
        METH_VARARGS,
        NULL
    },
    {
        "stat",
        pyos_directory_stat,
        METH_VARARGS,
        NULL
    },
    {
        "lstat",
        pyos_directory_lstat,
        METH_VARARGS,
        NULL
    },
    {
        "fstatat",
        pyos_directory_fstatat,
        METH_VARARGS,
        NULL
    },
    {
        "chdir",
        pyos_directory_chdir,
        METH_VARARGS,
        NULL
    },
    {
        "fchdir",
        pyos_directory_fchdir,
        METH_VARARGS,
        NULL
    },
    {
        "dirfd",
        pyos_directory_dirfd,
        METH_VARARGS,
        NULL
    },
    {
        "exists",
        pyos_directory_exists,
        METH_VARARGS,
        NULL
    },
    {
        "isdir",
        pyos_directory_isdir,
        METH_VARARGS,
        NULL
    },
    {
        "isfile",
        pyos_directory_isfile,
        METH_VARARGS,
        NULL
    },
    {
        "islink",
        pyos_directory_islink,
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
pyos_get_directory_methods(void)
{
    return pyos_directory_methods;
}
