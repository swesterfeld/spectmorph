// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smproject.hh"
#include "smmidisynth.hh"

using namespace SpectMorph;

using std::string;
using std::vector;

void
ControlEventVector::take (SynthControlEvent *ev)
{
  // we'd rather run destructors in non-rt part of the code
  if (clear)
    {
      events.clear();
      clear = false;
    }

  events.emplace_back (ev);
}

void
ControlEventVector::run_rt (MidiSynth *midi_synth)
{
  if (!clear)
    {
      for (const auto& ev : events)
        ev->run_rt (midi_synth);

      clear = true;
    }
}

void
Project::try_update_synth()
{
  // handle synth updates (if locking is possible without blocking)
  //  - apply new parameters
  //  - process events
  if (m_synth_mutex.try_lock())
    {
      m_control_events.run_rt (m_midi_synth);
      m_out_events = m_midi_synth->inst_edit_synth()->take_out_events();

      m_synth_mutex.unlock();
    }
}

void
Project::synth_take_control_event (SynthControlEvent *event)
{
  std::lock_guard<std::mutex> lg (m_synth_mutex);
  m_control_events.take (event);
}

void
Project::rebuild()
{
  WavSetBuilder *builder = new WavSetBuilder (&instrument, /* keep_samples */ false);

  new std::thread ([this, builder]() {
    struct Event : public SynthControlEvent {
      std::shared_ptr<WavSet> wav_set;
      Project *project;

      void
      run_rt (SpectMorph::MidiSynth*)
      {
        project->wav_set = wav_set;
      }
    } *event = new Event();

    event->wav_set = std::shared_ptr<WavSet> (builder->run());
    event->project = this;
    delete builder;

    synth_take_control_event (event);
  });
}

vector<string>
Project::notify_take_events()
{
  std::lock_guard<std::mutex> lg (m_synth_mutex);
  return std::move (m_out_events);
}
