// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smtimeinfo.hh"

using namespace SpectMorph;

/*---- time helper classes ----
 *
 * These classes provide a monotonic clock for TimeInfo: time_ms and ppq_pos
 *
 * For time_ms this is relatively simple, because we maintain the time stamp in
 * samples that is used to compute the time_ms field.
 *
 * For ppq_pos, we need to use the song tempo to be able to compute the
 * position inside a block, which can cause problems for tempo changes.
 * So we need to constrain ppq_pos using the value from the last block.
 */

TimeInfoGenerator::TimeInfoGenerator (double mix_freq) :
  m_mix_freq (mix_freq)
{
}

void
TimeInfoGenerator::start_block (uint64 audio_time_stamp, uint n_samples, double ppq_pos, double tempo)
{
  /* detect backwards jumps; in this case our ppq pos should also not be monotonic */
  if (ppq_pos < m_last_block_ppq_pos)
    m_max_ppq_pos = ppq_pos;
  m_last_block_ppq_pos = ppq_pos;

  m_audio_time_stamp = audio_time_stamp;
  m_n_samples = n_samples;
  m_ppq_pos = ppq_pos;
  m_tempo = tempo;

  double block_size_ms = 0;
  if (m_n_samples > 0)
    block_size_ms = (m_n_samples - 1) / m_mix_freq * 1000;

  m_max_ppq_pos = std::max (m_ppq_pos + (block_size_ms / 1000) * (m_tempo / 60), m_max_ppq_pos);
  m_max_time_ms = m_audio_time_stamp / m_mix_freq * 1000 + block_size_ms;
}

void
TimeInfoGenerator::update_time_stamp (uint64 audio_time_stamp)
{
  uint delta_samples = audio_time_stamp - m_audio_time_stamp;

  m_audio_time_stamp += delta_samples;
  m_n_samples -= delta_samples;

  double delta_ms = delta_samples / m_mix_freq * 1000;
  m_ppq_pos += (delta_ms / 1000) * (m_tempo / 60);
}

/* read only accessor */
TimeInfo
TimeInfoGenerator::time_info (double offset_ms) const
{
  TimeInfo ti;

  ti.time_ms = m_audio_time_stamp / m_mix_freq * 1000;
  ti.time_ms += offset_ms;
  ti.time_ms = std::min (ti.time_ms, m_max_time_ms);

  ti.ppq_pos = m_ppq_pos;
  ti.ppq_pos += (offset_ms / 1000) * (m_tempo / 60);
  ti.ppq_pos = std::min (ti.ppq_pos, m_max_ppq_pos);

  return ti;
}
