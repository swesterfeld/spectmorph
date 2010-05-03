/* 
 * Copyright (C) 2009-2010 Stefan Westerfeld
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "smsinedecoder.hh"
#include <assert.h>
#include <stdio.h>
#include <math.h>

using SpectMorph::SineDecoder;
using SpectMorph::Frame;
using std::vector;

void
SineDecoder::process (Frame& frame,
                      Frame& next_frame,
		      const vector<double>& window)
{
  fill (frame.decoded_sines.begin(), frame.decoded_sines.end(), 0.0);

  /* phase synchronous reconstruction (no loops) */
  if (mode == MODE_PHASE_SYNC_OVERLAP)
    {
      for (size_t i = 0; i < frame.freqs.size(); i++)
        {
          const double SA = 0.5;
          const double smag = frame.phases[i * 2];
          const double cmag = frame.phases[i * 2 + 1];
          const double mag = sqrt (smag * smag + cmag * cmag) * SA;
          const double phase_delta = 2 * M_PI * frame.freqs[i] / mix_freq;

          double phase = atan2 (cmag, smag);
          for (size_t t = 0; t < frame_size; t++)
            {
	      frame.decoded_sines[t] += sin (phase) * mag;
              phase += phase_delta;
              while (phase > 2 * M_PI)
                phase -= 2 * M_PI;
            }
        }
      for (size_t t = 0; t < frame_size; t++)
        frame.decoded_sines[t] *= window[t];
      return;
    }

  /* phase distorted reconstruction */
  vector<double> freqs = frame.freqs;
  vector<double> nfreqs = next_frame.freqs;
  vector<double>::iterator phase_it = frame.phases.begin(), nphase_it = next_frame.phases.begin();

  int todo = freqs.size() + nfreqs.size();

  synth_fixed_phase = next_synth_fixed_phase;
  synth_fixed_phase.resize (freqs.size());
  next_synth_fixed_phase.resize (nfreqs.size());

  const double SIN_AMP = 1.0;
  const bool TRACKING_SYNTH = true;
  while (todo)
    {
      double best_delta = 1e10;
      int best_i, best_j;
      for (size_t i = 0; i < freqs.size(); i++)
	{
	  for (size_t j = 0; j < nfreqs.size(); j++)
	    {
	      double delta = fabs (freqs[i] - nfreqs[j]) / freqs[i];
	      if (freqs[i] >= 0 && nfreqs[j] >= 0 && delta < best_delta && delta < 0.1)
		{
		  best_delta = delta;
		  best_i = i;
		  best_j = j;
		}
	    }
	}
      if (best_delta < 0.1)
	{
	  double freq = freqs[best_i];
	  freqs[best_i] = -1;
	  double nfreq = nfreqs[best_j];
	  nfreqs[best_j] = -1;
	  double s_mag = *(phase_it + best_i * 2);
	  double c_mag = *(phase_it + best_i * 2 + 1);
	  double ns_mag = *(nphase_it + best_j * 2);
	  double nc_mag = *(nphase_it + best_j * 2 + 1);
	  double mag = sqrt (s_mag * s_mag + c_mag * c_mag);
	  double nmag = sqrt (ns_mag * ns_mag + nc_mag * nc_mag);

	  // fprintf (stderr, "%f | %f ==> %f | %f\n", freq, mag, nfreq, nmag);
	  assert (fabs (nfreq - freq) / freq < 0.1);

	  double phase_delta = 2 * M_PI * freq / mix_freq;
	  double nphase_delta = 2 * M_PI * nfreq / mix_freq;
	  double phase = synth_fixed_phase[best_i];
	  if (TRACKING_SYNTH)
	    {
	      for (size_t i = 0; i < frame_step; i++)
		{
		  double inter = i / double (frame_step);

		  frame.decoded_sines [i] += sin (phase) * ((1 - inter) * mag + inter * nmag) * SIN_AMP;
		  phase += (1 - inter) * phase_delta + inter * nphase_delta;
		  while (phase > 2 * M_PI)
		    phase -= 2 * M_PI;
		}
	      next_synth_fixed_phase[best_j] = phase;
	    }
	  else
	    {
	      for (size_t i = 0; i < frame_size; i++)
		{
		  frame.decoded_sines [i] += sin (phase) * window[i] * mag * SIN_AMP;
		  phase += phase_delta;
		  while (phase > 2 * M_PI)
		    phase -= 2 * M_PI;
		  // nfreq phase required -> ramp
		  if (i == frame_step - 1)
		    next_synth_fixed_phase[best_j] = phase;
		}
	    }
	  todo -= 2;
	}
      else
	{
	  for (size_t from = 0; from < freqs.size(); from++)
	    {
	      if (freqs[from] > -1)
		{
		  double freq = freqs[from];
		  freqs[from] = -1;
		  double s_mag = *(phase_it + from * 2);
		  double c_mag = *(phase_it + from * 2 + 1);
		  double mag = sqrt (s_mag * s_mag + c_mag * c_mag);

		  // fprintf (stderr, "%f | %f   >>> \n", freq, mag);

		  double phase_delta = 2 * M_PI * freq / mix_freq;
		  double phase = synth_fixed_phase[from];
		  if (TRACKING_SYNTH)
		    {
		      for (size_t i = 0; i < frame_step; i++)
			{
			  double inter = i / double (frame_step);

			  frame.decoded_sines [i] += sin (phase) * (1 - inter) * mag * SIN_AMP;
			  phase += phase_delta;
			  while (phase > 2 * M_PI)
			    phase -= 2 * M_PI;
			}
		    }
		  else
		    {
		      for (size_t i = 0; i < frame_size; i++)
			{
			  frame.decoded_sines [i] += sin (phase) * window[i] * mag * SIN_AMP;
			  phase += phase_delta;
			  while (phase > 2 * M_PI)
			    phase -= 2 * M_PI;
			}
		    }
		  todo--;
		}
	    }
	  for (int to = 0; to < nfreqs.size(); to++)
	    {
	      if (nfreqs[to] > -1)
		{
		  double freq = nfreqs[to];
		  nfreqs[to] = -1;
		  double s_mag = *(nphase_it + to * 2);
		  double c_mag = *(nphase_it + to * 2 + 1);
		  double mag = sqrt (s_mag * s_mag + c_mag * c_mag);

		  // fprintf (stderr, "%f | %f   <<< \n", freq, mag);

		  double phase_delta = 2 * M_PI * freq / mix_freq;
		  double phase = 0;
		  if (TRACKING_SYNTH)
		    {
		      for (size_t i = 0; i < frame_step; i++)
			{
			  double inter = i / double (frame_step);

			  frame.decoded_sines[i] += sin (phase) * inter * mag * SIN_AMP; /* XXX */
			  phase += phase_delta;
			  while (phase > 2 * M_PI)
			    phase -= 2 * M_PI;
			}
		      next_synth_fixed_phase[to] = phase;
		    }
		  todo--;
		}
	    }
	}
    }
}


