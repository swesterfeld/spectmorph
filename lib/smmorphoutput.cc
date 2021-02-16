// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmorphoutput.hh"
#include "smmorphplan.hh"
#include "smleakdebugger.hh"

#include <assert.h>

#define CHANNEL_OP_COUNT 4

using namespace SpectMorph;

using std::string;
using std::vector;

static LeakDebugger leak_debugger ("SpectMorph::MorphOutput");

MorphOutput::MorphOutput (MorphPlan *morph_plan) :
  MorphOperator (morph_plan)
{
  connect (morph_plan->signal_operator_removed, this, &MorphOutput::on_operator_removed);

  m_config.channel_ops.resize (CHANNEL_OP_COUNT);

  add_property (&m_config.velocity_sensitivity, P_VELOCITY_SENSITIVITY, "Velocity Sns", "%.2f dB", 24, 0, 48);

  m_config.sines = true;
  m_config.noise = true;

  m_config.unison = false;
  add_property (&m_config.unison_voices, P_UNISON_VOICES, "Voices", "%d", 2, 2, 7);
  add_property (&m_config.unison_detune, P_UNISON_DETUNE, "Detune", "%.1f Cent", 6, 0.5, 50);

  m_config.adsr = false;
  add_property (&m_config.adsr_skip, P_ADSR_SKIP, "Skip", "%.1f ms", 500, 0, 1000);
  add_property (&m_config.adsr_attack, P_ADSR_ATTACK, "Attack", "%.1f %%", 15, 0, 100);
  add_property (&m_config.adsr_decay, P_ADSR_DECAY,"Decay", "%.1f %%", 20, 0, 100);
  add_property (&m_config.adsr_sustain, P_ADSR_SUSTAIN, "Sustain", "%.1f %%", 70, 0, 100);
  add_property (&m_config.adsr_release, P_ADSR_RELEASE, "Release", "%.1f %%", 50, 0, 100);

  EnumInfo filter_type_enum_info (
    {
      { FILTER_LP1, "Low-pass 6dB" },
      { FILTER_LP2, "Low-pass 12dB" },
      { FILTER_LP3, "Low-pass 18dB" },
      { FILTER_LP4, "Low-pass 24dB" }
    });
  m_config.filter        = false;
  add_property_enum (&m_config.filter_type, P_FILTER_TYPE, "Filter Type", FILTER_LP2, filter_type_enum_info);
  add_property (&m_config.filter_attack, P_FILTER_ATTACK, "Attack", "%.1f %%", 15, 0, 100);
  add_property (&m_config.filter_decay, P_FILTER_DECAY, "Decay", "%.1f %%", 20, 0, 100);
  add_property (&m_config.filter_sustain, P_FILTER_SUSTAIN, "Sustain", "%.1f %%", 50, 0, 100);
  add_property (&m_config.filter_release, P_FILTER_RELEASE, "Release", "%.1f %%", 50, 0, 100);
  add_property (&m_config.filter_depth, P_FILTER_DEPTH, "Depth", "%.1f st", 24, -60, 60);
  add_property_log (&m_config.filter_cutoff, P_FILTER_CUTOFF, "Cutoff", "%.1f Hz", 500, 20, 20000);
  add_property (&m_config.filter_resonance, P_FILTER_RESONANCE, "Resonance", "%.1f %%", 30, 0, 100);

  m_config.portamento = false;
  add_property_xparam (&m_config.portamento_glide, P_PORTAMENTO_GLIDE, "Glide", "%.2f ms", 200, 0, 1000, 3);

  m_config.vibrato = false;
  add_property (&m_config.vibrato_depth, P_VIBRATO_DEPTH, "Depth", "%.2f Cent", 10, 0, 50);
  add_property_log (&m_config.vibrato_frequency, P_VIBRATO_FREQUENCY, "Frequency", "%.3f Hz", 4, 1, 15);
  add_property (&m_config.vibrato_attack, P_VIBRATO_ATTACK, "Attack", "%.2f ms", 0, 0, 1000);

  leak_debugger.add (this);
}

MorphOutput::~MorphOutput()
{
  leak_debugger.del (this);
}

const char *
MorphOutput::type()
{
  return "SpectMorph::MorphOutput";
}

int
MorphOutput::insert_order()
{
  return 1000;
}

bool
MorphOutput::save (OutFile& out_file)
{
  write_properties (out_file);

  for (size_t i = 0; i < m_config.channel_ops.size(); i++)
    {
      string name;

      if (m_config.channel_ops[i])   // NULL pointer => name = ""
        name = m_config.channel_ops[i].get()->name();

      out_file.write_string ("channel", name);
    }
  out_file.write_bool ("sines", m_config.sines);
  out_file.write_bool ("noise", m_config.noise);
  out_file.write_bool ("unison", m_config.unison);
  out_file.write_bool ("adsr", m_config.adsr);
  out_file.write_bool ("filter", m_config.filter);
  out_file.write_bool ("portamento", m_config.portamento);
  out_file.write_bool ("vibrato", m_config.vibrato);

  return true;
}

bool
MorphOutput::load (InFile& ifile)
{
  load_channel_op_names.clear();

  while (ifile.event() != InFile::END_OF_FILE)
    {
      if (read_property_event (ifile))
        {
          // property has been read, so we ignore the event
        }
      else if (ifile.event() == InFile::STRING)
        {
          if (ifile.event_name() == "channel")
            {
              load_channel_op_names.push_back (ifile.event_data());
            }
          else
            {
              g_printerr ("bad string\n");
              return false;
            }
        }
      else if (ifile.event() == InFile::BOOL)
        {
          if (ifile.event_name() == "sines")
            {
              m_config.sines = ifile.event_bool();
            }
          else if (ifile.event_name() == "noise")
            {
              m_config.noise = ifile.event_bool();
            }
          else if (ifile.event_name() == "unison")
            {
              m_config.unison = ifile.event_bool();
            }
          else if (ifile.event_name() == "adsr")
            {
              m_config.adsr = ifile.event_bool();
            }
          else if (ifile.event_name() == "filter")
            {
              m_config.filter = ifile.event_bool();
            }
          else if (ifile.event_name() == "portamento")
            {
              m_config.portamento = ifile.event_bool();
            }
          else if (ifile.event_name() == "vibrato")
            {
              m_config.vibrato = ifile.event_bool();
            }
          else
            {
              g_printerr ("bad bool\n");
              return false;
            }
        }
      else
        {
          g_printerr ("bad event\n");
          return false;
        }
      ifile.next_event();
    }
  return true;
}

void
MorphOutput::post_load (OpNameMap& op_name_map)
{
  for (size_t i = 0; i < m_config.channel_ops.size(); i++)
    {
      if (i < load_channel_op_names.size())
        {
          string name = load_channel_op_names[i];
          m_config.channel_ops[i].set (op_name_map[name]);
        }
    }
}

MorphOperator::OutputType
MorphOutput::output_type()
{
  return OUTPUT_NONE;
}

void
MorphOutput::set_channel_op (int ch, MorphOperator *op)
{
  assert (ch >= 0 && ch < CHANNEL_OP_COUNT);

  m_config.channel_ops[ch].set (op);
  m_morph_plan->emit_plan_changed();
}

MorphOperator *
MorphOutput::channel_op (int ch)
{
  assert (ch >= 0 && ch < CHANNEL_OP_COUNT);

  return m_config.channel_ops[ch].get();
}

bool
MorphOutput::sines() const
{
  return m_config.sines;
}

void
MorphOutput::set_sines (bool es)
{
  m_config.sines = es;

  m_morph_plan->emit_plan_changed();
}

bool
MorphOutput::noise() const
{
  return m_config.noise;
}

void
MorphOutput::set_noise (bool en)
{
  m_config.noise = en;

  m_morph_plan->emit_plan_changed();
}

//---- unison effect ----

bool
MorphOutput::unison() const
{
  return m_config.unison;
}

void
MorphOutput::set_unison (bool eu)
{
  m_config.unison = eu;

  m_morph_plan->emit_plan_changed();
}

//---- adsr ----

bool
MorphOutput::adsr() const
{
  return m_config.adsr;
}

void
MorphOutput::set_adsr (bool ea)
{
  m_config.adsr = ea;

  m_morph_plan->emit_plan_changed();
}

//---- filter ----

void
MorphOutput::set_filter (bool efilter)
{
  m_config.filter = efilter;

  m_morph_plan->emit_plan_changed();
}

bool
MorphOutput::filter() const
{
  return m_config.filter;
}

//---- portamento/mono mode ----

bool
MorphOutput::portamento() const
{
  return m_config.portamento;
}

void
MorphOutput::set_portamento (bool ep)
{
  m_config.portamento = ep;

  m_morph_plan->emit_plan_changed();
}

//---- vibrato ----

bool
MorphOutput::vibrato() const
{
  return m_config.vibrato;
}

void
MorphOutput::set_vibrato (bool ev)
{
  m_config.vibrato = ev;

  m_morph_plan->emit_plan_changed();
}

void
MorphOutput::on_operator_removed (MorphOperator *op)
{
  for (size_t ch = 0; ch < m_config.channel_ops.size(); ch++)
    {
      if (m_config.channel_ops[ch].get() == op)
        m_config.channel_ops[ch].set (nullptr);
    }
}

vector<MorphOperator *>
MorphOutput::dependencies()
{
  vector<MorphOperator *> deps;

  for (auto& ptr : m_config.channel_ops)
    deps.push_back (ptr.get());

  return deps;
}

MorphOperatorConfig *
MorphOutput::clone_config()
{
  Config *cfg = new Config (m_config);
  return cfg;
}
