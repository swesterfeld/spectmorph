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
  for (size_t i = 0; i < channel_ops.size(); i++)
    {
      string name;

      if (channel_ops[i])   // NULL pointer => name = ""
        name = channel_ops[i]->name();

      out_file.write_string ("channel", name);
    }
  out_file.write_bool ("sines", m_sines);
  out_file.write_bool ("noise", m_noise);
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
MorphOutput::sines()
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
MorphOutput::noise()
{
  return m_noise;
}

void
MorphOutput::set_noise (bool en)
{
  m_noise = en;

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
