// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_NO_BSE_HH
#define SPECTMORPH_NO_BSE_HH

#include <glib.h>
#include <string>

/* Rapicorn fake */

typedef long long           int64;
typedef unsigned long long  uint64;
typedef guint               uint;
typedef guint8              uint8;

/* Birnet */
#define BIRNET_PRIVATE_CLASS_COPY(Class)        private: Class (const Class&); Class& operator= (const Class&);
#define BIRNET_PRINTF(format_idx, arg_idx)      __attribute__ ((__format__ (__printf__, format_idx, arg_idx)))

namespace Birnet
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

/* Bse Error Type */
typedef enum
{
  BSE_ERROR_NONE		= 0,
  BSE_ERROR_INTERNAL,
  BSE_ERROR_UNKNOWN,
  /* general errors */
  BSE_ERROR_IO,
  BSE_ERROR_PERMS,
  /* file errors */
  BSE_ERROR_FILE_BUSY,
  BSE_ERROR_FILE_EXISTS,
  BSE_ERROR_FILE_EOF,
  BSE_ERROR_FILE_EMPTY,
  BSE_ERROR_FILE_NOT_FOUND,
  BSE_ERROR_FILE_IS_DIR,
  BSE_ERROR_FILE_OPEN_FAILED,
  BSE_ERROR_FILE_SEEK_FAILED,
  BSE_ERROR_FILE_READ_FAILED,
  BSE_ERROR_FILE_WRITE_FAILED,
  /* out of resource conditions */
  BSE_ERROR_MANY_FILES,
  BSE_ERROR_NO_FILES,
  BSE_ERROR_NO_SPACE,
  BSE_ERROR_NO_MEMORY,
  /* content errors */
  BSE_ERROR_NO_HEADER,
  BSE_ERROR_NO_SEEK_INFO,
  BSE_ERROR_NO_DATA,
  BSE_ERROR_DATA_CORRUPT,
  BSE_ERROR_WRONG_N_CHANNELS,
  BSE_ERROR_FORMAT_INVALID,
  BSE_ERROR_FORMAT_UNKNOWN,
  BSE_ERROR_DATA_UNMATCHED,
  /* miscellaneous errors */
  BSE_ERROR_TEMP,
  BSE_ERROR_WAVE_NOT_FOUND,
  BSE_ERROR_CODEC_FAILURE,
  BSE_ERROR_UNIMPLEMENTED,
  BSE_ERROR_INVALID_PROPERTY,
  BSE_ERROR_INVALID_MIDI_CONTROL,
  BSE_ERROR_PARSE_ERROR,
  BSE_ERROR_SPAWN,
  /* Device errors */
  BSE_ERROR_DEVICE_NOT_AVAILABLE,
  BSE_ERROR_DEVICE_ASYNC,
  BSE_ERROR_DEVICE_BUSY,
  BSE_ERROR_DEVICE_FORMAT,
  BSE_ERROR_DEVICE_BUFFER,
  BSE_ERROR_DEVICE_LATENCY,
  BSE_ERROR_DEVICE_CHANNELS,
  BSE_ERROR_DEVICE_FREQUENCY,
  BSE_ERROR_DEVICES_MISMATCH,
  /* BseSource errors */
  BSE_ERROR_SOURCE_NO_SUCH_MODULE,
  BSE_ERROR_SOURCE_NO_SUCH_ICHANNEL,
  BSE_ERROR_SOURCE_NO_SUCH_OCHANNEL,
  BSE_ERROR_SOURCE_NO_SUCH_CONNECTION,
  BSE_ERROR_SOURCE_PRIVATE_ICHANNEL,
  BSE_ERROR_SOURCE_ICHANNEL_IN_USE,
  BSE_ERROR_SOURCE_CHANNELS_CONNECTED,
  BSE_ERROR_SOURCE_CONNECTION_INVALID,
  BSE_ERROR_SOURCE_PARENT_MISMATCH,
  BSE_ERROR_SOURCE_BAD_LOOPBACK,
  BSE_ERROR_SOURCE_BUSY,
  BSE_ERROR_SOURCE_TYPE_INVALID,
  /* BseProcedure errors */
  BSE_ERROR_PROC_NOT_FOUND,
  BSE_ERROR_PROC_BUSY,
  BSE_ERROR_PROC_PARAM_INVAL,
  BSE_ERROR_PROC_EXECUTION,
  BSE_ERROR_PROC_ABORT,
  /* various procedure errors */
  BSE_ERROR_NO_ENTRY,
  BSE_ERROR_NO_EVENT,
  BSE_ERROR_NO_TARGET,
  BSE_ERROR_NOT_OWNER,
  BSE_ERROR_INVALID_OFFSET,
  BSE_ERROR_INVALID_DURATION,
  BSE_ERROR_INVALID_OVERLAP,
} BseErrorType;

const gchar* bse_error_blurb (BseErrorType error_value);

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
  BseErrorType	 (*open)		(GslDataHandle		*data_handle,
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
BseErrorType	  gsl_data_handle_open		    (GslDataHandle	  *dhandle);
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
                              gchar        ***argv,
                              const char     *app_name,
                              SfiInitValue    values[]);

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
                                                 BseErrorType    *error);
BseWaveDsc*           bse_wave_dsc_load         (BseWaveFileInfo *wave_file_info,
                                                 guint            nth_wave,
                                                 gboolean         accept_empty,
                                                 BseErrorType    *error);
GslDataHandle*        bse_wave_handle_create    (BseWaveDsc      *wave_dsc,
                                                 guint            nth_chunk,
                                                 BseErrorType    *error);

#endif
