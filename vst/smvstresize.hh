// Code from JUCE4 - Licensed GNU GPL v2 or later
/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

#include "smutils.hh"

#ifndef SM_OS_WINDOWS
static inline void
vst_manual_resize (SpectMorph::Window *window, int newWidth, int newHeight)
{
  /* dummy implementation */
}
#else
#include "windows.h"

using std::string;

// Returns the actual container window, unlike GetParent, which can also return a separate owner window.
static inline HWND
getWindowParent (HWND w)
{
  return GetAncestor (w, GA_PARENT);
}

static inline bool
string_eq_ignore_case (const string& a, const string &b)
{
  size_t sz = a.size();

  if (b.size() != sz)
    return false;

  for (size_t i = 0; i < sz; ++i)
    if (tolower (a[i]) != tolower (b[i]))
      return false;

  return true;
}

static inline void
vst_manual_resize (SpectMorph::Window *window, int newWidth, int newHeight)
{
  // some hosts don't support the sizeWindow call, so do it manually..
  int dw = 0;
  int dh = 0;
  const int frameThickness = GetSystemMetrics (SM_CYFIXEDFRAME);

  HWND w = (HWND) window->native_window();

  while (w != 0)
    {
      HWND parent = getWindowParent (w);

      if (parent == 0)
        break;

      TCHAR windowType [32] = { 0 };
      GetClassName (parent, windowType, 31);

      if (string_eq_ignore_case (windowType, "MDIClient"))
        break;

      RECT windowPos, parentPos;
      GetWindowRect (w, &windowPos);
      GetWindowRect (parent, &parentPos);

      SetWindowPos (w, 0, 0, 0, newWidth + dw, newHeight + dh,
                    SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOOWNERZORDER);

      dw = (parentPos.right - parentPos.left) - (windowPos.right - windowPos.left);
      dh = (parentPos.bottom - parentPos.top) - (windowPos.bottom - windowPos.top);

      w = parent;

      if (dw == 2 * frameThickness)
        break;

      if (dw > 100 || dh > 100)
        w = 0;
    }

  if (w != 0)
    SetWindowPos (w, 0, 0, 0, newWidth + dw, newHeight + dh,
        SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOOWNERZORDER);
}
#endif
