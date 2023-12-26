// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "smoutfile.hh"
#include "sminfile.hh"

#include <cmath>

namespace SpectMorph
{

struct Curve
{
  struct Point
  {
    float x = 0;
    float y = 0;
    float slope = 0;
  };
  std::vector<Point> points;
  float
  operator() (float pos_x) const
  {
    for (size_t i = 0; i < points.size() - 1; i++)
      {
        if (points[i + 1].x >= pos_x)
          {
            float frac = (pos_x - points[i].x) / (points[i + 1].x - points[i].x);
            float slope = points[i].slope;

            if (slope > 0)
              frac = std::pow (frac, slope * 3 + 1);
            if (slope < 0)
              frac = 1 - std::pow ((1 - frac), -slope * 3 + 1);

            return points[i].y + frac * (points[i + 1].y - points[i].y);
          }
      }
    return 0; // FIXME: not correct
  };
  void save (const std::string& identifier, OutFile& out_file);
  bool load (const std::string& identifier, InFile& in_file);
};

}
