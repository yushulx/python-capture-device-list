#include <Python.h>

#include <stdio.h>

#include "pycamera.h"
#include "pywindow.h"

#define INITERROR return NULL

static PyObject *getDeviceList(PyObject *obj, PyObject *args)
{
	PyObject *pyList = PyList_New(0);

	std::vector<CaptureDeviceInfo> devices = ListCaptureDevices();

	for (size_t i = 0; i < devices.size(); i++)
	{
		CaptureDeviceInfo &device = devices[i];

#ifdef _WIN32
		PyObject *pyName = PyUnicode_FromWideChar(device.friendlyName, wcslen(device.friendlyName));
		if (pyName != NULL)
		{
			PyList_Append(pyList, pyName);
			Py_DECREF(pyName);
		}
#else
		// For Linux and macOS: Use char* directly
		PyObject *pyName = PyUnicode_FromString(device.friendlyName);
		if (pyName != NULL)
		{
			PyList_Append(pyList, pyName);
			Py_DECREF(pyName);
		}
#endif
	}

	return pyList;
}

static PyObject *saveJpeg(PyObject *obj, PyObject *args)
{
	const char *filename = NULL;
	int width = 0, height = 0;
	PyObject *byteArray = NULL;

	if (!PyArg_ParseTuple(args, "siiO", &filename, &width, &height, &byteArray))
	{
		PyErr_SetString(PyExc_TypeError, "Expected arguments: str, int, int, PyByteArray");
		return NULL;
	}

	unsigned char *data = (unsigned char *)PyByteArray_AsString(byteArray);
	Py_ssize_t size = PyByteArray_Size(byteArray);

	if (size != (Py_ssize_t)(width * height * 3))
	{
		PyErr_SetString(PyExc_ValueError, "Invalid byte array size for the given width and height.");
		return NULL;
	}

	saveFrameAsJPEG(data, width, height, filename);

	Py_RETURN_NONE;
}

static PyMethodDef litecam_methods[] = {
	{"getDeviceList", getDeviceList, METH_VARARGS, "Get available cameras"},
	{"saveJpeg", saveJpeg, METH_VARARGS, "Get available cameras"},
	{NULL, NULL, 0, NULL}};

static struct PyModuleDef litecam_module_def = {
	PyModuleDef_HEAD_INIT,
	"litecam",
	"Internal \"litecam\" module",
	-1,
	litecam_methods};

PyMODINIT_FUNC PyInit_litecam(void)
{
	PyObject *module = PyModule_Create(&litecam_module_def);
	if (module == NULL)
		INITERROR;

	if (PyType_Ready(&PyCameraType) < 0)
	{
		printf("Failed to initialize PyCameraType\n");
		Py_DECREF(module);
		return NULL;
	}

	if (PyModule_AddObject(module, "PyCamera", (PyObject *)&PyCameraType) < 0)
	{
		printf("Failed to add PyCamera to the module\n");
		Py_DECREF(&PyCameraType);
		Py_DECREF(module);
		INITERROR;
	}

	if (PyType_Ready(&PyWindowType) < 0)
	{
		printf("Failed to initialize PyWindowType\n");
		Py_DECREF(module);
		return NULL;
	}

	if (PyModule_AddObject(module, "PyWindow", (PyObject *)&PyWindowType) < 0)
	{
		printf("Failed to add PyWindow to the module\n");
		Py_DECREF(&PyWindowType);
		Py_DECREF(module);
		INITERROR;
	}

	return module;
}
