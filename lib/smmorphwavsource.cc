// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smmorphwavsource.hh"
#include "smmorphplan.hh"
#include "smproject.hh"
#include "smzip.hh"

using namespace SpectMorph;

using std::string;
using std::vector;

MorphWavSource::MorphWavSource (MorphPlan *morph_plan) :
  MorphOperator (morph_plan)
{
  EnumInfo play_mode_enum_info (
    {
      { PLAY_MODE_STANDARD,        "Standard" },
      { PLAY_MODE_CUSTOM_POSITION, "Custom Position" }
    });

  add_property_enum (&m_config.play_mode, P_PLAY_MODE, "Play Mode", PLAY_MODE_STANDARD, play_mode_enum_info);
  auto position = add_property (&m_config.position_mod, P_POSITION, "Position", "%.1f %%", 50, 0, 100);
  position->modulation_list()->set_compat_type_and_op ("position_control_type", "position_op");

  EnumInfo formant_correct_enum_info (
    {
      { FormantCorrection::MODE_REPITCH,                    "Repitch" },
      { FormantCorrection::MODE_PRESERVE_SPECTRAL_ENVELOPE, "Preserve Spectral Envelope" },
      { FormantCorrection::MODE_HARMONIC_RESYNTHESIS,       "Harmonic Resynthesis" }
    });
  add_property_enum (&m_config.formant_correct, P_FORMANT_CORRECTION, "Formants", FormantCorrection::MODE_REPITCH, formant_correct_enum_info);
  add_property (&m_config.fuzzy_resynth, P_FUZZY_FREQS, "Fuzzy Freqs", "%.1f %%", 20, 0, 100);

  UserInstrumentIndex *user_instrument_index = morph_plan->project()->user_instrument_index();
  connect (user_instrument_index->signal_instrument_updated, this, &MorphWavSource::on_instrument_updated);
  connect (user_instrument_index->signal_bank_removed, this, &MorphWavSource::on_bank_removed);
}

void
MorphWavSource::set_object_id (int id)
{
  // object id to use in Project
  m_config.object_id = id;

  m_morph_plan->emit_plan_changed();
}

int
MorphWavSource::object_id()
{
  return m_config.object_id;
}

void
MorphWavSource::set_bank_and_instrument (const string& bank, int instrument)
{
  instrument = std::clamp (instrument, 1, 128);
  if (m_bank != bank || m_instrument != instrument)
    {
      m_bank = bank;
      m_instrument = instrument;

      Project *project = morph_plan()->project();
      Instrument *instrument = project->lookup_instrument (this).instrument.get();
      UserInstrumentIndex *user_instrument_index = project->user_instrument_index();

      string filename = user_instrument_index->filename (m_bank, m_instrument);
      Error error = instrument->load (filename);
      if (error)
        {
          /* most likely cause of error: this user instrument doesn't exist yet */
          instrument->clear();
          filename = "";
        }
      project->set_lv2_absolute_path (this, filename);
      project->rebuild (this);

      signal_labels_changed();

      m_morph_plan->emit_plan_changed();
    }
}

void
MorphWavSource::on_bank_removed (const string& bank)
{
  if (bank == m_bank)
    {
      m_bank = ""; // force re-read

      set_bank_and_instrument (MorphWavSource::USER_BANK, 1);
    }
}

int
MorphWavSource::instrument()
{
  return m_instrument;
}

string
MorphWavSource::bank()
{
  return m_bank;
}

void
MorphWavSource::set_lv2_abstract_path (const string& path)
{
  m_lv2_abstract_path = path;

  m_morph_plan->emit_plan_changed();
}

string
MorphWavSource::lv2_abstract_path()
{
  return m_lv2_abstract_path;
}

void
MorphWavSource::on_instrument_updated (const std::string& bank, int number, const Instrument *new_instrument)
{
  if (bank == m_bank && number == m_instrument)
    {
      auto project  = m_morph_plan->project();
      Instrument *instrument = project->lookup_instrument (this).instrument.get();

      if (new_instrument->size())
        {
          ZipWriter new_inst_writer;
          new_instrument->save (new_inst_writer);
          ZipReader new_inst_reader (new_inst_writer.data());
          instrument->load (new_inst_reader);

          // point lv2 absolute path back to user instrument dir to save the changed version
          UserInstrumentIndex *user_instrument_index = project->user_instrument_index();
          project->set_lv2_absolute_path (this, user_instrument_index->filename (m_bank, m_instrument));
        }
      else
        {
          instrument->clear();
          project->set_lv2_absolute_path (this, "");
        }
      project->rebuild (this);
      project->state_changed();

      signal_labels_changed();
    }
}

const char *
MorphWavSource::type()
{
  return "SpectMorph::MorphWavSource";
}

int
MorphWavSource::insert_order()
{
  return 0;
}

bool
MorphWavSource::save (OutFile& out_file)
{
  write_properties (out_file);

  out_file.write_int ("object_id", m_config.object_id);
  out_file.write_int ("instrument", m_instrument);
  // compat: old projects save the abstract path as "lv2_filename", so we keep it that way
  out_file.write_string ("lv2_filename", m_lv2_abstract_path);
  out_file.write_string ("bank", m_bank);

  return true;
}

bool
MorphWavSource::load (InFile& ifile)
{
  bool read_ok = true;
  while (ifile.event() != InFile::END_OF_FILE)
    {
      if (read_property_event (ifile))
        {
          // property has been read, so we ignore the event
        }
      else if (ifile.event() == InFile::INT)
        {
          if (ifile.event_name() == "object_id")
            {
              m_config.object_id = ifile.event_int();
            }
          else if (ifile.event_name() == "instrument")
            {
              m_instrument = ifile.event_int();
            }
          else
            {
              report_bad_event (read_ok, ifile);
            }
        }
      else if (ifile.event() == InFile::STRING)
        {
          // compat: old projects save ithe abstract path as "lv2_filename", so we keep it that way
          if (ifile.event_name() == "lv2_filename")
            {
              m_lv2_abstract_path = ifile.event_data();
            }
          else if (ifile.event_name() == "bank")
            {
              m_bank = ifile.event_data();
            }
          else
            {
              report_bad_event (read_ok, ifile);
            }
        }
      else
        {
          report_bad_event (read_ok, ifile);
        }
      ifile.next_event();
    }
  return read_ok;
}

MorphOperator::OutputType
MorphWavSource::output_type()
{
  return OUTPUT_AUDIO;
}

vector<MorphOperator *>
MorphWavSource::dependencies()
{
  vector<MorphOperator *> deps;

  if (m_config.play_mode == PLAY_MODE_CUSTOM_POSITION)
    get_property_dependencies (deps, { P_POSITION });
  return deps;
}

MorphOperatorConfig *
MorphWavSource::clone_config()
{
  Config *cfg = new Config (m_config);

  cfg->project = morph_plan()->project();

  return cfg;
}
