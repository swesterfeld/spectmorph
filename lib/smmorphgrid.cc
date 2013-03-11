// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmorphgrid.hh"
#include "smleakdebugger.hh"

using namespace SpectMorph;

static LeakDebugger leak_debugger ("SpectMorph::MorphGrid");

MorphGrid::MorphGrid (MorphPlan *morph_plan) :
  MorphOperator (morph_plan)
{
  leak_debugger.add (this);
}

MorphGrid::~MorphGrid()
{
  leak_debugger.del (this);
}

const char *
MorphGrid::type()
{
  return "SpectMorph::MorphGrid";
}

bool
MorphGrid::save (OutFile& out_file)
{
  return true;
}

bool
MorphGrid::load (InFile& ifile)
{
  return true;
}

MorphOperator::OutputType
MorphGrid::output_type()
{
  return OUTPUT_AUDIO;
}


