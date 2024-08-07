// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smmorphsource.hh"
#include "smmorphplan.hh"
#include "smwavsetrepo.hh"

using namespace SpectMorph;

using std::string;

MorphSource::MorphSource (MorphPlan *morph_plan) :
  MorphOperator (morph_plan)
{
}

void
MorphSource::set_smset (const string& smset)
{
  m_smset = smset;
  m_morph_plan->emit_plan_changed();
}

string
MorphSource::smset()
{
  return m_smset;
}

MorphOperatorConfig *
MorphSource::clone_config()
{
  Config *cfg = new Config (m_config);

  string smset_dir = morph_plan()->index()->smset_dir();
  cfg->wav_set = WavSetRepo::the()->get (smset_dir + "/" + m_smset);

  return cfg;
}

const char *
MorphSource::type()
{
  return "SpectMorph::MorphSource";
}

int
MorphSource::insert_order()
{
  return 0;
}

bool
MorphSource::save (OutFile& out_file)
{
  out_file.write_string ("instrument", m_smset);

  return true;
}

bool
MorphSource::load (InFile& ifile)
{
  bool read_ok = true;
  while (ifile.event() != InFile::END_OF_FILE)
    {
      if (ifile.event() == InFile::STRING)
        {
          if (ifile.event_name() == "instrument")
            {
              m_smset = ifile.event_data();
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
MorphSource::output_type()
{
  return OUTPUT_AUDIO;
}
