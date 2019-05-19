// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_PROJECT_HH
#define SPECTMORPH_PROJECT_HH

#include "sminstrument.hh"
#include "smwavset.hh"
#include "smwavsetbuilder.hh"
#include "smobject.hh"
#include "smbuilderthread.hh"
#include "smmorphplan.hh"
#include "smuserinstrumentindex.hh"

#include <thread>
#include <mutex>

namespace SpectMorph
{

class MidiSynth;
class SynthInterface;
class MorphWavSource;

class SynthControlEvent
{
public:
  virtual void run_rt (Project *project) = 0;
  virtual
  ~SynthControlEvent()
  {
  }
};

struct InstFunc : public SynthControlEvent
{
  std::function<void(Project *)> func;
  std::function<void()>          free_func;
public:
  InstFunc (const std::function<void(Project *)>& func,
            const std::function<void()>& free_func) :
    func (func),
    free_func (free_func)
  {
  }
  ~InstFunc()
  {
    free_func();
  }
  void
  run_rt (Project *project)
  {
    func (project);
  }
};

class ControlEventVector
{
  std::vector<std::unique_ptr<SynthControlEvent>> events;
  bool clear = false;
public:
  void take (SynthControlEvent *ev);
  void run_rt (Project *project);
};

class Project : public SignalReceiver
{
  std::vector<std::shared_ptr<WavSet>> wav_sets;

  std::unique_ptr<MidiSynth>  m_midi_synth;
  double                      m_mix_freq = 0;
  double                      m_volume = -6;
  RefPtr<MorphPlan>           m_morph_plan;

  std::mutex                  m_synth_mutex;
  ControlEventVector          m_control_events;          // protected by synth mutex
  std::vector<std::string>    m_out_events;              // protected by synth mutex
  bool                        m_voices_active = false;   // protected by synth mutex

  std::unique_ptr<SynthInterface> m_synth_interface;

  UserInstrumentIndex         m_user_instrument_index;
  BuilderThread               m_builder_thread;

  std::map<int, std::unique_ptr<Instrument>> instrument_map;

  std::vector<MorphWavSource *> list_wav_sources();

  void on_plan_changed();

public:
  Project();

  void rebuild (int inst_id);
  void add_rebuild_result (int inst_id, WavSet *wav_set);

  Instrument *get_instrument (MorphWavSource *wav_source);

  std::shared_ptr<WavSet> get_wav_set (int inst_id);

  void synth_take_control_event (SynthControlEvent *event);

  std::mutex&
  synth_mutex()
  {
    /* the synthesis thread will typically not block on synth_mutex
     * instead, it tries locking it, and if that fails, continues
     *
     * the ui thread will block on the synth_mutex to enqueue events,
     * parameter changes (in form of a new morph plan, volume, ...)
     * and so on
     *
     * if the synthesis thread can obtain a lock, it will then be
     * able to process these events to update its internal state
     * and also send notifications back to the ui
     */
    return m_synth_mutex;
  }
  void try_update_synth();
  void set_mix_freq (double mix_freq);
  bool voices_active();

  void set_volume (double new_volume);
  double volume() const;

  std::vector<std::string> notify_take_events();
  SynthInterface *synth_interface() const;
  MidiSynth *midi_synth() const;
  MorphPlanPtr morph_plan() const;
  UserInstrumentIndex *user_instrument_index();

  Error save (const std::string& filename);
  Error save (ZipWriter& zip_writer, MorphPlan::ExtraParameters *params);
  Error load (const std::string& filename);
  Error load (ZipReader& zip_reader, MorphPlan::ExtraParameters *params);
  Error load_compat (GenericIn *in, MorphPlan::ExtraParameters *params);

  std::string save_plan_lv2 (std::function<std::string(std::string)> abstract_path);
  void        load_plan_lv2 (std::function<std::string(std::string)> absolute_path, const std::string& plan);

  Signal<double> signal_volume_changed;
};

}

#endif
