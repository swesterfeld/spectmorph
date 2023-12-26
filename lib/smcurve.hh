// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "smoutfile.hh"
#include "sminfile.hh"

#include <cmath>

namespace SpectMorph
{

struct Curve
{
  /* grid/snap settings for UI */
  bool snap = true;
  int  grid_x = 4;
  int  grid_y = 4;
  /* curve points */
  struct Point
  {
    float x = 0;
    float y = 0;
    float slope = 0;
  };
  std::vector<Point> points;
  /* evaluate curve function */
  float
  operator() (float pos_x) const
  {
    if (points.empty())
      return 0;
    for (int i = 0; i < int (points.size()) - 1; i++)
      {
        if (points[i].x <= pos_x && points[i + 1].x >= pos_x)
          {
            float dx = points[i + 1].x - points[i].x;
            float frac = pos_x - points[i].x;
            if (dx > 1e-5)
              frac /= dx;
            else
              {
                /* avoid division by zero */
                frac = 0;
              }
            float slope = points[i].slope;

            if (slope > 0)
              frac = std::pow (frac, slope * 3 + 1);
            if (slope < 0)
              frac = 1 - std::pow ((1 - frac), -slope * 3 + 1);

            return points[i].y + frac * (points[i + 1].y - points[i].y);
          }
      }
    if (pos_x < points.front().x)
      return points.front().y;
    else
      return points.back().y;
  };
  void save (const std::string& identifier, OutFile& out_file);
  bool load (const std::string& identifier, InFile& in_file);
};

}
