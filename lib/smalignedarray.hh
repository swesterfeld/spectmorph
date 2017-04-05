// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_ALIGNED_ARRAY_HH
#define SPECTMORPH_ALIGNED_ARRAY_HH

#include <glib.h>
#include "smutils.hh"

namespace SpectMorph
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

#endif /* SPECTMORPH_ALIGNED_ARRAY_HH */
