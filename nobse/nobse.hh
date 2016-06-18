// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_NO_BSE_HH
#define SPECTMORPH_NO_BSE_HH

#include <glib.h>
#include <string>
#include <vector>
#include <string>

/* Rapicorn fake */

typedef long long           int64;
typedef unsigned long long  uint64;
typedef guint               uint;
typedef guint8              uint8;

#define RAPICORN_CLASS_NON_COPYABLE(Class)        private: Class (const Class&); Class& operator= (const Class&);
#define RAPICORN_PRINTF(format_idx, arg_idx)      __attribute__ ((__format__ (__printf__, format_idx, arg_idx)))
#define RAPICORN_AIDA_ENUM_DEFINE_ARITHMETIC_EQ(Enum)   \
  bool constexpr operator== (Enum v, int64_t n) { return int64_t (v) == n; } \
  bool constexpr operator== (int64_t n, Enum v) { return n == int64_t (v); } \
  bool constexpr operator!= (Enum v, int64_t n) { return int64_t (v) != n; } \
  bool constexpr operator!= (int64_t n, Enum v) { return n != int64_t (v); }

namespace Rapicorn
{

/* --- memory utils --- */
void* malloc_aligned            (size_t                total_size,
                                 size_t                alignment,
                                 uint8               **free_pointer);

template<class T, int ALIGN>
class AlignedArray {
  unsigned char *unaligned_mem;
  T *data;
  size_t n_elements;
  void
  allocate_aligned_data()
  {
    g_assert ((ALIGN % sizeof (T)) == 0);
    data = reinterpret_cast<T *> (malloc_aligned (n_elements * sizeof (T), ALIGN, &unaligned_mem));
  }
public:
  AlignedArray (size_t n_elements) :
    n_elements (n_elements)
  {
    allocate_aligned_data();
    for (size_t i = 0; i < n_elements; i++)
      new (data + i) T();
  }
  ~AlignedArray()
  {
    /* C++ destruction order: last allocated element is deleted first */
    while (n_elements)
      data[--n_elements].~T();
    g_free (unaligned_mem);
  }
  T&
  operator[] (size_t pos)
  {
    return data[pos];
  }
  const T&
  operator[] (size_t pos) const
  {
    return data[pos];
  }
};

}

namespace Bse {

/* Bse Error Type */
enum class Error : int64_t {
  NONE = 0, // _("OK")
  INTERNAL = 1, // _("Internal error (please report)")
  UNKNOWN = 2, // _("Unknown error")
  IO = 3, // _("Input/output error")
  PERMS = 4, // _("Insufficient permissions")
  FILE_BUSY = 5, // _("Device or resource busy")
  FILE_EXISTS = 6, // _("File exists already")
  FILE_EOF = 7, // _("End of file")
  FILE_EMPTY = 8, // _("File empty")
  FILE_NOT_FOUND = 9, // _("No such file, device or directory")
  FILE_IS_DIR = 10, // _("Is a directory")
  FILE_OPEN_FAILED = 11, // _("Open failed")
  FILE_SEEK_FAILED = 12, // _("Seek failed")
  FILE_READ_FAILED = 13, // _("Read failed")
  FILE_WRITE_FAILED = 14, // _("Write failed")
  MANY_FILES = 15, // _("Too many open files")
  NO_FILES = 16, // _("Too many open files in system")
  NO_SPACE = 17, // _("No space left on device")
  NO_MEMORY = 18, // _("Out of memory")
  NO_HEADER = 19, // _("Failed to detect header")
  NO_SEEK_INFO = 20, // _("Failed to retrieve seek information")
  NO_DATA = 21, // _("No data available")
  DATA_CORRUPT = 22, // _("Data corrupt")
  WRONG_N_CHANNELS = 23, // _("Wrong number of channels")
  FORMAT_INVALID = 24, // _("Invalid format")
  FORMAT_UNKNOWN = 25, // _("Unknown format")
  DATA_UNMATCHED = 26, // _("Requested data values unmatched")
  TEMP = 27, // _("Temporary error")
  WAVE_NOT_FOUND = 28, // _("No such wave")
  CODEC_FAILURE = 29, // _("Codec failure")
  UNIMPLEMENTED = 30, // _("Functionality not implemented")
  INVALID_PROPERTY = 31, // _("Invalid object property")
  INVALID_MIDI_CONTROL = 32, // _("Invalid MIDI control type")
  PARSE_ERROR = 33, // _("Parsing error")
  SPAWN = 34, // _("Failed to spawn child process")
  DEVICE_NOT_AVAILABLE = 35, // _("No device (driver) available")
  DEVICE_ASYNC = 36, // _("Device not async capable")
  DEVICE_BUSY = 37, // _("Device busy")
  DEVICE_FORMAT = 38, // _("Failed to configure device format")
  DEVICE_BUFFER = 39, // _("Failed to configure device buffer")
  DEVICE_LATENCY = 40, // _("Failed to configure device latency")
  DEVICE_CHANNELS = 41, // _("Failed to configure number of device channels")
  DEVICE_FREQUENCY = 42, // _("Failed to configure device frequency")
  DEVICES_MISMATCH = 43, // _("Device configurations mismatch")
  SOURCE_NO_SUCH_MODULE = 44, // _("No such synthesis module")
  SOURCE_NO_SUCH_ICHANNEL = 45, // _("No such input channel")
  SOURCE_NO_SUCH_OCHANNEL = 46, // _("No such output channel")
  SOURCE_NO_SUCH_CONNECTION = 47, // _("Input/Output channels not connected")
  SOURCE_PRIVATE_ICHANNEL = 48, // _("Input channel is private")
  SOURCE_ICHANNEL_IN_USE = 49, // _("Input channel already in use")
  SOURCE_CHANNELS_CONNECTED = 50, // _("Input/output channels already connected")
  SOURCE_CONNECTION_INVALID = 51, // _("Invalid synthesis module connection")
  SOURCE_PARENT_MISMATCH = 52, // _("Parent mismatch")
  SOURCE_BAD_LOOPBACK = 53, // _("Bad loopback")
  SOURCE_BUSY = 54, // _("Synthesis module currently busy")
  SOURCE_TYPE_INVALID = 55, // _("Invalid synthsis module type")
  PROC_NOT_FOUND = 56, // _("No such procedure")
  PROC_BUSY = 57, // _("Procedure currently busy")
  PROC_PARAM_INVAL = 58, // _("Procedure parameter invalid")
  PROC_EXECUTION = 59, // _("Procedure execution failed")
  PROC_ABORT = 60, // _("Procedure execution aborted")
  NO_ENTRY = 61, // _("No such entry")
  NO_EVENT = 62, // _("No such event")
  NO_TARGET = 63, // _("No target")
  NOT_OWNER = 64, // _("Ownership mismatch")
  INVALID_OFFSET = 65, // _("Invalid offset")
  INVALID_DURATION = 66, // _("Invalid duration")
  INVALID_OVERLAP = 67, // _("Invalid overlap")
};

RAPICORN_AIDA_ENUM_DEFINE_ARITHMETIC_EQ (Error);

}

const gchar* bse_error_blurb (Bse::Error error_value);

namespace Bse
{
  struct Spinlock
  {
  };
}

/* Gsl DataHandles */
struct GslDataHandle;
struct GslDataHandleSetup;

struct GslDataHandleFuncs
{
  Bse::Error 	 (*open)		(GslDataHandle		*data_handle,
					 GslDataHandleSetup	*setup);
  int64		 (*read)		(GslDataHandle		*data_handle,
					 int64			 voffset, /* in values */
					 int64			 n_values,
					 gfloat			*values);
  void		 (*close)		(GslDataHandle		*data_handle);
  GslDataHandle* (*get_source)          (GslDataHandle          *data_handle);
  int64          (*get_state_length)	(GslDataHandle	        *data_handle);
  void           (*destroy)		(GslDataHandle		*data_handle);
};
struct GslDataHandleSetup
{
  guint		n_channels;
  int64		n_values;
  gchar       **xinfos;
  guint         bit_depth : 8;
  guint         needs_cache : 1;
  gfloat        mix_freq;
};
struct GslDataHandle
{
  /* constant members */
  GslDataHandleFuncs *vtable;
  gchar		     *name;
  /* common members */
  Bse::Spinlock       spinlock;
  guint		      ref_count;
  guint		      open_count;
  /* opened data handle setup (open_count > 0) */
  GslDataHandleSetup  setup;
};
struct GslDataPeekBuffer
{
};

Bse::Error   	  gsl_data_handle_open		    (GslDataHandle	  *dhandle);
void              gsl_data_handle_close             (GslDataHandle        *dhandle);
int64		  gsl_data_handle_length	    (GslDataHandle	  *data_handle);
int64		  gsl_data_handle_read		    (GslDataHandle	  *data_handle,
						     int64		   value_offset,
						     int64		   n_values,
						     gfloat		  *values);
guint		  gsl_data_handle_n_channels	    (GslDataHandle	  *data_handle);
gfloat		  gsl_data_handle_mix_freq	    (GslDataHandle	  *data_handle);
GslDataHandle*	  gsl_data_handle_new_mem	    (guint		   n_channels,
						     guint                 bit_depth,
						     gfloat                mix_freq,
						     gfloat                osc_freq,
						     int64		   n_values,
						     const gfloat	  *values,
						     void                (*free) (gpointer values));
GslDataHandle*	  gsl_data_handle_new_insert	    (GslDataHandle	  *src_handle,
						     guint                 pasted_bit_depth,
						     int64		   insertion_offset,
						     int64		   n_paste_values,
						     const gfloat	  *paste_values,
						     void                (*free) (gpointer values));
GslDataHandle*	  bse_data_handle_new_upsample2	    (GslDataHandle  *src_handle,	// implemented in bsedatahandle-resample.cc
						     int             precision_bits);
int64		  gsl_data_handle_length	    (GslDataHandle	  *data_handle);
#define	          gsl_data_handle_n_values(	     dh) \
						     gsl_data_handle_length (dh)
float             gsl_data_handle_peek_value        (GslDataHandle	*dhandle,
						     int64		 position,
						     GslDataPeekBuffer	*peekbuf);

/* --- decibel conversion --- */
gdouble bse_db_to_factor        (gdouble        dB);
gdouble bse_db_from_factor      (gdouble        factor,
                                 gdouble        min_dB);

int bse_fpu_okround();

/* Bse Block utils */

namespace Bse
{

class Block
{
public:
  static void  mul    (guint           n_values,
                       float          *ovalues,
                       const float    *ivalues);
  static void  add    (guint           n_values,
                       float          *ovalues,
                       const float    *ivalues);
  static void  range  (guint           n_values,
                       const float    *ivalues,
                       float&          min_value,
                       float&          max_value);
};

}

/* --- signal processing: windows --- */
double  bse_window_cos (double x);
double  bse_window_blackman (double x);

/* --- macros for frequency valued signals --- */
double BSE_SIGNAL_TO_FREQ (double sig);

/* --- gsl ffts --- */

void    gsl_power2_fftac (const uint         n_values,
                          const double      *ri_values_in,
                          double            *ri_values_out);
void    gsl_power2_fftsc (const uint         n_values,
                          const double      *ri_values_in,
                          double            *ri_values_out);
void    gsl_power2_fftar (const uint         n_values,
                          const double      *r_values_in,
                          double            *ri_values_out);
void    gsl_power2_fftsr (const unsigned int n_values,
                          const double      *ri_values_in,
                          double            *r_values_out);

/* --- sfi/bse init --- */
typedef struct
{
  const char *value_name;       /* value list ends with value_name == NULL */
  const char *value_string;
  long double value_num;        /* valid if value_string == NULL */
} SfiInitValue;

void bse_init_inprocess      (gint           *argc,
                              gchar         **argv,
                              const char     *app_name,
                              const std::vector<std::string>& args = std::vector<std::string>());

/* --- bse loader --- */
struct BseLoader;

struct BseWaveFileInfo {
  guint    n_waves;
  struct Wave {
    gchar *name;
  }       *waves;
  gchar  **comments;
  /*< private >*/
  gchar     *file_name;
  BseLoader *loader;
  guint      ref_count;
};
struct BseWaveChunkDsc;
struct BseWaveDsc
{
  gchar           *name;
  guint            n_chunks;
  BseWaveChunkDsc *chunks;
  guint            n_channels;
  gchar          **xinfos;
  /*< private >*/
  BseWaveFileInfo *file_info;
};
struct BseWaveChunkDsc
{
  gfloat          osc_freq;
  gfloat          mix_freq;
  gchar         **xinfos;
  /* loader-specific */
  union {
    guint         uint;
    gpointer      ptr;
    gfloat        vfloat;
  }               loader_data[8];
};

BseWaveFileInfo*      bse_wave_file_info_load   (const gchar     *file_name,
                                                 Bse::Error      *error);
BseWaveDsc*           bse_wave_dsc_load         (BseWaveFileInfo *wave_file_info,
                                                 guint            nth_wave,
                                                 gboolean         accept_empty,
                                                 Bse::Error      *error);
GslDataHandle*        bse_wave_handle_create    (BseWaveDsc      *wave_dsc,
                                                 guint            nth_chunk,
                                                 Bse::Error      *error);
#include "minipcg32.hh"

#endif
