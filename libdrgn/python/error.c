// Copyright 2018-2019 - Omar Sandoval
// SPDX-License-Identifier: GPL-3.0+

#include "drgnpy.h"

static struct drgn_error drgn_error_python = {
	.code = DRGN_ERROR_OTHER,
	.message = "error in Python callback",
};

_Py_IDENTIFIER(drgn_in_python);

bool set_drgn_in_python(void)
{
	PyObject *dict, *key, *value;

	dict = PyThreadState_GetDict();
	if (!dict)
		return false;
	key = _PyUnicode_FromId(&PyId_drgn_in_python);
	if (!key) {
		PyErr_Clear();
		return false;
	}
	value = PyDict_GetItemWithError(dict, key);
	if (value == Py_True)
		return false;
	if ((!value && PyErr_Occurred()) ||
	    PyDict_SetItem(dict, key, Py_True) == -1) {
		PyErr_Clear();
		return false;
	}
	return true;
}

void clear_drgn_in_python(void)
{
	PyObject *exc_type, *exc_value, *exc_traceback;
	PyObject *dict;

	PyErr_Fetch(&exc_type, &exc_value, &exc_traceback);
	dict = PyThreadState_GetDict();
	if (dict)
		_PyDict_SetItemId(dict, &PyId_drgn_in_python, Py_False);
	PyErr_Restore(exc_type, exc_value, exc_traceback);
}

struct drgn_error *drgn_error_from_python(void)
{
	PyObject *exc_type, *exc_value, *exc_traceback, *exc_message;
	PyObject *dict;
	const char *type, *message;
	struct drgn_error *err;

	PyErr_Fetch(&exc_type, &exc_value, &exc_traceback);
	if (!exc_type)
		return NULL;

	dict = PyThreadState_GetDict();
	if (dict && _PyDict_GetItemId(dict, &PyId_drgn_in_python) == Py_True) {
		PyErr_Restore(exc_type, exc_value, exc_traceback);
		return &drgn_error_python;
	}

	type = ((PyTypeObject *)exc_type)->tp_name;
	if (exc_value) {
		exc_message = PyObject_Str(exc_value);
		message = exc_message ? PyUnicode_AsUTF8(exc_message) : NULL;
		if (!message) {
			err = drgn_error_format(DRGN_ERROR_OTHER,
						"%s: <exception str() failed>", type);
			goto out;
		}
	} else {
		exc_message = NULL;
		message = "";
	}

	if (message[0]) {
		err = drgn_error_format(DRGN_ERROR_OTHER, "%s: %s", type,
					message);
	} else {
		err = drgn_error_create(DRGN_ERROR_OTHER, type);
	}

out:
	Py_XDECREF(exc_message);
	Py_XDECREF(exc_traceback);
	Py_XDECREF(exc_value);
	Py_DECREF(exc_type);
	return err;
}

DRGNPY_PUBLIC PyObject *set_drgn_error(struct drgn_error *err)
{
	if (err == &drgn_error_python)
		return NULL;

	switch (err->code) {
	case DRGN_ERROR_NO_MEMORY:
		PyErr_NoMemory();
		break;
	case DRGN_ERROR_INVALID_ARGUMENT:
		PyErr_SetString(PyExc_ValueError, err->message);
		break;
	case DRGN_ERROR_OVERFLOW:
		PyErr_SetString(PyExc_OverflowError, err->message);
		break;
	case DRGN_ERROR_RECURSION:
		PyErr_SetString(PyExc_RecursionError, err->message);
		break;
	case DRGN_ERROR_OS:
		errno = err->errnum;
		PyErr_SetFromErrnoWithFilename(PyExc_OSError, err->path);
		break;
	case DRGN_ERROR_ELF_FORMAT:
	case DRGN_ERROR_DWARF_FORMAT:
		PyErr_SetString(FileFormatError, err->message);
		break;
	case DRGN_ERROR_MISSING_DEBUG_INFO:
		PyErr_SetString(MissingDebugInfoError, err->message);
		break;
	case DRGN_ERROR_SYNTAX:
		PyErr_SetString(PyExc_SyntaxError, err->message);
		break;
	case DRGN_ERROR_LOOKUP:
		PyErr_SetString(PyExc_LookupError, err->message);
		break;
	case DRGN_ERROR_FAULT:
		PyErr_SetString(FaultError, err->message);
		break;
	case DRGN_ERROR_TYPE:
		PyErr_SetString(PyExc_TypeError, err->message);
		break;
	case DRGN_ERROR_ZERO_DIVISION:
		PyErr_SetString(PyExc_ZeroDivisionError, err->message);
		break;
	default:
		PyErr_SetString(PyExc_Exception, err->message);
		break;
	}

	drgn_error_destroy(err);
	return NULL;
}

void *set_error_type_name(const char *format,
			  struct drgn_qualified_type qualified_type)
{
	set_drgn_error(drgn_qualified_type_error(format, qualified_type));
	return NULL;
}
