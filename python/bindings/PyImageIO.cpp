/*
 * Copyright (c) 2019, NVIDIA CORPORATION. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include "PyImageIO.h"
#include "PyCUDA.h"

#include "loadImage.h"


// PyImageIO_LoadRGBA
PyObject* PyImageIO_LoadRGBA( PyObject* self, PyObject* args )
{
	const char* filename = NULL;

	if( !PyArg_ParseTuple(args, "s", &filename) )
	{
		PyErr_SetString(PyExc_Exception, LOG_PY_UTILS "loadImageRGBA() failed to parse filename argument");
		return NULL;
	}
		
	// load the image
	void* imgPtr = NULL;
	int   width  = 0;
	int   height = 0;

	if( !loadImageRGBA(filename, (float4**)&imgPtr, &width, &height) )
	{
		PyErr_SetString(PyExc_Exception, LOG_PY_UTILS "loadImageRGBA() failed to load the image");
		return NULL;
	}
		
	// register memory container
	PyObject* capsule = PyCUDA_RegisterImage(imgPtr, width, height, IMAGE_RGBA32F, true);

	if( !capsule )
		return NULL;

	// create dimension objects
	PyObject* pyWidth  = PYLONG_FROM_LONG(width);
	PyObject* pyHeight = PYLONG_FROM_LONG(height);

	// return tuple
	PyObject* tuple = PyTuple_Pack(3, capsule, pyWidth, pyHeight);

	Py_DECREF(capsule);
	Py_DECREF(pyWidth);
	Py_DECREF(pyHeight);

	return tuple;
}


// PyImageIO_SaveRGBA
PyObject* PyImageIO_SaveRGBA( PyObject* self, PyObject* args, PyObject* kwds )
{
	// parse arguments
	const char* filename = NULL;
	PyObject* capsule = NULL;

	int width  = 0;
	int height = 0;
	int quality = 95;

	float max_pixel = 255.0f;

	static char* kwlist[] = {"filename", "image", "width", "height", "max_pixel", "quality", NULL};

	if( !PyArg_ParseTupleAndKeywords(args, kwds, "sO|iif", kwlist, &filename, &capsule, &width, &height, &max_pixel))
	{
		PyErr_SetString(PyExc_Exception, LOG_PY_UTILS "saveImageRGBA() failed to parse args tuple");
		return NULL;
	}

	// verify dimensions
#if 0
	if( width <= 0 || height <= 0 )
	{
		PyErr_SetString(PyExc_Exception, LOG_PY_UTILS "saveImageRGBA() image dimensions are invalid");
		return NULL;
	}

	void* img = PyCapsule_GetPointer(capsule, CUDA_MAPPED_MEMORY_CAPSULE);	// TODO  support GPU-only memory

	if( !img )
	{
		PyErr_SetString(PyExc_Exception, LOG_PY_UTILS "saveImageRGBA() failed to get input image pointer from PyCapsule container");
		return NULL;
	}
#endif

	// get pointer to image data
	PyCudaImage* img = PyCUDA_GetImage(capsule);

	if( img != NULL )
	{
		if( !saveImage(filename, img->base.ptr, img->width, img->height, img->format, quality, make_float2(0,max_pixel)) )
		{
			PyErr_SetString(PyExc_Exception, LOG_PY_UTILS "saveImageRGBA() failed to save the image");
			return NULL;
		}
	}
	else
	{
		PyCudaMemory* mem = PyCUDA_GetMemory(capsule);

		if( !mem )
		{
			PyErr_SetString(PyExc_Exception, LOG_PY_UTILS "saveImageRGBA() wasn't passed a cudaImage or cudaMemory object");
			return NULL;
		}

		if( !saveImageRGBA(filename, (float4*)mem->ptr, width, height, max_pixel, quality) )
		{
			PyErr_SetString(PyExc_Exception, LOG_PY_UTILS "saveImageRGBA() failed to save the image");
			return NULL;
		}
	}

	// return void
	Py_RETURN_NONE;
}


//-------------------------------------------------------------------------------

static PyMethodDef pyImageIO_Functions[] = 
{
	{ "loadImageRGBA", (PyCFunction)PyImageIO_LoadRGBA, METH_VARARGS, "Load an image from disk into GPU memory as float4 RGBA" },
	{ "saveImageRGBA", (PyCFunction)PyImageIO_SaveRGBA, METH_VARARGS|METH_KEYWORDS, "Save a float4 RGBA image to disk" },	
	{NULL}  /* Sentinel */
};

// Register functions
PyMethodDef* PyImageIO_RegisterFunctions()
{
	return pyImageIO_Functions;
}

// Register types
bool PyImageIO_RegisterTypes( PyObject* module )
{
	if( !module )
		return false;
	
	return true;
}

