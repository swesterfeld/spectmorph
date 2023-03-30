// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

namespace SpectMorph
{

class TimeInfo
{
public:
  double time_ms = 0;
  double ppq_pos = 0;
};

class TimeInfoGenerator
{
  TimeInfo m_time_info;
  double   m_tempo = 120;
public:
  void start_block (double time_ms, double ppq_pos);
  void set_time_ms (double time_ms);
  void set_tempo (double tempo);

  TimeInfo time_info (double offset_ms) const;
};

}
