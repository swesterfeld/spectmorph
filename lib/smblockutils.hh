// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_BLOCK_UTILS_HH
#define SPECTMORPH_BLOCK_UTILS_HH

#include <glib.h>

namespace SpectMorph
{

/* Block utils */

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

#endif /* SPECTMORPH_BLOCK_UTILS_HH */
