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

  m_config.velocity_sensitivity = 24; /* dB */

  m_config.sines = true;
  m_config.noise = true;

  m_config.unison = false;
  m_config.unison_voices = 2;
  m_config.unison_detune = 6.0;

  m_config.adsr = false;
  add_property (&m_config.adsr_skip, P_ADSR_SKIP, "Skip", "%.1f ms", 500, 0, 1000);
  add_property (&m_config.adsr_attack, P_ADSR_ATTACK, "Attack", "%.1f %%", 15, 0, 100);
  add_property (&m_config.adsr_decay, P_ADSR_DECAY,"Decay", "%.1f %%", 20, 0, 100);
  add_property (&m_config.adsr_sustain, P_ADSR_SUSTAIN, "Sustain", "%.1f %%", 70, 0, 100);
  add_property (&m_config.adsr_release, P_ADSR_RELEASE, "Release", "%.1f %%", 50, 0, 100);

  m_config.portamento = false;
  m_config.portamento_glide = 200; /* ms */

  m_config.vibrato = false;
  add_property (&m_config.vibrato_depth, P_VIBRATO_DEPTH, "Depth", "%.2f Cent", 10, 0, 50);
  add_property_log (&m_config.vibrato_frequency, P_VIBRATO_FREQUENCY, "Frequency", "%.3f Hz", 4, 1, 15);
  add_property (&m_config.vibrato_attack, P_VIBRATO_ATTACK, "Attack", "%.2f ms", 0, 0, 1000);

  leak_debugger.add (this);
}

MorphOutputProperties::MorphOutputProperties (MorphOutput *output) :
  portamento_glide (output, "Glide", "%.2f ms", 0, 1000, 3, &MorphOutput::portamento_glide, &MorphOutput::set_portamento_glide),
  velocity_sensitivity (output, "Velocity Sns", "%.2f dB", 0, 48, &MorphOutput::velocity_sensitivity, &MorphOutput::set_velocity_sensitivity)
{
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
  out_file.write_int ("unison_voices", m_config.unison_voices);
  out_file.write_float ("unison_detune", m_config.unison_detune);

  out_file.write_bool ("adsr", m_config.adsr);

  out_file.write_bool ("portamento", m_config.portamento);
  out_file.write_float ("portamento_glide", m_config.portamento_glide);

  out_file.write_bool ("vibrato", m_config.vibrato);

  out_file.write_float ("velocity_sensitivity", m_config.velocity_sensitivity);

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
      else if (ifile.event() == InFile::INT)
        {
          if (ifile.event_name() == "unison_voices")
            {
              m_config.unison_voices = ifile.event_int();
            }
          else
            {
              g_printerr ("bad int\n");
              return false;
            }
        }
      else if (ifile.event() == InFile::FLOAT)
        {
          if (ifile.event_name() == "unison_detune")
            {
              m_config.unison_detune = ifile.event_float();
            }
          else if (ifile.event_name() == "portamento_glide")
            {
              m_config.portamento_glide = ifile.event_float();
            }
          else if (ifile.event_name() == "velocity_sensitivity")
            {
              m_config.velocity_sensitivity = ifile.event_float();
            }
          else
            {
              g_printerr ("bad float\n");
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

int
MorphOutput::unison_voices() const
{
  return m_config.unison_voices;
}

void
MorphOutput::set_unison_voices (int voices)
{
  m_config.unison_voices = voices;

  m_morph_plan->emit_plan_changed();
}

float
MorphOutput::unison_detune() const
{
  return m_config.unison_detune;
}

void
MorphOutput::set_unison_detune (float detune)
{
  m_config.unison_detune = detune;

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

float
MorphOutput::portamento_glide() const
{
  return m_config.portamento_glide;
}

void
MorphOutput::set_portamento_glide (float glide)
{
  m_config.portamento_glide = glide;

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

float
MorphOutput::velocity_sensitivity() const
{
  return m_config.velocity_sensitivity;
}

void
MorphOutput::set_velocity_sensitivity (float velocity_sensitivity)
{
  m_config.velocity_sensitivity = velocity_sensitivity;

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
