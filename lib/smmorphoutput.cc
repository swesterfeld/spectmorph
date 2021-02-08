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
  m_config.adsr_skip     = 500;
  m_config.adsr_attack   = 15;
  m_config.adsr_decay    = 20;
  m_config.adsr_sustain  = 70;
  m_config.adsr_release  = 50;

  m_config.portamento = false;
  m_config.portamento_glide = 200; /* ms */

  m_config.vibrato = false;
  m_config.vibrato_depth = 10;    /* cent */
  m_config.vibrato_frequency = 4; /* Hz */
  m_config.vibrato_attack = 0;    /* ms */

  leak_debugger.add (this);
}

MorphOutputProperties::MorphOutputProperties (MorphOutput *output) :
  adsr_skip (output, "Skip", "%.1f ms", 0, 1000, &MorphOutput::adsr_skip, &MorphOutput::set_adsr_skip),
  adsr_attack (output, "Attack", "%.1f %%", 0, 100, &MorphOutput::adsr_attack, &MorphOutput::set_adsr_attack),
  adsr_decay (output, "Decay", "%.1f %%", 0, 100, &MorphOutput::adsr_decay, &MorphOutput::set_adsr_decay),
  adsr_sustain (output, "Sustain", "%.1f %%", 0, 100, &MorphOutput::adsr_sustain, &MorphOutput::set_adsr_sustain),
  adsr_release (output, "Release", "%.1f %%", 0, 100, &MorphOutput::adsr_release, &MorphOutput::set_adsr_release),
  portamento_glide (output, "Glide", "%.2f ms", 0, 1000, 3, &MorphOutput::portamento_glide, &MorphOutput::set_portamento_glide),
  vibrato_depth (output, "Depth", "%.2f Cent", 0, 50, &MorphOutput::vibrato_depth, &MorphOutput::set_vibrato_depth),
  vibrato_frequency (output, "Frequency", "%.3f Hz", 1.0, 15, &MorphOutput::vibrato_frequency, &MorphOutput::set_vibrato_frequency),
  vibrato_attack (output, "Attack", "%.2f ms", 0, 1000, &MorphOutput::vibrato_attack, &MorphOutput::set_vibrato_attack),
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
  out_file.write_float ("adsr_skip",    m_config.adsr_skip);
  out_file.write_float ("adsr_attack",  m_config.adsr_attack);
  out_file.write_float ("adsr_decay",   m_config.adsr_decay);
  out_file.write_float ("adsr_sustain", m_config.adsr_sustain);
  out_file.write_float ("adsr_release", m_config.adsr_release);

  out_file.write_bool ("portamento", m_config.portamento);
  out_file.write_float ("portamento_glide", m_config.portamento_glide);

  out_file.write_bool ("vibrato", m_config.vibrato);
  out_file.write_float ("vibrato_depth", m_config.vibrato_depth);
  out_file.write_float ("vibrato_frequency", m_config.vibrato_frequency);
  out_file.write_float ("vibrato_attack", m_config.vibrato_attack);

  out_file.write_float ("velocity_sensitivity", m_config.velocity_sensitivity);

  return true;
}

bool
MorphOutput::load (InFile& ifile)
{
  load_channel_op_names.clear();

  while (ifile.event() != InFile::END_OF_FILE)
    {
      if (ifile.event() == InFile::STRING)
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
          else if (ifile.event_name() == "adsr_skip")
            {
              m_config.adsr_skip = ifile.event_float();
            }
          else if (ifile.event_name() == "adsr_attack")
            {
              m_config.adsr_attack = ifile.event_float();
            }
          else if (ifile.event_name() == "adsr_decay")
            {
              m_config.adsr_decay = ifile.event_float();
            }
          else if (ifile.event_name() == "adsr_sustain")
            {
              m_config.adsr_sustain = ifile.event_float();
            }
          else if (ifile.event_name() == "adsr_release")
            {
              m_config.adsr_release = ifile.event_float();
            }
          else if (ifile.event_name() == "portamento_glide")
            {
              m_config.portamento_glide = ifile.event_float();
            }
          else if (ifile.event_name() == "vibrato_depth")
            {
              m_config.vibrato_depth = ifile.event_float();
            }
          else if (ifile.event_name() == "vibrato_frequency")
            {
              m_config.vibrato_frequency = ifile.event_float();
            }
          else if (ifile.event_name() == "vibrato_attack")
            {
              m_config.vibrato_attack = ifile.event_float();
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

float
MorphOutput::adsr_skip() const
{
  return m_config.adsr_skip;
}

void
MorphOutput::set_adsr_skip (float skip)
{
  m_config.adsr_skip = skip;

  m_morph_plan->emit_plan_changed();
}

float
MorphOutput::adsr_attack() const
{
  return m_config.adsr_attack;
}

void
MorphOutput::set_adsr_attack (float attack)
{
  m_config.adsr_attack = attack;

  m_morph_plan->emit_plan_changed();
}

float
MorphOutput::adsr_decay() const
{
  return m_config.adsr_decay;
}

void
MorphOutput::set_adsr_decay (float decay)
{
  m_config.adsr_decay = decay;

  m_morph_plan->emit_plan_changed();
}

float
MorphOutput::adsr_release() const
{
  return m_config.adsr_release;
}

void
MorphOutput::set_adsr_release (float release)
{
  m_config.adsr_release = release;

  m_morph_plan->emit_plan_changed();
}

float
MorphOutput::adsr_sustain() const
{
  return m_config.adsr_sustain;
}

void
MorphOutput::set_adsr_sustain (float sustain)
{
  m_config.adsr_sustain = sustain;

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
MorphOutput::vibrato_depth() const
{
  return m_config.vibrato_depth;
}

void
MorphOutput::set_vibrato_depth (float depth)
{
  m_config.vibrato_depth = depth;

  m_morph_plan->emit_plan_changed();
}

float
MorphOutput::vibrato_frequency() const
{
  return m_config.vibrato_frequency;
}

void
MorphOutput::set_vibrato_frequency (float frequency)
{
  m_config.vibrato_frequency = frequency;

  m_morph_plan->emit_plan_changed();
}

float
MorphOutput::vibrato_attack() const
{
  return m_config.vibrato_attack;
}

void
MorphOutput::set_vibrato_attack (float attack)
{
  m_config.vibrato_attack = attack;

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
