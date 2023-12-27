// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smcurve.hh"
#include "smoutfile.hh"
#include "sminfile.hh"

using namespace SpectMorph;

void
Curve::save (const std::string& identifier, OutFile& out_file)
{
  std::vector<float> xs, ys, slopes;
  for (auto p : points)
    {
      xs.push_back (p.x);
      ys.push_back (p.y);
      slopes.push_back (p.slope);
    }
  out_file.write_bool (identifier + ".snap", snap);
  out_file.write_int (identifier + ".grid_x", grid_x);
  out_file.write_int (identifier + ".grid_y", grid_y);
  out_file.write_int (identifier + ".loop", int (loop));
  out_file.write_int (identifier + ".loop_start", loop_start);
  out_file.write_int (identifier + ".loop_end", loop_end);
  out_file.write_float_block (identifier + ".xs", xs);
  out_file.write_float_block (identifier + ".ys", ys);
  out_file.write_float_block (identifier + ".slopes", slopes);
}

bool
Curve::load (const std::string& identifier, InFile& in_file)
{
  if (in_file.event() == InFile::BOOL)
    {
      if (in_file.event_name() == identifier + ".snap")
        {
          snap = in_file.event_bool();
        }
      else
        {
          return false;
        }
    }
  else if (in_file.event() == InFile::INT)
    {
      if (in_file.event_name() == identifier + ".grid_x")
        {
          grid_x = in_file.event_int();
        }
      else if (in_file.event_name() == identifier + ".grid_y")
        {
          grid_y = in_file.event_int();
        }
      else if (in_file.event_name() == identifier + ".loop")
        {
          loop = Loop (in_file.event_int());
        }
      else if (in_file.event_name() == identifier + ".loop_start")
        {
          loop_start = in_file.event_int();
        }
      else if (in_file.event_name() == identifier + ".loop_end")
        {
          loop_end = in_file.event_int();
        }
      else
        {
          return false;
        }
    }
  else if (in_file.event() == InFile::FLOAT_BLOCK)
    {
      const std::vector<float>& v = in_file.event_float_block();
      if (in_file.event_name() == identifier + ".xs")
        {
          points.resize (v.size());

          for (size_t i = 0; i < v.size(); i++)
            points[i].x = v[i];
        }
      else if (in_file.event_name() == identifier + ".ys")
        {
          if (points.size() != v.size())
            {
              g_printerr ("inconsistent size loading curve %s.ys: %zd vs. %zd\n", identifier.c_str(), points.size(), v.size());
              return false;
            }

          for (size_t i = 0; i < v.size(); i++)
            points[i].y = v[i];
        }
      else if (in_file.event_name() == identifier + ".slopes")
        {
          if (points.size() != v.size())
            {
              g_printerr ("inconsistent size loading curve %s.slopes: %zd vs. %zd\n", identifier.c_str(), points.size(), v.size());
              return false;
            }

          for (size_t i = 0; i < v.size(); i++)
            points[i].slope = v[i];
        }
      else
        {
          return false;
        }
    }
  else
    {
      return false;
    }
  return true;
}
