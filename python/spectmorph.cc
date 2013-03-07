// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include <Python.h>
#include "smaudio.hh"
#include <string>
#include <vector>

using std::string;
using std::vector;

/// @cond
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

/*----------- access wrapper for AudioBlock parameters -----------*/

typedef struct
{
  PyObject_HEAD
  /* my fields */
  SpectMorph::Audio *audio;
  size_t             index;
  enum {
    FREQS, PHASES, DEBUG_SAMPLES
  }                  part;
} spectmorph_AudioBlockVecObject;

static PyObject *
AudioBlockVec_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
  spectmorph_AudioBlockVecObject *self;

  self = (spectmorph_AudioBlockVecObject *)type->tp_alloc(type, 0);
  if (self != NULL)
    {
      self->audio = new SpectMorph::Audio();
      self->index = 0;
      self->part = spectmorph_AudioBlockVecObject::FREQS;
    }

  return (PyObject *)self;
}

static void
AudioBlockVec_dealloc (spectmorph_AudioBlockVecObject *self)
{
#if 0        // FIXME: reference counting
  if (self->audio_blocks)
    delete self->audio_blocks;
#endif
  self->ob_type->tp_free ((PyObject *)self);
}

static PyMethodDef AudioBlockVec_methods[] =
{
  {NULL}  /* Sentinel */
};

static PyGetSetDef AudioBlockVec_getseters[] =
{
  {NULL}  /* Sentinel */
};

static Py_ssize_t
AudioBlockVec_length (spectmorph_AudioBlockVecObject *self)
{
  if (self->part == spectmorph_AudioBlockVecObject::FREQS)
    return self->audio->contents[self->index].freqs.size();
  else if (self->part == spectmorph_AudioBlockVecObject::PHASES)
    return self->audio->contents[self->index].phases.size();
  else if (self->part == spectmorph_AudioBlockVecObject::DEBUG_SAMPLES)
    return self->audio->contents[self->index].debug_samples.size();
  return -1;
}

static PyObject*
AudioBlockVec_item (spectmorph_AudioBlockVecObject *self, Py_ssize_t i)
{
  if (self->part == spectmorph_AudioBlockVecObject::FREQS)
    return Py_BuildValue ("d", self->audio->contents[self->index].freqs[i]);
  if (self->part == spectmorph_AudioBlockVecObject::PHASES)
    return Py_BuildValue ("d", self->audio->contents[self->index].phases[i]);
  if (self->part == spectmorph_AudioBlockVecObject::DEBUG_SAMPLES)
    return Py_BuildValue ("d", self->audio->contents[self->index].debug_samples[i]);
  Py_RETURN_FALSE;
#if 0
  spectmorph_AudioBlockObject *audio_block = PyObject_New (spectmorph_AudioBlockObject, &spectmorph_AudioBlockType);
  PyObject *obj = (PyObject *)audio_block;
  Py_IncRef (obj);
  return obj;
#endif
}

static PySequenceMethods spectmorph_AudioBlockVecType_as_sequence =
{
  (lenfunc)AudioBlockVec_length,          /* sq_length */
  0,                                      /* sq_concat */
  0,                                      /* sq_repeat */
  (ssizeargfunc)AudioBlockVec_item,       /* sq_item */
  0,                                      /* sq_slice */
  0,                                      /* sq_ass_item */
  0,                                      /* sq_ass_slice */
  0,                                      /* sq_contains */
};

static PyTypeObject spectmorph_AudioBlockVecType =
{
  PyObject_HEAD_INIT(NULL)
  0,                                        /*ob_size (always set to zero)*/
  "spectmorph.AudioBlockVec",               /*tp_name*/
  sizeof(spectmorph_AudioBlockVecObject),   /*tp_basicsize*/
  0,                                        /*tp_itemsize*/
  (destructor) AudioBlockVec_dealloc,       /*tp_dealloc*/
  0,                                        /*tp_print*/
  0,                                        /*tp_getattr*/
  0,                                        /*tp_setattr*/
  0,                                        /*tp_compare*/
  0,                                        /*tp_repr*/
  0,                                        /*tp_as_number*/
  &spectmorph_AudioBlockVecType_as_sequence,/*tp_as_sequence*/
  0,                                        /*tp_as_mapping*/
  0,                                        /*tp_hash */
  0,                                        /*tp_call*/
  0,                                        /*tp_str*/
  0,                                        /*tp_getattro*/
  0,                                        /*tp_setattro*/
  0,                                        /*tp_as_buffer*/
  Py_TPFLAGS_DEFAULT,                       /*tp_flags*/
  "AudioBlockVec object",                   /* tp_doc */
  0,                                        /* tp_traverse */
  0,                                        /* tp_clear */
  0,                                        /* tp_richcompare */
  0,                                        /* tp_weaklistoffset */
  0,                                        /* tp_iter */
  0,                                        /* tp_iternext */
  AudioBlockVec_methods,                    /* tp_methods */
  0,                                        /* tp_members */
  AudioBlockVec_getseters,                  /* tp_getset */
  0,                                        /* tp_base */
  0,                                        /* tp_dict */
  0,                                        /* tp_descr_get */
  0,                                        /* tp_descr_set */
  0,                                        /* tp_dictoffset */
  0,                                        /* tp_init */
  0,                                        /* tp_alloc */
  AudioBlockVec_new,                        /* tp_new */
};


/*---------------------------- wrap AudioBlock -------------------*/

typedef struct
{
  PyObject_HEAD
  /* my fields */
  SpectMorph::Audio *audio;
  size_t             index;
} spectmorph_AudioBlockObject;

static PyObject *
AudioBlock_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
  spectmorph_AudioBlockObject *self;

  self = (spectmorph_AudioBlockObject *)type->tp_alloc(type, 0);
  if (self != NULL)
    {
      self->audio = new SpectMorph::Audio();
      self->index = 0;
    }

  return (PyObject *)self;
}

static void
AudioBlock_dealloc (spectmorph_AudioBlockObject *self)
{
#if 0        // FIXME: reference counting
  if (self->audio_blocks)
    delete self->audio_blocks;
#endif
  self->ob_type->tp_free ((PyObject *)self);
}

static PyMethodDef AudioBlock_methods[] =
{
  {NULL}  /* Sentinel */
};

static PyObject *
AudioBlock_get_freqs (spectmorph_AudioBlockObject *self, void *closure)
{
  spectmorph_AudioBlockVecObject *audio_block_vec = PyObject_New (spectmorph_AudioBlockVecObject, &spectmorph_AudioBlockVecType);
  audio_block_vec->audio = self->audio;
  audio_block_vec->index = self->index;
  audio_block_vec->part = spectmorph_AudioBlockVecObject::FREQS;
  PyObject *obj = (PyObject *)audio_block_vec;
  Py_IncRef (obj);
  return obj;
}

static PyObject *
AudioBlock_get_phases (spectmorph_AudioBlockObject *self, void *closure)
{
  spectmorph_AudioBlockVecObject *audio_block_vec = PyObject_New (spectmorph_AudioBlockVecObject, &spectmorph_AudioBlockVecType);
  audio_block_vec->audio = self->audio;
  audio_block_vec->index = self->index;
  audio_block_vec->part = spectmorph_AudioBlockVecObject::PHASES;
  PyObject *obj = (PyObject *)audio_block_vec;
  Py_IncRef (obj);
  return obj;
}

static PyObject *
AudioBlock_get_debug_samples (spectmorph_AudioBlockObject *self, void *closure)
{
  spectmorph_AudioBlockVecObject *audio_block_vec = PyObject_New (spectmorph_AudioBlockVecObject, &spectmorph_AudioBlockVecType);
  audio_block_vec->audio = self->audio;
  audio_block_vec->index = self->index;
  audio_block_vec->part = spectmorph_AudioBlockVecObject::DEBUG_SAMPLES;
  PyObject *obj = (PyObject *)audio_block_vec;
  Py_IncRef (obj);
  return obj;
}

static int
AudioBlock_set_freqs (spectmorph_AudioBlockObject *self, PyObject *value, void *closure)
{
  printf ("FIXME\n");
}

static int
AudioBlock_set_phases (spectmorph_AudioBlockObject *self, PyObject *value, void *closure)
{
  printf ("FIXME\n");
}

static int
AudioBlock_set_debug_samples (spectmorph_AudioBlockObject *self, PyObject *value, void *closure)
{
  printf ("FIXME\n");
}

#define GETSETER(foo) {const_cast<char*> (#foo), (getter)AudioBlock_get_##foo, (setter)AudioBlock_set_##foo, const_cast<char*> (#foo), NULL}

static PyGetSetDef AudioBlock_getseters[] =
{
  GETSETER(freqs),
  GETSETER(phases),
  GETSETER(debug_samples),
  {NULL}  /* Sentinel */
};

#undef GETSETER


static PyTypeObject spectmorph_AudioBlockType =
{
  PyObject_HEAD_INIT(NULL)
  0,                                        /*ob_size (always set to zero)*/
  "spectmorph.AudioBlock",                  /*tp_name*/
  sizeof(spectmorph_AudioBlockObject),      /*tp_basicsize*/
  0,                                        /*tp_itemsize*/
  (destructor) AudioBlock_dealloc,          /*tp_dealloc*/
  0,                                        /*tp_print*/
  0,                                        /*tp_getattr*/
  0,                                        /*tp_setattr*/
  0,                                        /*tp_compare*/
  0,                                        /*tp_repr*/
  0,                                        /*tp_as_number*/
  0,                                        /*tp_as_sequence*/
  0,                                        /*tp_as_mapping*/
  0,                                        /*tp_hash */
  0,                                        /*tp_call*/
  0,                                        /*tp_str*/
  0,                                        /*tp_getattro*/
  0,                                        /*tp_setattro*/
  0,                                        /*tp_as_buffer*/
  Py_TPFLAGS_DEFAULT,                       /*tp_flags*/
  "AudioBlock object",                      /* tp_doc */
  0,                                        /* tp_traverse */
  0,                                        /* tp_clear */
  0,                                        /* tp_richcompare */
  0,                                        /* tp_weaklistoffset */
  0,                                        /* tp_iter */
  0,                                        /* tp_iternext */
  AudioBlock_methods,                       /* tp_methods */
  0,                                        /* tp_members */
  AudioBlock_getseters,                     /* tp_getset */
  0,                                        /* tp_base */
  0,                                        /* tp_dict */
  0,                                        /* tp_descr_get */
  0,                                        /* tp_descr_set */
  0,                                        /* tp_dictoffset */
  0,                                        /* tp_init */
  0,                                        /* tp_alloc */
  AudioBlock_new,                           /* tp_new */
};

/*---------------------------- wrap Audio contents ---------------*/

typedef struct
{
  PyObject_HEAD
  /* my fields */
  SpectMorph::Audio *audio;
} spectmorph_AudioContentsObject;

static PyObject *
AudioContents_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
  spectmorph_AudioContentsObject *self;

  self = (spectmorph_AudioContentsObject *)type->tp_alloc(type, 0);
  if (self != NULL)
    self->audio = new SpectMorph::Audio();

  return (PyObject *)self;
}

static void
AudioContents_dealloc (spectmorph_AudioContentsObject *self)
{
#if 0        // FIXME: reference counting
  if (self->audio_blocks)  
    delete self->audio_blocks;
#endif
  self->ob_type->tp_free ((PyObject *)self);
}

static PyMethodDef AudioContents_methods[] =
{
  {NULL}  /* Sentinel */
};

static PyGetSetDef AudioContents_getseters[] =
{
  {NULL}  /* Sentinel */
};

static Py_ssize_t
AudioContents_length (spectmorph_AudioContentsObject *self)
{
  return self->audio->contents.size();
}

static PyObject*
AudioContents_item (spectmorph_AudioContentsObject *self, Py_ssize_t i)
{
  spectmorph_AudioBlockObject *audio_block = PyObject_New (spectmorph_AudioBlockObject, &spectmorph_AudioBlockType);
  audio_block->audio = self->audio;
  audio_block->index = i;
  PyObject *obj = (PyObject *)audio_block;
  Py_IncRef (obj);
  return obj;
}

static PySequenceMethods spectmorph_AudioContentsType_as_sequence =
{
  (lenfunc)AudioContents_length,          /* sq_length */
  0,                                      /* sq_concat */
  0,                                      /* sq_repeat */
  (ssizeargfunc)AudioContents_item,       /* sq_item */
  0,                                      /* sq_slice */
  0,                                      /* sq_ass_item */
  0,                                      /* sq_ass_slice */
  0,                                      /* sq_contains */
};

static PyTypeObject spectmorph_AudioContentsType =
{
  PyObject_HEAD_INIT(NULL)
  0,                                        /*ob_size (always set to zero)*/
  "spectmorph.AudioContents",               /*tp_name*/
  sizeof(spectmorph_AudioContentsObject),   /*tp_basicsize*/
  0,                                        /*tp_itemsize*/
  (destructor) AudioContents_dealloc,       /*tp_dealloc*/
  0,                                        /*tp_print*/
  0,                                        /*tp_getattr*/
  0,                                        /*tp_setattr*/
  0,                                        /*tp_compare*/
  0,                                        /*tp_repr*/
  0,                                        /*tp_as_number*/
  &spectmorph_AudioContentsType_as_sequence,/*tp_as_sequence*/
  0,                                        /*tp_as_mapping*/
  0,                                        /*tp_hash */
  0,                                        /*tp_call*/
  0,                                        /*tp_str*/
  0,                                        /*tp_getattro*/
  0,                                        /*tp_setattro*/
  0,                                        /*tp_as_buffer*/
  Py_TPFLAGS_DEFAULT,                       /*tp_flags*/
  "EntropyPool objects",                    /* tp_doc */
  0,                                        /* tp_traverse */
  0,                                        /* tp_clear */
  0,                                        /* tp_richcompare */
  0,                                        /* tp_weaklistoffset */
  0,                                        /* tp_iter */
  0,                                        /* tp_iternext */
  AudioContents_methods,                    /* tp_methods */
  0,                                        /* tp_members */
  AudioContents_getseters,                  /* tp_getset */
  0,                                        /* tp_base */
  0,                                        /* tp_dict */
  0,                                        /* tp_descr_get */
  0,                                        /* tp_descr_set */
  0,                                        /* tp_dictoffset */
  0,                                        /* tp_init */
  0,                                        /* tp_alloc */
  AudioContents_new,                        /* tp_new */
};

/*---------------------------- wrap SpectMorph::Audio ---------------------------*/
typedef struct
{
  PyObject_HEAD
  /* my fields */
  SpectMorph::Audio *audio;
  spectmorph_AudioContentsObject *audio_contents;
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

static PyObject *
Audio_get_contents (spectmorph_AudioObject *self, void *closure)
{
  PyObject *obj = (PyObject *)self->audio_contents;
  Py_IncRef (obj);
  return obj;
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

static int
Audio_set_contents (spectmorph_AudioObject *self, PyObject *value, void *closure)
{
  printf ("FIXME\n");
}

#define GETSETER(foo) {const_cast<char*> (#foo), (getter)Audio_get_##foo, (setter)Audio_set_##foo, const_cast<char*> (#foo), NULL}

static PyGetSetDef Audio_getseters[] =
{
  GETSETER (fundamental_freq),
  GETSETER (mix_freq),
  GETSETER (frame_size_ms),
  GETSETER (frame_step_ms),
  GETSETER (zeropad),
  GETSETER (contents),
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
      newaudio->audio_contents = PyObject_New (spectmorph_AudioContentsObject, &spectmorph_AudioContentsType);
      newaudio->audio_contents->audio = newaudio->audio;

      BseErrorType error = newaudio->audio->load (filename);
    }

  return (PyObject *)newaudio;
}

static PyMethodDef SpectMorphMethods[] =
{
  { "load_stwafile", load_stwafile, METH_VARARGS, "Load spectmorph model" },
  { NULL, NULL, 0, NULL}
};

PyMODINIT_FUNC
initspectmorph(void)
{
  PyObject *m;

  if (PyType_Ready (&spectmorph_AudioType) < 0)
    return;
  if (PyType_Ready (&spectmorph_AudioContentsType) < 0)
    return;
  if (PyType_Ready (&spectmorph_AudioBlockType) < 0)
    return;
  if (PyType_Ready (&spectmorph_AudioBlockVecType) < 0)
    return;

  m = Py_InitModule3("spectmorph", SpectMorphMethods, "SpectMorph spectral analysis/morphing/resynthesis");

  Py_INCREF (&spectmorph_AudioType);
  PyModule_AddObject (m, "Audio", (PyObject *)&spectmorph_AudioType);
  PyModule_AddObject (m, "AudioContents", (PyObject *)&spectmorph_AudioContentsType);
  PyModule_AddObject (m, "AudioBlock", (PyObject *)&spectmorph_AudioBlockType);
  PyModule_AddObject (m, "AudioBlockVec", (PyObject *)&spectmorph_AudioBlockVecType);
}

/// @endcond
