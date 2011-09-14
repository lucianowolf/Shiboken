/*
 * This file is part of the Shiboken Python Bindings Generator project.
 *
 * Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
 *
 * Contact: PySide team <contact@pyside.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "helper.h"
#include <stdarg.h>

namespace Shiboken
{

bool sequenceToArgcArgv(PyObject* argList, int* argc, char*** argv, const char* defaultAppName)
{
    if (!PySequence_Check(argList))
        return false;

    if (!defaultAppName)
        defaultAppName = "PySideApplication";

    // Check all items
    Shiboken::AutoDecRef args(PySequence_Fast(argList, 0));
    int numArgs = PySequence_Fast_GET_SIZE(argList);
    for (int i = 0; i < numArgs; ++i) {
        PyObject* item = PySequence_Fast_GET_ITEM(args.object(), i);
        if (!PyBytes_Check(item) && !PyUnicode_Check(item))
            return false;
    }

    bool hasEmptyArgList = numArgs == 0;
    if (hasEmptyArgList)
        numArgs = 1;

    *argc = numArgs;
    *argv = new char*[*argc];

    if (hasEmptyArgList) {
        // Try to get the script name
        PyObject* globals = PyEval_GetGlobals();
        PyObject* appName = PyDict_GetItemString(globals, "__file__");
        (*argv)[0] = strdup(appName ? PyBytes_AS_STRING(appName) : defaultAppName);
    } else {
        for (int i = 0; i < numArgs; ++i) {
            PyObject* item = PySequence_Fast_GET_ITEM(args.object(), i);
            char* string;
            if (PyUnicode_Check(item)) {
                Shiboken::AutoDecRef utf8(PyUnicode_AsUTF8String(item));
                string = strdup(PyBytes_AS_STRING(utf8.object()));
            } else {
                string = strdup(PyBytes_AS_STRING(item));
            }
            (*argv)[i] = string;
        }
    }

    return true;
}

int* sequenceToIntArray(PyObject* obj, bool zeroTerminated)
{
    AutoDecRef seq(PySequence_Fast(obj, "Sequence of ints expected"));
    if (seq.isNull())
        return 0;

    Py_ssize_t size = PySequence_Fast_GET_SIZE(seq.object());
    int* array = new int[size + (zeroTerminated ? 1 : 0)];

    for (int i = 0; i < size; i++) {
        PyObject* item = PySequence_Fast_GET_ITEM(seq.object(), i);
        if (!PyInt_Check(item)) {
            PyErr_SetString(PyExc_TypeError, "Sequence of ints expected");
            delete[] array;
            return 0;
        } else {
            array[i] = PyInt_AsLong(item);
        }
    }

    if (zeroTerminated)
        array[size] = 0;

    return array;
}


int warning(PyObject *category, int stacklevel, const char *format, ...)
{
    va_list args;
    va_start(args, format);
#if _WIN32
    va_list args2 = args;
#else
    va_list args2;
    va_copy(args2, args);
#endif

    // check the necessary memmory
    int result = vsnprintf(NULL, 0, format, args);
    char *message = (char*) malloc(result);
    if (message) {
        // format the message
        vsnprintf(message, result, format, args2);
        result = PyErr_WarnEx(category, message, stacklevel);
        free(message);
    } else {
        result = 0;
    }
    va_end(args2);
    va_end(args);
    return result;
}

} // namespace Shiboken
