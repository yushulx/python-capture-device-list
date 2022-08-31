// https://docs.microsoft.com/en-us/windows/win32/directshow/selecting-a-capture-device
// Python includes
#include <Python.h>

// STD includes
#include <stdio.h>

#include <windows.h>
#include <dshow.h>
#include <comutil.h>
#include <string>

#pragma comment(lib, "strmiids")

#if PY_MAJOR_VERSION >= 3
#ifndef IS_PY3K
#define IS_PY3K 1
#endif
#endif

struct module_state {
    PyObject *error;
};

#if defined(IS_PY3K)
#define GETSTATE(m) ((struct module_state*)PyModule_GetState(m))
#else
#define GETSTATE(m) (&_state)
static struct module_state _state;
#endif

// #pragma comment(lib, "kernel32")
// #pragma comment(lib, "user32")
// #pragma comment(lib, "gdi32")
// #pragma comment(lib, "winspool")
// #pragma comment(lib, "comdlg32")
// #pragma comment(lib, "advapi32")
// #pragma comment(lib, "shell32")
// #pragma comment(lib, "ole32")
// #pragma comment(lib, "oleaut32")
// #pragma comment(lib, "uuid")
// #pragma comment(lib, "odbc32")
// #pragma comment(lib, "odbccp32")
#pragma comment(lib, "comsuppwd.lib")

// https://docs.microsoft.com/en-us/windows/win32/directshow/deletemediatype
void _FreeMediaType(AM_MEDIA_TYPE& mt)
{
    if (mt.cbFormat != 0)
    {
        CoTaskMemFree((PVOID)mt.pbFormat);
        mt.cbFormat = 0;
        mt.pbFormat = NULL;
    }
    if (mt.pUnk != NULL)
    {
        // pUnk should not be used.
        mt.pUnk->Release();
        mt.pUnk = NULL;
    }
}

void _DeleteMediaType(AM_MEDIA_TYPE *pmt)
{
    if (pmt != NULL)
    {
        _FreeMediaType(*pmt); 
        CoTaskMemFree(pmt);
    }
}

HRESULT EnumerateDevices(REFGUID category, IEnumMoniker **ppEnum)
{
	// Create the System Device Enumerator.
	ICreateDevEnum *pDevEnum;
	HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL,
		CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pDevEnum));

	if (SUCCEEDED(hr))
	{
		// Create an enumerator for the category.
		hr = pDevEnum->CreateClassEnumerator(category, ppEnum, 0);
		if (hr == S_FALSE)
		{
			hr = VFW_E_NOT_FOUND;  // The category is empty. Treat as an error.
		}
		pDevEnum->Release();
	}
	return hr;
}

PyObject* DisplayDeviceInformation(IEnumMoniker *pEnum)
{
	// Create an empty Python list
	PyObject* pyList = PyList_New(0); 

	IMoniker *pMoniker = NULL;

	while (pEnum->Next(1, &pMoniker, NULL) == S_OK)
	{
		IPropertyBag *pPropBag;
		HRESULT hr = pMoniker->BindToStorage(0, 0, IID_PPV_ARGS(&pPropBag));
		if (FAILED(hr))
		{
			pMoniker->Release();
			continue;
		}

		// Get supported resolution
		// https://docs.microsoft.com/en-us/windows/win32/directshow/enumerating-media-types
		// https://stackoverflow.com/questions/4359775/windows-how-to-get-cameras-supported-resolutions/4360002#4360002
		IEnumPins *pEnum = NULL;
		IBaseFilter * pFilter = NULL;
		hr = pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&pFilter);
		if (FAILED(hr))
		{
			pMoniker->Release();
			continue;
		}

		hr = pFilter->EnumPins(&pEnum);
		if (FAILED(hr))
		{
			pFilter->Release();
			continue;
		}

		IPin *pPin = NULL;
		PyObject* resolutionList = PyList_New(0); 
		while(S_OK == pEnum->Next(1, &pPin, NULL))
		{
			IEnumMediaTypes *pEnumMediaTypes = NULL;
			AM_MEDIA_TYPE *mediaType = NULL;
			VIDEOINFOHEADER* videoInfoHeader = NULL;
			HRESULT hr = pPin->EnumMediaTypes(&pEnumMediaTypes);
			if (FAILED(hr))
			{
				continue;
			}

			while (hr = pEnumMediaTypes->Next(1, &mediaType, NULL), hr == S_OK)
			{
				if ((mediaType->formattype == FORMAT_VideoInfo) &&
					(mediaType->cbFormat >= sizeof(VIDEOINFOHEADER)) &&
					(mediaType->pbFormat != NULL))
				{
					videoInfoHeader = (VIDEOINFOHEADER*)mediaType->pbFormat;
					videoInfoHeader->bmiHeader.biWidth;  
					videoInfoHeader->bmiHeader.biHeight; 
					PyObject *size = PyTuple_New(2);
					PyTuple_SET_ITEM(size, 0, Py_BuildValue("i", videoInfoHeader->bmiHeader.biWidth));
					PyTuple_SET_ITEM(size, 1, Py_BuildValue("i", videoInfoHeader->bmiHeader.biHeight));
					// printf("%d x %d\n", videoInfoHeader->bmiHeader.biWidth, videoInfoHeader->bmiHeader.biHeight);
					// std::string s = std::to_string(videoInfoHeader->bmiHeader.biWidth) + "x" + std::to_string(videoInfoHeader->bmiHeader.biHeight);
					PyList_Append(resolutionList, size);
				}
				_DeleteMediaType(mediaType);
			}
			pEnumMediaTypes->Release();
		}

		VARIANT var;
		VariantInit(&var);

		// Get description or friendly name.
		hr = pPropBag->Read(L"Description", &var, 0);
		if (FAILED(hr))
		{
			hr = pPropBag->Read(L"FriendlyName", &var, 0);
		}
		if (SUCCEEDED(hr))
		{
			// Append a result to Python list
			char  *pValue = _com_util::ConvertBSTRToString(var.bstrVal);
			PyObject *tuple = PyTuple_New(2);
			PyTuple_SET_ITEM(tuple, 0, Py_BuildValue("s", pValue));
			PyTuple_SET_ITEM(tuple, 1, resolutionList);
			hr = PyList_Append(pyList, tuple);
			delete[] pValue;  
			if (FAILED(hr)) {
				printf("Failed to append the object item at the end of list list\n");
				return pyList;
			}

			// printf("%S\n", var.bstrVal);
			VariantClear(&var);
		}

		hr = pPropBag->Write(L"FriendlyName", &var);

		// WaveInID applies only to audio capture devices.
		hr = pPropBag->Read(L"WaveInID", &var, 0);
		if (SUCCEEDED(hr))
		{
			printf("WaveIn ID: %d\n", var.lVal);
			VariantClear(&var);
		}

		hr = pPropBag->Read(L"DevicePath", &var, 0);
		if (SUCCEEDED(hr))
		{
			// The device path is not intended for display.
			// printf("Device path: %S\n", var.bstrVal);
			VariantClear(&var);
		}

		pPropBag->Release();
		pMoniker->Release();
	}

	return pyList;
}

static PyObject *getDeviceList(PyObject *self, PyObject *args)
{
	PyObject* pyList = NULL; 
	
	HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	if (SUCCEEDED(hr))
	{
		IEnumMoniker *pEnum;

		hr = EnumerateDevices(CLSID_VideoInputDeviceCategory, &pEnum);
		if (SUCCEEDED(hr))
		{
			pyList = DisplayDeviceInformation(pEnum);
			pEnum->Release();
		}
		CoUninitialize();
	}

    return pyList;
}

//-----------------------------------------------------------------------------
static PyMethodDef device_methods[] = {
  {"getDeviceList", getDeviceList, METH_VARARGS, "get device list"},
  {NULL, NULL, 0, NULL}        /* Sentinel */
};

//-----------------------------------------------------------------------------
#if PY_MAJOR_VERSION < 3
PyMODINIT_FUNC init_device(void)
{
  (void) Py_InitModule("device", device_methods);
}
#else /* PY_MAJOR_VERSION >= 3 */
static struct PyModuleDef device_module_def = {
  PyModuleDef_HEAD_INIT,
  "device",
  "Internal \"device\" module",
  -1,
  device_methods
};

PyMODINIT_FUNC PyInit_device(void)
{
  return PyModule_Create(&device_module_def);
}
#endif /* PY_MAJOR_VERSION >= 3 */
