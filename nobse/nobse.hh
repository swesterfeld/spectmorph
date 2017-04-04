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
  size_t size () const
  {
    return n_elements;
  }
};

}

#define SPECTMORPH_NOBSE 1

/* --- Bse IEEE754 --- */
#if defined (__i386__) && defined (__GNUC__)
static inline int G_GNUC_CONST
bse_ftoi (register float f)
{
  int r;

  __asm__ ("fistl %0"
           : "=m" (r)
           : "t" (f));
  return r;
}
static inline int G_GNUC_CONST
bse_dtoi (register double f)
{
  int r;

  __asm__ ("fistl %0"
           : "=m" (r)
           : "t" (f));
  return r;
}
#endif

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

/* --- macros for frequency valued signals --- */
double BSE_SIGNAL_TO_FREQ (double sig);

void bse_init_inprocess      (gint           *argc,
                              gchar         **argv,
                              const char     *app_name,
                              const std::vector<std::string>& args = std::vector<std::string>());

#include "minipcg32.hh"

#endif
