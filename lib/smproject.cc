// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smproject.hh"
#include "smmidisynth.hh"
#include "smsynthinterface.hh"
#include "smmorphoutputmodule.hh"
#include <unistd.h>

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
ControlEventVector::run_rt (Project *project)
{
  if (!clear)
    {
      for (const auto& ev : events)
        ev->run_rt (project);

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
      m_control_events.run_rt (this);
      m_out_events = m_midi_synth->inst_edit_synth()->take_out_events();
      m_voices_active = m_midi_synth->active_voice_count() > 0;

      m_synth_mutex.unlock();
    }
}

void
Project::synth_take_control_event (SynthControlEvent *event)
{
  std::lock_guard<std::mutex> lg (m_synth_mutex);
  m_control_events.take (event);
}

class SpectMorph::Job
{
  Project                       *project;
  std::unique_ptr<WavSetBuilder> builder;
  int                            inst_id;
public:
  Job (Project *project, WavSetBuilder *builder, int inst_id) :
    project (project),
    builder (builder),
    inst_id (inst_id)
  {
  }
  void run()
  {
    printf ("start iid=%d\n", inst_id);

    WavSet *wav_set = builder->run();

    project->synth_interface()->emit_add_rebuild_result (inst_id, wav_set);

    printf ("done iid=%d\n", inst_id);
  }
};

void
Project::rebuild (int inst_id)
{
  Instrument *instrument = instrument_map[inst_id].get();

  if (!instrument)
    return;

  WavSetBuilder *builder = new WavSetBuilder (instrument, /* keep_samples */ false);

  builder->set_kill_function ([this]() {
    std::lock_guard<std::mutex> lg (instrument_worker_mutex);
    return instrument_worker_quit;
  });
  instrument_worker_mutex.lock();
  instrument_worker_todo.emplace_back (new Job (this, builder, inst_id));
  instrument_worker_mutex.unlock();
}

void
Project::add_rebuild_result (int inst_id, WavSet *wav_set)
{
  size_t s = inst_id + 1;
  if (s > wav_sets.size())
    wav_sets.resize (s);

  wav_sets[inst_id] = std::shared_ptr<WavSet> (wav_set);
}

int
Project::add_instrument()
{
  int inst_id = 1;

  while (instrument_map[inst_id].get()) /* find first free slot */
    inst_id++;

  instrument_map[inst_id].reset (new Instrument());
  return inst_id;
}

Instrument *
Project::get_instrument (int inst_id)
{
  return instrument_map[inst_id].get();
}

std::shared_ptr<WavSet>
Project::get_wav_set (int inst_id)
{
  if (size_t (inst_id) < wav_sets.size())
    return wav_sets[inst_id];
  else
    return nullptr;
}

vector<string>
Project::notify_take_events()
{
  std::lock_guard<std::mutex> lg (m_synth_mutex);
  return std::move (m_out_events);
}

SynthInterface *
Project::synth_interface() const
{
  return m_synth_interface.get();
}

MidiSynth *
Project::midi_synth() const
{
  return m_midi_synth.get();
}

void
Project::thread_main()
{
  printf ("start\n");
  bool quit;
  while (!quit)
    {
      // FIXME: sleep on condition instead
      usleep (100 * 1000);

      instrument_worker_mutex.lock();
      quit = instrument_worker_quit;
      if (!quit && instrument_worker_todo.size())
        {
          Job *job = instrument_worker_todo[0].get();
          instrument_worker_mutex.unlock();
          job->run();
          instrument_worker_mutex.lock();
          instrument_worker_todo.erase (instrument_worker_todo.begin());
        }
      instrument_worker_mutex.unlock();
    }
  printf ("end\n");
}

Project::Project()
  : instrument_worker (std::thread (&Project::thread_main, this))
{
  m_morph_plan = new MorphPlan (*this);
  m_morph_plan->load_default();

  connect (m_morph_plan->signal_plan_changed, this, &Project::on_plan_changed);

  m_synth_interface.reset (new SynthInterface (this));
}

Project::~Project()
{
  instrument_worker_mutex.lock();
  instrument_worker_quit = true;
  instrument_worker_mutex.unlock();
  instrument_worker.join();
}

void
Project::set_mix_freq (double mix_freq)
{
  // not rt safe, needs to be called when synthesis thread is not running
  m_midi_synth.reset (new MidiSynth (mix_freq, 64));
  m_mix_freq = mix_freq;

  // FIXME: can this cause problems if an old plan change control event remained
  m_midi_synth->update_plan (m_morph_plan->clone());
  m_midi_synth->set_gain (db_to_factor (m_volume));
}

void
Project::on_plan_changed()
{
  MorphPlanPtr plan = m_morph_plan->clone();

  // this might take a while, and cannot be done in synthesis thread
  MorphPlanSynth mp_synth (m_mix_freq);
  MorphPlanVoice *mp_voice = mp_synth.add_voice();
  mp_synth.update_plan (plan);

  MorphOutputModule *om = mp_voice->output();
  if (om)
    {
      om->retrigger (0, 440, 1);
      float s;
      float *values[1] = { &s };
      om->process (1, values, 1);
    }

  // FIXME: refptr is locking (which is not too good)
  m_synth_interface->emit_update_plan (plan);
}

bool
Project::voices_active()
{
  std::lock_guard<std::mutex> lg (m_synth_mutex);
  return m_voices_active;
}

MorphPlanPtr
Project::morph_plan() const
{
  return m_morph_plan;
}

double
Project::volume() const
{
  return m_volume;
}

void
Project::set_volume (double volume)
{
  m_volume = volume;
  m_synth_interface->emit_update_gain (db_to_factor (m_volume));

  signal_volume_changed (m_volume);
}
