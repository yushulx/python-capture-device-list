#include <Python.h>

#include <stdio.h>

#include "pycamera.h"

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

static PyMethodDef device_methods[] = {
	{"getDeviceList", getDeviceList, METH_VARARGS, "Get available cameras"},
	{NULL, NULL, 0, NULL}};

static struct PyModuleDef device_module_def = {
	PyModuleDef_HEAD_INIT,
	"device",
	"Internal \"device\" module",
	-1,
	device_methods};

PyMODINIT_FUNC PyInit_device(void)
{
	PyObject *module = PyModule_Create(&device_module_def);
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

	return module;
}
////