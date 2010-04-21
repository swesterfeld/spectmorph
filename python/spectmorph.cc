/*
 * Copyright (C) 2010 Stefan Westerfeld
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <Python.h>
#include "smaudio.hh"
#include "smafile.hh"
#include <string>
#include <vector>

using std::string;
using std::vector;

static int
attr_float_set (float& d, PyObject *value, const string& name)
{
  if (value == NULL)
    {
      PyErr_Format (PyExc_TypeError, "Cannot delete the %s attribute", name.c_str());
      return -1;
    }
  if (!PyNumber_Check (value))
    {
      PyErr_Format (PyExc_TypeError, "The %s attribute value must be a number", name.c_str());
      return -1;
    }
  PyObject* number = PyNumber_Float (value);
  if (!number)
    {
      PyErr_Format (PyExc_TypeError, "The %s attribute value must be a float", name.c_str());
      return -1;
    }
  d = PyFloat_AsDouble (number);
  Py_DECREF (number);
  return 0;
}

static int
attr_int_set (int& i, PyObject *value, const string& name)
{
  if (value == NULL)
    {
      PyErr_Format (PyExc_TypeError, "Cannot delete the %s attribute", name.c_str());
      return -1;
    }
  i = PyLong_AsLong (value);
  if (i == -1 && PyErr_Occurred())
    {
      PyErr_Format (PyExc_TypeError, "The %s attribute value must be an integer", name.c_str());
      return -1;
    }
  return 0;
}

/*---------------------------- wrap vector<SpectMorph::AudioBlock>---------------*/

#if 0
typedef struct
{
  PyObject_HEAD
  /* my fields */
  vector<SpectMorph::AudioBlock> *audio_blocks;
} spectmorph_AudioBlockVector;

static void
AudioBlockVector_dealloc (spectmorph_AudioObject *self)
{
  if (self->audio_blocks)
    delete self->audio_blocks;
  self->ob_type->tp_free ((PyObject *)self);
}

static PyTypeObject spectmorph_AudioBlockVectorType =
{
  PyObject_HEAD_INIT(NULL)
  0,                                    /*ob_size (always set to zero)*/
  "spectmorph.AudioBlockVector",        /*tp_name*/
  sizeof(spectmorph_AudioBlockVector),  /*tp_basicsize*/
  0,                                    /*tp_itemsize*/
  (destructor) AudioBlockVector_dealloc,/*tp_dealloc*/
  0,                                    /*tp_print*/
  0,                                    /*tp_getattr*/
  0,                                    /*tp_setattr*/
  0,                                    /*tp_compare*/
  0,                                    /*tp_repr*/
  0,                                    /*tp_as_number*/
  0,                                    /*tp_as_sequence*/
  0,                                    /*tp_as_mapping*/
  0,                                    /*tp_hash */
  0,                                    /*tp_call*/
  0,                                    /*tp_str*/
  0,                                    /*tp_getattro*/
  0,                                    /*tp_setattro*/
  0,                                    /*tp_as_buffer*/
  Py_TPFLAGS_DEFAULT,                   /*tp_flags*/
  "EntropyPool objects",                /* tp_doc */
  0,                                    /* tp_traverse */
  0,                                    /* tp_clear */
  0,                                    /* tp_richcompare */
  0,                                    /* tp_weaklistoffset */
  0,                                    /* tp_iter */
  0,                                    /* tp_iternext */
  AudioBlockVector_methods,             /* tp_methods */
  0,                                    /* tp_members */
  AudioBlockVector_getseters,           /* tp_getset */
  0,                                    /* tp_base */
  0,                                    /* tp_dict */
  0,                                    /* tp_descr_get */
  0,                                    /* tp_descr_set */
  0,                                    /* tp_dictoffset */
  0,                                    /* tp_init */
  0,                                    /* tp_alloc */
  AudioBlockVector_new,                 /* tp_new */
};

#endif

/*---------------------------- wrap SpectMorph::Audio ---------------------------*/
typedef struct
{
  PyObject_HEAD
  /* my fields */
  SpectMorph::Audio *audio;
} spectmorph_AudioObject;

static PyObject *
Audio_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
  spectmorph_AudioObject *self;

  self = (spectmorph_AudioObject *)type->tp_alloc(type, 0);
  if (self != NULL)
    self->audio = new SpectMorph::Audio();

  return (PyObject *)self;
}

static void
Audio_dealloc (spectmorph_AudioObject *self)
{
  if (self->audio)
    delete self->audio;
  self->ob_type->tp_free ((PyObject *)self);
}

static PyMethodDef Audio_methods[] = {
#if 0
    {"get16",     (PyCFunction)EntropyPool_get16,     METH_NOARGS,
     "Get the low 16 bytes of the state" },
    {"get32",     (PyCFunction)EntropyPool_get32,     METH_NOARGS,
     "Get the 32-byte long state" },
    {"set32",     (PyCFunction)EntropyPool_set32,     METH_VARARGS,
     "Set the 32-byte long state to a new value" },
    {"bench_diffuse", (PyCFunction) EntropyPool_bench_diffuse, METH_VARARGS,
     "Benchmark diffuse operation by executing N diffuse operations" },
    {"diffuse",   (PyCFunction)EntropyPool_diffuse,   METH_NOARGS,
     "Change state contents to a new pseudo-random state" },
    {"undiffuse", (PyCFunction)EntropyPool_undiffuse, METH_NOARGS,
     "Undo effects of previous diffuse call (testing only)" },
#endif
    {NULL}  /* Sentinel */
};

static PyObject *
Audio_get_fundamental_freq (spectmorph_AudioObject *self, void *closure)
{
  return Py_BuildValue ("d", self->audio->fundamental_freq);
}

static PyObject *
Audio_get_mix_freq (spectmorph_AudioObject *self, void *closure)
{
  return Py_BuildValue ("d", self->audio->mix_freq);
}

static PyObject *
Audio_get_frame_size_ms (spectmorph_AudioObject *self, void *closure)
{
  return Py_BuildValue ("d", self->audio->frame_size_ms);
}

static PyObject *
Audio_get_frame_step_ms (spectmorph_AudioObject *self, void *closure)
{
  return Py_BuildValue ("d", self->audio->frame_step_ms);
}

static PyObject *
Audio_get_zeropad (spectmorph_AudioObject *self, void *closure)
{
  return Py_BuildValue ("i", self->audio->zeropad);
}

static int
Audio_set_fundamental_freq (spectmorph_AudioObject *self, PyObject *value, void *closure)
{
  return attr_float_set (self->audio->fundamental_freq, value, "fundamental_freq");
}

static int
Audio_set_mix_freq (spectmorph_AudioObject *self, PyObject *value, void *closure)
{
  return attr_float_set (self->audio->mix_freq, value, "mix_freq");
}

static int
Audio_set_frame_size_ms (spectmorph_AudioObject *self, PyObject *value, void *closure)
{
  return attr_float_set (self->audio->frame_size_ms, value, "frame_size_ms");
}

static int
Audio_set_frame_step_ms (spectmorph_AudioObject *self, PyObject *value, void *closure)
{
  return attr_float_set (self->audio->frame_step_ms, value, "frame_step_ms");
}

static int
Audio_set_zeropad (spectmorph_AudioObject *self, PyObject *value, void *closure)
{
  return attr_int_set (self->audio->zeropad, value, "zeropad");
}

#define GETSETER(foo) {const_cast<char*> (#foo), (getter)Audio_get_##foo, (setter)Audio_set_##foo, const_cast<char*> (#foo), NULL}

static PyGetSetDef Audio_getseters[] =
{
  GETSETER (fundamental_freq),
  GETSETER (mix_freq),
  GETSETER (frame_size_ms),
  GETSETER (frame_step_ms),
  GETSETER (zeropad),
  {NULL}  /* Sentinel */
};

static PyTypeObject spectmorph_AudioType =
{
  PyObject_HEAD_INIT(NULL)
  0,                              /*ob_size (always set to zero)*/
  "spectmorph.Audio",             /*tp_name*/
  sizeof(spectmorph_AudioObject), /*tp_basicsize*/
  0,                              /*tp_itemsize*/
  (destructor) Audio_dealloc,     /*tp_dealloc*/
  0,                              /*tp_print*/
  0,                              /*tp_getattr*/
  0,                              /*tp_setattr*/
  0,                              /*tp_compare*/
  0,                              /*tp_repr*/
  0,                              /*tp_as_number*/
  0,                              /*tp_as_sequence*/
  0,                              /*tp_as_mapping*/
  0,                              /*tp_hash */
  0,                              /*tp_call*/
  0,                              /*tp_str*/
  0,                              /*tp_getattro*/
  0,                              /*tp_setattro*/
  0,                              /*tp_as_buffer*/
  Py_TPFLAGS_DEFAULT,             /*tp_flags*/
  "EntropyPool objects",          /* tp_doc */
  0,                              /* tp_traverse */
  0,                              /* tp_clear */
  0,                              /* tp_richcompare */
  0,                              /* tp_weaklistoffset */
  0,                              /* tp_iter */
  0,                              /* tp_iternext */
  Audio_methods,                  /* tp_methods */
  0,                              /* tp_members */
  Audio_getseters,                /* tp_getset */
  0,                              /* tp_base */
  0,                              /* tp_dict */
  0,                              /* tp_descr_get */
  0,                              /* tp_descr_set */
  0,                              /* tp_dictoffset */
  0,                              /* tp_init */
  0,                              /* tp_alloc */
  Audio_new,                      /* tp_new */
};

/*-------------------------------------------------------------------------------*/

static PyObject*
load_stwafile (PyObject* self, PyObject* args)
{
  const char *filename;

  if (!PyArg_ParseTuple (args, "s", &filename))
    return NULL;

  spectmorph_AudioObject *newaudio;

  newaudio = PyObject_New (spectmorph_AudioObject, &spectmorph_AudioType);
  if (newaudio != NULL)
    {
      newaudio->audio = new SpectMorph::Audio();

      BseErrorType error = STWAFile::load (filename, *newaudio->audio);
    }

  return (PyObject *)newaudio;
}

static PyMethodDef SpectMorphMethods[] =
{
  { "load_stwafile", load_stwafile, METH_VARARGS, "Propagate entropy through the string" },
  { NULL, NULL, 0, NULL}
};

PyMODINIT_FUNC
initspectmorph(void)
{
  PyObject *m;

  if (PyType_Ready (&spectmorph_AudioType) < 0)
    return;

  m = Py_InitModule3("spectmorph", SpectMorphMethods, "SpectMorph spectral analysis/morphing/resynthesis");

  Py_INCREF (&spectmorph_AudioType);
  PyModule_AddObject (m, "Audio", (PyObject *)&spectmorph_AudioType);
}
