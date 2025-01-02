#ifndef __PyWindow_H__
#define __PyWindow_H__

#include <Python.h>
#include <structmember.h>

#include "CameraPreview.h"

typedef struct
{
    PyObject_HEAD CameraWindow *handler;

} PyWindow;

static int PyWindow_clear(PyWindow *self)
{
    if (self->handler)
    {
        delete self->handler;
    }
    return 0;
}

static void PyWindow_dealloc(PyWindow *self)
{
    PyWindow_clear(self);
    Py_TYPE(self)->tp_free((PyObject *)self);
}

static PyObject *PyWindow_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    PyWindow *self;

    int width = 0, height = 0;
    char *title = NULL;

    if (!PyArg_ParseTuple(args, "iis", &width, &height, &title))
    {
        return NULL;
    }

    self = (PyWindow *)type->tp_alloc(type, 0);
    if (self != NULL)
    {
        self->handler = new CameraWindow(width, height, title);

        if (!self->handler->Create())
        {
            std::cerr << "Failed to create window." << std::endl;
            return NULL;
        }

        self->handler->Show();
    }

    return (PyObject *)self;
}

static PyObject *waitKey(PyObject *obj, PyObject *args)
{
    PyWindow *self = (PyWindow *)obj;

    const char *key = NULL;

    if (!PyArg_ParseTuple(args, "s", &key) || strlen(key) != 1)
    {
        PyErr_SetString(PyExc_ValueError, "Expected a single character string");
        return NULL;
    }

    bool ret = self->handler->WaitKey(key[0]);
    return Py_BuildValue("i", ret);
}

static PyObject *showFrame(PyObject *obj, PyObject *args)
{
    PyWindow *self = (PyWindow *)obj;

    int width = 0, height = 0;
    PyObject *byteArray = NULL;

    if (!PyArg_ParseTuple(args, "iiO", &width, &height, &byteArray))
    {
        return NULL;
    }

    unsigned char *data = (unsigned char *)PyByteArray_AsString(byteArray);

    self->handler->ShowFrame(data, width, height);

    Py_RETURN_NONE;
}

static PyObject *drawContour(PyObject *obj, PyObject *args)
{
    PyWindow *self = (PyWindow *)obj;

    PyObject *pyPoints = NULL;

    if (!PyArg_ParseTuple(args, "O", &pyPoints))
    {
        return NULL;
    }

    std::vector<std::pair<int, int>> points;

    for (Py_ssize_t i = 0; i < PyList_Size(pyPoints); i++)
    {
        PyObject *item = PyList_GetItem(pyPoints, i);
        int x = PyLong_AsLong(PyTuple_GetItem(item, 0));
        int y = PyLong_AsLong(PyTuple_GetItem(item, 1));
        points.push_back(std::make_pair(x, y));
    }

    self->handler->DrawContour(points);

    Py_RETURN_NONE;
}

static PyObject *drawText(PyObject *obj, PyObject *args)
{
    PyWindow *self = (PyWindow *)obj;

    const char *text = NULL;
    int x = 0, y = 0, fontSize = 0;
    PyObject *pyColor = NULL;

    if (!PyArg_ParseTuple(args, "siiiO", &text, &x, &y, &fontSize, &pyColor))
    {
        return NULL;
    }

    CameraWindow::Color color;
    color.r = (unsigned char)PyLong_AsLong(PyTuple_GetItem(pyColor, 0));
    color.g = (unsigned char)PyLong_AsLong(PyTuple_GetItem(pyColor, 1));
    color.b = (unsigned char)PyLong_AsLong(PyTuple_GetItem(pyColor, 2));

    self->handler->DrawText(text, x, y, fontSize, color);

    Py_RETURN_NONE;
}

static PyMethodDef window_instance_methods[] = {
    {"waitKey", waitKey, METH_VARARGS, NULL},
    {"showFrame", showFrame, METH_VARARGS, NULL},
    {"drawContour", drawContour, METH_VARARGS, NULL},
    {"drawText", drawText, METH_VARARGS, NULL},
    {NULL, NULL, 0, NULL}};

static PyTypeObject PyWindowType = {
    PyVarObject_HEAD_INIT(NULL, 0) "litecam.PyWindow", /* tp_name */
    sizeof(PyWindow),                                  /* tp_basicsize */
    0,                                                 /* tp_itemsize */
    (destructor)PyWindow_dealloc,                      /* tp_dealloc */
    0,                                                 /* tp_print */
    0,                                                 /* tp_getattr */
    0,                                                 /* tp_setattr */
    0,                                                 /* tp_reserved */
    0,                                                 /* tp_repr */
    0,                                                 /* tp_as_number */
    0,                                                 /* tp_as_sequence */
    0,                                                 /* tp_as_mapping */
    0,                                                 /* tp_hash  */
    0,                                                 /* tp_call */
    0,                                                 /* tp_str */
    PyObject_GenericGetAttr,                           /* tp_getattro */
    PyObject_GenericSetAttr,                           /* tp_setattro */
    0,                                                 /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,          /*tp_flags*/
    "PyWindow",                                        /* tp_doc */
    0,                                                 /* tp_traverse */
    0,                                                 /* tp_clear */
    0,                                                 /* tp_richcompare */
    0,                                                 /* tp_weaklistoffset */
    0,                                                 /* tp_iter */
    0,                                                 /* tp_iternext */
    window_instance_methods,                           /* tp_methods */
    0,                                                 /* tp_members */
    0,                                                 /* tp_getset */
    0,                                                 /* tp_base */
    0,                                                 /* tp_dict */
    0,                                                 /* tp_descr_get */
    0,                                                 /* tp_descr_set */
    0,                                                 /* tp_dictoffset */
    0,                                                 /* tp_init */
    0,                                                 /* tp_alloc */
    PyWindow_new,                                      /* tp_new */
};

#endif