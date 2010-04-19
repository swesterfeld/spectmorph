#include <Python.h>
#include "smaudio.hh"
#include "smafile.hh"

/*---------------------------- wrap SpectMorph::Audio ---------------------------*/
typedef struct {
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

static int
Audio_set_fundamental_freq(spectmorph_AudioObject *self, PyObject *value, void *closure)
{
  printf ("FIXME!\n");
#if 0
  if (value == NULL) {
    PyErr_SetString(PyExc_TypeError, "Cannot delete the first attribute");
    return -1;
  }
  
  if (! PyString_Check(value)) {
    PyErr_SetString(PyExc_TypeError, 
                    "The first attribute value must be a string");
    return -1;
  }
      
  Py_DECREF(self->first);
  Py_INCREF(value);
  self->first = value;    
#endif
  return 0;
}

static PyGetSetDef Audio_getseters[] = {
    {"fundamental_freq", 
     (getter)Audio_get_fundamental_freq, (setter)Audio_set_fundamental_freq,
     "fundamental freq",
     NULL},
    {NULL}  /* Sentinel */
};

static PyTypeObject spectmorph_AudioType = {
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

  newaudio = (spectmorph_AudioObject *)spectmorph_AudioType.tp_alloc(&spectmorph_AudioType, 0);
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

  m = Py_InitModule3("spectmorph", SpectMorphMethods, "SpectMorph spectral analysis/morph/resynthesis");

  Py_INCREF (&spectmorph_AudioType);
  PyModule_AddObject (m, "Audio", (PyObject *)&spectmorph_AudioType);
}
