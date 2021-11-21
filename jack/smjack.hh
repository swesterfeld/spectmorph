// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SPECTMORPH_JACK_HH
#define SPECTMORPH_JACK_HH

#include "smmorphplanwindow.hh"
#include "smmorphplancontrol.hh"
#include "smmorphplansynth.hh"
#include "smmorphplanvoice.hh"
#include "smmidisynth.hh"
#include "smled.hh"

#include <jack/jack.h>

namespace SpectMorph
{

class JackSynth
{
protected:
  jack_client_t                *client;
  jack_port_t                  *input_port;
  std::vector<jack_port_t *>    output_ports;

  Project                      *m_project;

public:
  JackSynth (jack_client_t *client, Project *project);

  int  process (jack_nframes_t nframes);
};

}

#endif
