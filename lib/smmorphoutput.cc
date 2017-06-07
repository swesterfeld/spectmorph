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
  MorphOperator (morph_plan),
  channel_ops (CHANNEL_OP_COUNT)
{
  connect (morph_plan, SIGNAL (operator_removed (MorphOperator *)), this, SLOT (on_operator_removed (MorphOperator *)));

  m_sines = true;
  m_noise = true;

  m_unison = false;
  m_unison_voices = 2;
  m_unison_detune = 6.0;

  m_portamento = false;
  m_portamento_glide = 200; /* ms */

  m_vibrato = false;
  m_vibrato_depth = 10;    /* cent */
  m_vibrato_frequency = 4; /* Hz */
  m_vibrato_attack = 0;    /* ms */

  leak_debugger.add (this);
}

MorphOutputProperties::MorphOutputProperties (MorphOutput *output) :
  portamento_glide (output, "Glide", "%.2f ms", 0, 1000, 3, &MorphOutput::portamento_glide, &MorphOutput::set_portamento_glide),
  vibrato_depth (output, "Depth", "%.2f Cent", 0, 100, 1, &MorphOutput::vibrato_depth, &MorphOutput::set_vibrato_depth),
  vibrato_frequency (output, "Frequency", "%.3f Hz", 0.25, 25, &MorphOutput::vibrato_frequency, &MorphOutput::set_vibrato_frequency),
  vibrato_attack (output, "Attack", "%.2f ms", 0, 1000, 1, &MorphOutput::vibrato_attack, &MorphOutput::set_vibrato_attack)
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
  for (size_t i = 0; i < channel_ops.size(); i++)
    {
      string name;

      if (channel_ops[i])   // NULL pointer => name = ""
        name = channel_ops[i]->name();

      out_file.write_string ("channel", name);
    }
  out_file.write_bool ("sines", m_sines);
  out_file.write_bool ("noise", m_noise);
  out_file.write_bool ("unison", m_unison);
  out_file.write_int ("unison_voices", m_unison_voices);
  out_file.write_float ("unison_detune", m_unison_detune);
  out_file.write_bool ("portamento", m_portamento);
  out_file.write_float ("portamento_glide", m_portamento_glide);
  out_file.write_bool ("vibrato", m_vibrato);
  out_file.write_float ("vibrato_depth", m_vibrato_depth);
  out_file.write_float ("vibrato_frequency", m_vibrato_frequency);
  out_file.write_float ("vibrato_attack", m_vibrato_attack);
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
              m_sines = ifile.event_bool();
            }
          else if (ifile.event_name() == "noise")
            {
              m_noise = ifile.event_bool();
            }
          else if (ifile.event_name() == "unison")
            {
              m_unison = ifile.event_bool();
            }
          else if (ifile.event_name() == "portamento")
            {
              m_portamento = ifile.event_bool();
            }
          else if (ifile.event_name() == "vibrato")
            {
              m_vibrato = ifile.event_bool();
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
              m_unison_voices = ifile.event_int();
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
              m_unison_detune = ifile.event_float();
            }
          else if (ifile.event_name() == "portamento_glide")
            {
              m_portamento_glide = ifile.event_float();
            }
          else if (ifile.event_name() == "vibrato_depth")
            {
              m_vibrato_depth = ifile.event_float();
            }
          else if (ifile.event_name() == "vibrato_frequency")
            {
              m_vibrato_frequency = ifile.event_float();
            }
          else if (ifile.event_name() == "vibrato_attack")
            {
              m_vibrato_attack = ifile.event_float();
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
  for (size_t i = 0; i < channel_ops.size(); i++)
    {
      if (i < load_channel_op_names.size())
        {
          string name = load_channel_op_names[i];
          channel_ops[i] = op_name_map[name];
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

  channel_ops[ch] = op;
  m_morph_plan->emit_plan_changed();
}

MorphOperator *
MorphOutput::channel_op (int ch)
{
  assert (ch >= 0 && ch < CHANNEL_OP_COUNT);

  return channel_ops[ch];
}

bool
MorphOutput::sines() const
{
  return m_sines;
}

void
MorphOutput::set_sines (bool es)
{
  m_sines = es;

  m_morph_plan->emit_plan_changed();
}

bool
MorphOutput::noise() const
{
  return m_noise;
}

void
MorphOutput::set_noise (bool en)
{
  m_noise = en;

  m_morph_plan->emit_plan_changed();
}

//---- unison effect ----

bool
MorphOutput::unison() const
{
  return m_unison;
}

void
MorphOutput::set_unison (bool eu)
{
  m_unison = eu;

  m_morph_plan->emit_plan_changed();
}

int
MorphOutput::unison_voices() const
{
  return m_unison_voices;
}

void
MorphOutput::set_unison_voices (int voices)
{
  m_unison_voices = voices;

  m_morph_plan->emit_plan_changed();
}

float
MorphOutput::unison_detune() const
{
  return m_unison_detune;
}

void
MorphOutput::set_unison_detune (float detune)
{
  m_unison_detune = detune;

  m_morph_plan->emit_plan_changed();
}

//---- portamento/mono mode ----

bool
MorphOutput::portamento() const
{
  return m_portamento;
}

void
MorphOutput::set_portamento (bool ep)
{
  m_portamento = ep;

  m_morph_plan->emit_plan_changed();
}

float
MorphOutput::portamento_glide() const
{
  return m_portamento_glide;
}

void
MorphOutput::set_portamento_glide (float glide)
{
  m_portamento_glide = glide;

  m_morph_plan->emit_plan_changed();
}


//---- vibrato ----

bool
MorphOutput::vibrato() const
{
  return m_vibrato;
}

void
MorphOutput::set_vibrato (bool ev)
{
  m_vibrato = ev;

  m_morph_plan->emit_plan_changed();
}

float
MorphOutput::vibrato_depth() const
{
  return m_vibrato_depth;
}

void
MorphOutput::set_vibrato_depth (float depth)
{
  m_vibrato_depth = depth;

  m_morph_plan->emit_plan_changed();
}

float
MorphOutput::vibrato_frequency() const
{
  return m_vibrato_frequency;
}

void
MorphOutput::set_vibrato_frequency (float frequency)
{
  m_vibrato_frequency = frequency;

  m_morph_plan->emit_plan_changed();
}

float
MorphOutput::vibrato_attack() const
{
  return m_vibrato_attack;
}

void
MorphOutput::set_vibrato_attack (float attack)
{
  m_vibrato_attack = attack;

  m_morph_plan->emit_plan_changed();
}

void
MorphOutput::on_operator_removed (MorphOperator *op)
{
  for (size_t ch = 0; ch < channel_ops.size(); ch++)
    {
      if (channel_ops[ch] == op)
        channel_ops[ch] = NULL;
    }
}
