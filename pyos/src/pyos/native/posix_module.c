#define PY_SSIZE_T_CLEAN

#include <Python.h>

#include "posix_internal.h"


static int
pyos_add_methods(
    PyObject *module,
    PyMethodDef *methods
)
{
    PyMethodDef *method;

    if (methods == NULL) {
        return 0;
    }

    for (method = methods; method->ml_name != NULL; method++) {
        PyObject *function;

        function = PyCFunction_NewEx(
            method,
            NULL,
            module
        );

        if (function == NULL) {
            return -1;
        }

        if (
            PyModule_AddObject(
                module,
                method->ml_name,
                function
            ) < 0
        ) {
            Py_DECREF(function);
            return -1;
        }
    }

    return 0;
}


static int
pyos_add_all_methods(
    PyObject *module
)
{
    PyMethodDef *methods;
    PyMethodDef *(*getters[])(void) = {
        pyos_get_core_methods,
        pyos_get_fd_methods,
        pyos_get_fs_methods,
        pyos_get_directory_methods,
        pyos_get_process_methods,
        pyos_get_time_methods,
        pyos_get_signal_methods,
        pyos_get_terminal_methods,
        pyos_get_poll_methods,
        pyos_get_socket_methods,
        pyos_get_mmap_methods,
        pyos_get_sysinfo_methods,
        pyos_get_exec_methods,
        pyos_get_user_methods,
        pyos_get_mount_methods,
        pyos_get_misc_methods,
        pyos_get_env_methods,
        pyos_get_file_methods,
        pyos_get_resource_methods,
        NULL
    };
    int i;

    for (i = 0; getters[i] != NULL; i++) {
        methods = getters[i]();
        if (pyos_add_methods(module, methods) < 0) {
            return -1;
        }
    }

    return 0;
}


static struct PyModuleDef pyos_native_module = {
    PyModuleDef_HEAD_INIT,
    "_native",
    "Native POSIX interface for pyos.",
    -1,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};


PyMODINIT_FUNC
PyInit__native(void)
{
    PyObject *module;

    module = PyModule_Create(
        &pyos_native_module
    );

    if (module == NULL) {
        return NULL;
    }

    if (pyos_initialize_native() < 0) {
        Py_DECREF(module);
        return NULL;
    }

    if (pyos_add_all_methods(module) < 0) {
        Py_DECREF(module);
        return NULL;
    }

    if (pyos_posix_error != NULL) {
        Py_INCREF(pyos_posix_error);
        if (PyModule_AddObject(module, "POSIXError", pyos_posix_error) < 0) {
            Py_DECREF(pyos_posix_error);
            Py_DECREF(module);
            return NULL;
        }
    }

    if (pyos_native_error != NULL) {
        Py_INCREF(pyos_native_error);
        if (PyModule_AddObject(module, "NativeError", pyos_native_error) < 0) {
            Py_DECREF(pyos_native_error);
            Py_DECREF(module);
            return NULL;
        }
    }

    return module;
}
