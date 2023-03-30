// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smtimeinfo.hh"

using namespace SpectMorph;

/*---- time helper classes ----*/

void
TimeInfoGenerator::start_block (double time_ms, double ppq_pos)
{
  m_time_info.time_ms = time_ms;
  m_time_info.ppq_pos = ppq_pos;
}

void
TimeInfoGenerator::set_tempo (double tempo)
{
  m_tempo = tempo;
}

/* write new timestamp */
void
TimeInfoGenerator::set_time_ms (double time_ms)
{
  double delta_ms = time_ms - m_time_info.time_ms;

  m_time_info.time_ms = time_ms;
  m_time_info.ppq_pos += (delta_ms / 1000) * (m_tempo / 60);
}

/* read only accessor */
TimeInfo
TimeInfoGenerator::time_info (double offset_ms) const
{
  TimeInfo ti = m_time_info;

  ti.time_ms += offset_ms;
  ti.ppq_pos += (offset_ms / 1000) * (m_tempo / 60);

  return ti;
}

