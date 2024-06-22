// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smproject.hh"
#include "smmidisynth.hh"
#include "smsynthinterface.hh"
#include "smmorphoutputmodule.hh"
#include "smzip.hh"
#include "smmemout.hh"
#include "smmorphwavsource.hh"
#include "smuserinstrumentindex.hh"
#include "smproject.hh"
#include "smhexstring.hh"

#include <unistd.h>

#include <filesystem>

using namespace SpectMorph;

using std::string;
using std::vector;
using std::set;
using std::map;

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
ControlEventVector::destroy_all_events()
{
  events.clear();
  clear = false;
}

/*
 * do not use a std::mutex here because it may not be hard RT safe to
 * try_lock() / unlock() it (depending on how the mutex is implemented)
 */
bool
ControlEventVector::try_lock()
{
  return !locked_flag.test_and_set();
}

void
ControlEventVector::unlock()
{
  locked_flag.clear();
}

bool
Project::try_update_synth()
{
  bool state_changed = false;
  // handle synth updates (if locking is possible without blocking)
  //  - apply new parameters
  //  - process events
  if (m_control_events.try_lock())
    {
      m_control_events.run_rt (this);
      state_changed = m_state_changed;
      m_state_changed = false;

      m_control_events.unlock();
    }
  return state_changed;
}

void
Project::synth_take_control_event (SynthControlEvent *event)
{
  while (!m_control_events.try_lock())
    {
      // this doesn't happen very often and we are in the non-RT thread, so we
      // can block it for some time
      //  => wait for less than one frame drawing time until trying again
      float fps = 240;
      usleep (1000 * 1000 / fps);
    }
  m_control_events.take (event);
  m_control_events.unlock();
}

void
Project::rebuild (MorphWavSource *wav_source)
{
  const int   object_id  = wav_source->object_id();
  Instrument *instrument = m_instrument_map[object_id].instrument.get();

  if (!instrument)
    return;

  WavSetBuilder *builder = new WavSetBuilder (instrument, /* keep_samples */ false);
  m_builder_thread.kill_jobs_by_id (object_id);
  synth_interface()->emit_add_rebuild_result (object_id, nullptr);
  // trigger configuration update, this will ensure that the modules pick up
  // the nullptr from the project, so that they will stop playing and not
  // access the old WavSet anymore
  m_morph_plan.emit_plan_changed();
  m_builder_thread.add_job (builder, object_id,
    [this, object_id] (WavSet *wav_set)
      {
        synth_interface()->emit_add_rebuild_result (object_id, wav_set);
      });
}

bool
Project::rebuild_active (int object_id)
{
  if (object_id == 0)
    fprintf (stderr, "Project::rebuild_active (object_id = 0)\n");
  return m_builder_thread.search_job (object_id);
}

void
Project::add_rebuild_result (int object_id, std::unique_ptr<WavSet>& wav_set)
{
  // this function runs in audio thread
  size_t s = object_id + 1;
  if (s > wav_sets.size())
    wav_sets.resize (s);

  wav_sets[object_id].swap (wav_set);
}

void
Project::clear_wav_sets (vector<std::unique_ptr<WavSet>>& new_wav_sets)
{
  // this function runs in audio thread
  wav_sets.swap (new_wav_sets);
}

Project::InstrumentMapEntry&
Project::lookup_instrument (MorphWavSource *wav_source)
{
  if (wav_source->object_id() == 0) /* create if not used */
    {
      /* check which object ids are currently used */
      set<int> used_object_ids;
      for (auto wav_source : list_wav_sources())
        {
          const int object_id = wav_source->object_id();

          if (object_id)
            {
              assert (m_instrument_map[object_id].instrument); /* can only be used if it has a map entry */

              used_object_ids.insert (object_id);
            }
        }
      int object_id = 1;

      while (used_object_ids.count (object_id)) /* find first free slot */
        object_id++;

      wav_source->set_object_id (object_id);
      m_instrument_map[object_id].instrument.reset (new Instrument());
      m_instrument_map[object_id].lv2_absolute_path = "";
    }

  return m_instrument_map[wav_source->object_id()];
}

WavSet*
Project::get_wav_set (int object_id)
{
  if (size_t (object_id) < wav_sets.size())
    return wav_sets[object_id].get();
  else
    return nullptr;
}

NotifyBuffer *
Project::notify_buffer()
{
  return m_midi_synth->notify_buffer();
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

Project::Project() :
  m_morph_plan (*this)
{
  m_morph_plan.load_default();

  connect (m_morph_plan.signal_plan_changed, this, &Project::on_plan_changed);
  connect (m_morph_plan.signal_operator_added, this, &Project::on_operator_added);
  connect (m_morph_plan.signal_operator_removed, this, &Project::on_operator_removed);

  m_synth_interface.reset (new SynthInterface (this));

  /* avoid malloc in audio threads if wav sets are added */
  wav_sets.reserve (Project::WAV_SETS_RESERVE);
}

void
Project::set_mix_freq (double mix_freq)
{
  /* if there are old control events, these cannot be executed anymore because we're
   * deleting the MidiSynth they refer to; we don't need a lock here because
   * this function is not supposed to be running while the audio thread is active
   */
  m_control_events.destroy_all_events();

  // not rt safe, needs to be called when synthesis thread is not running
  m_midi_synth.reset (new MidiSynth (mix_freq, 64));
  m_mix_freq = mix_freq;
  m_midi_synth->set_random_seed (m_random_seed);

  // not rt safe either
  LiveDecoder::precompute_tables (mix_freq);

  auto update = m_midi_synth->prepare_update (m_morph_plan);
  m_midi_synth->apply_update (update);
  m_midi_synth->set_gain (db_to_factor (m_volume));
}

void
Project::set_storage_model (StorageModel model)
{
  m_storage_model = model;
}

void
Project::set_state_changed_notify (bool notify)
{
  m_state_changed_notify = notify;
}

void
Project::state_changed()
{
  if (m_state_changed_notify)
    m_state_changed = true;
}

void
Project::on_plan_changed()
{
  /* FIXME: CONFIG
   *
   * we used to save here anyway, so state changed check could be done easily
   * however now with fast config updates, we just save for state changed
   * checks which is a waste of time
   */
  // create a deep copy (by saving/loading)
  vector<unsigned char> plan_data;
  m_morph_plan.save (MemOut::open (&plan_data));

  if (plan_data != m_last_plan_data)
    {
      m_last_plan_data = plan_data;
      state_changed();
    }

  MorphPlanSynth::UpdateP update = m_midi_synth->prepare_update (m_morph_plan);
  m_synth_interface->emit_apply_update (update);
}

void
Project::on_operator_added (MorphOperator *op)
{
  string type = op->type();

  if (type == "SpectMorph::MorphWavSource")
    {
      MorphWavSource *wav_source = static_cast<MorphWavSource *> (op);

      /*
       * only MorphWavSource objects which have been newly created using the UI have object_id == 0
       */
      if (wav_source->object_id() == 0)
        {
          /* load default instrument */
          auto& map_entry = lookup_instrument (wav_source);

          string filename = m_user_instrument_index.filename (wav_source->bank(), wav_source->instrument());
          Error error = map_entry.instrument->load (filename);
          if (!error)
            map_entry.lv2_absolute_path = filename;

          rebuild (wav_source);
        }
    }
}

void
Project::on_operator_removed (MorphOperator *op)
{
  string type = op->type();

  if (type == "SpectMorph::MorphWavSource")
    {
      MorphWavSource *wav_source = static_cast<MorphWavSource *> (op);

      const int object_id = wav_source->object_id();
      if (object_id)
        {
          /* free instrument data */
          m_instrument_map[object_id].instrument.reset (nullptr);
          m_instrument_map[object_id].lv2_absolute_path = "";

          /* stop rebuild jobs (if any) */
          m_builder_thread.kill_jobs_by_id (object_id);
          synth_interface()->emit_add_rebuild_result (object_id, nullptr);
        }
    }
}

MorphPlan *
Project::morph_plan()
{
  return &m_morph_plan;
}

UserInstrumentIndex *
Project::user_instrument_index()
{
  return &m_user_instrument_index;
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

void
Project::set_random_seed (int seed)
{
  // need to set random seed directly after creation
  g_return_if_fail (m_mix_freq == 0);

  m_random_seed = seed;
}

vector<MorphWavSource *>
Project::list_wav_sources()
{
  vector<MorphWavSource *> wav_sources;

  // find instrument ids
  for (auto op : m_morph_plan.operators())
    {
      string type = op->type();

      if (type == "SpectMorph::MorphWavSource")
        wav_sources.push_back (static_cast<MorphWavSource *> (op));
    }
  return wav_sources;
}

void
Project::post_load()
{
  m_builder_thread.kill_all_jobs();
  synth_interface()->emit_clear_wav_sets();
  for (auto wav_source : list_wav_sources())
    rebuild (wav_source);

  // plan has changed due to instrument map initialization:
  //  -> rebuild morph plan view (somewhat hacky)
  m_morph_plan.signal_need_view_rebuild();
  m_morph_plan.emit_plan_changed();
}

Error
Project::load (const string& filename, bool load_wav_sources)
{
  if (ZipReader::is_zip (filename))
    {
      ZipReader zip_reader (filename);
      if (zip_reader.error())
        return zip_reader.error();

      return load (zip_reader, nullptr, load_wav_sources);
    }
  else
    {
      GenericInP file = GenericIn::open (filename);
      if (file)
        {
          return load_compat (file, nullptr);
        }
      else
        {
          return Error::Code::FILE_NOT_FOUND;
        }
    }
}

Error
Project::load (ZipReader& zip_reader, MorphPlan::ExtraParameters *params, bool load_wav_sources)
{
  /* backup old plan */
  vector<unsigned char> data;
  m_morph_plan.save (MemOut::open (&data));

  /* backup old instruments */
  InstrumentMap old_instrument_map;
  old_instrument_map.swap (m_instrument_map);

  Error error = load_internal (zip_reader, params, load_wav_sources);
  if (error)
    {
      /* restore old plan/instruments if something went wrong */
      m_morph_plan.load (MMapIn::open_vector (data));
      m_instrument_map.swap (old_instrument_map);
    }
  return error;
}

Error
Project::load_internal (ZipReader& zip_reader, MorphPlan::ExtraParameters *params, bool load_wav_sources)
{
  vector<uint8_t> plan = zip_reader.read ("plan.smplan");
  if (zip_reader.error())
    return Error ("Unable to read 'plan.smplan' from input file");

  Error error = m_morph_plan.load (MMapIn::open_vector (plan), params);
  if (error)
    return error;

  for (auto wav_source : list_wav_sources())
    {
      const int object_id = wav_source->object_id();

      Instrument *inst = new Instrument();
      m_instrument_map[object_id].instrument.reset (inst);

      if (m_storage_model == StorageModel::COPY && load_wav_sources)
        {
          string inst_file = string_printf ("instrument%d.sminst", object_id);
          vector<uint8_t> inst_data = zip_reader.read (inst_file);
          if (zip_reader.error())
            return Error (string_printf ("Unable to read '%s' from input file", inst_file.c_str()));

          ZipReader inst_zip (inst_data);
          if (inst_zip.error())
            return inst_zip.error();

          error = inst->load (inst_zip);
          if (error)
            return error;
        }
      else
        {
          string filename = m_user_instrument_index.filename (wav_source->bank(), wav_source->instrument());
          error = inst->load (filename); /* still load preset on error */
          if (!error)
            m_instrument_map[object_id].lv2_absolute_path = filename;
        }
    }

  /* only trigger rebuilds if we loaded everything without error */
  post_load();

  return Error::Code::NONE;
}

Error
Project::load_compat (GenericInP in, MorphPlan::ExtraParameters *params)
{
  Error error = m_morph_plan.load (in, params);

  if (!error)
    {
      m_instrument_map.clear();
      post_load();
    }

  return error;
}

void
Project::load_plan_lv2 (std::function<string(string)> absolute_path, const string& plan_str)
{
  // we return silently if string decode or plan load fail:
  //  -> as LV2 plugin we can't really do much if things go wrong

  vector<unsigned char> data;
  if (!HexString::decode (plan_str, data))
    return;

  Error error = m_morph_plan.load (MMapIn::open_vector (data), nullptr);
  if (error)
    return;

  m_instrument_map.clear();
  // LV2 doesn't include instruments
  for (auto wav_source : list_wav_sources())
    {
      const int object_id = wav_source->object_id();

      Instrument *inst = new Instrument();

      // try load mapped path; if this fails, try user instrument dir
      string filename = absolute_path (wav_source->lv2_abstract_path());

      Error error = inst->load (filename);

      Debug::debug ("lv2", "load '%s':\n", wav_source->name().c_str());
      Debug::debug ("lv2", " - abstract path: '%s'\n", wav_source->lv2_abstract_path().c_str());
      Debug::debug ("lv2", " - canonical path: '%s'\n", std::filesystem::weakly_canonical (filename).string().c_str());
      Debug::debug ("lv2", " - trying load absolute path: '%s' => '%s'\n", filename.c_str(), error.message());

      if (error)
        {
          filename = m_user_instrument_index.filename (wav_source->bank(), wav_source->instrument());
          error = inst->load (filename);

          Debug::debug ("lv2", " - trying load user instrument: '%s' => '%s'\n", filename.c_str(), error.message());

          if (error)
            filename = "";
        }

      // ignore error (if any): we still load preset if instrument is missing
      auto& map_entry = m_instrument_map[object_id];
      map_entry.instrument.reset (inst);
      map_entry.lv2_absolute_path = filename;
    }
  clear_lv2_filenames();
  post_load();
}

Error
Project::save (const std::string& filename)
{
  ZipWriter zip_writer (filename);

  return save (zip_writer, nullptr);
}

Error
Project::save (ZipWriter& zip_writer, MorphPlan::ExtraParameters *params)
{
  vector<unsigned char> data;
  m_morph_plan.save (MemOut::open (&data), params);

  zip_writer.add ("plan.smplan", data);
  for (auto wav_source : list_wav_sources())
    {
      // must do this before using object_id (lazy creation)
      auto& map_entry = lookup_instrument (wav_source);

      int    object_id = wav_source->object_id();
      string inst_file = string_printf ("instrument%d.sminst", object_id);

      ZipWriter   mem_zip;
      map_entry.instrument->save (mem_zip);
      zip_writer.add (inst_file, mem_zip.data(), ZipWriter::Compress::STORE);
    }

  zip_writer.close();
  if (zip_writer.error())
    return zip_writer.error();

  return Error::Code::NONE;
}

string
Project::save_plan_lv2 (std::function<string(string)> abstract_path)
{
  for (auto wav_source : list_wav_sources())
    {
      auto& map_entry = lookup_instrument (wav_source);
      string absolute_path = map_entry.lv2_absolute_path;

      string filename = abstract_path (absolute_path);
      wav_source->set_lv2_abstract_path (filename);

      Debug::debug ("lv2", "save '%s':\n", wav_source->name().c_str());
      Debug::debug ("lv2", " - absolute path: '%s'\n", absolute_path.c_str());
      Debug::debug ("lv2", " - canonical path: '%s'\n", std::filesystem::weakly_canonical (absolute_path).string().c_str());
      Debug::debug ("lv2", " - abstract path: '%s'\n", filename.c_str());
    }

  vector<unsigned char> data;
  m_morph_plan.save (MemOut::open (&data));

  clear_lv2_filenames();

  string plan_str = HexString::encode (data);
  return plan_str;
}

void
Project::clear_lv2_filenames()
{
  /* lv2 abstract path should only be set for the morph plan saved during lv2 load/save */
  for (auto wav_source : list_wav_sources())
    wav_source->set_lv2_abstract_path ("");
}

void
Project::set_lv2_absolute_path (MorphWavSource *wav_source, const string& path)
{
  lookup_instrument (wav_source).lv2_absolute_path = path;

  Debug::debug ("lv2", "edit '%s':\n", wav_source->name().c_str());
  Debug::debug ("lv2", " - absolute path: '%s'\n", path.c_str());
  Debug::debug ("lv2", " - canonical path: '%s'\n", std::filesystem::weakly_canonical (path).string().c_str());
}
