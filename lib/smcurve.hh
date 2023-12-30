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

  enum class Loop {
    NONE = 0,
    SUSTAIN = 1,
    FORWARD = 2,
    PING_PONG = 3
  };
  Loop loop = Loop::NONE;
  int loop_start = 1;
  int loop_end = 1;

  /* evaluate curve function between two points */
  static float
  evaluate2 (float pos_x, const Point& p1, const Point& p2)
  {
    float dx = p2.x - p1.x;
    float frac = pos_x - p1.x;
    if (dx > 1e-5)
      {
        frac /= dx;
        if (frac > 1)
          frac = 1;
      }
    else
      {
        /* avoid division by zero */
        frac = 0;
      }
    float slope = p1.slope;

    if (slope > 0)
      frac = std::pow (frac, slope * 3 + 1);
    if (slope < 0)
      frac = 1 - std::pow ((1 - frac), -slope * 3 + 1);

    return p1.y + frac * (p2.y - p1.y);
  }
  /* evaluate curve function */
  float
  operator() (float pos_x) const
  {
    if (points.empty())
      return 0;
    for (int i = 0; i < int (points.size()) - 1; i++)
      {
        if (points[i].x <= pos_x && points[i + 1].x >= pos_x)
          return evaluate2 (pos_x, points[i], points[i + 1]);
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
