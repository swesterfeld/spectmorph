// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smuserinstrumentindex.hh"
#include "smmorphwavsource.hh"
#include "smzip.hh"

#include <filesystem>

using namespace SpectMorph;

using std::vector;
using std::string;

static string
tolower (const string& s)
{
  string lower;
  for (auto c : s)
    lower.push_back (tolower (c)); // will not work for utf8
  return lower;
}

UserInstrumentIndex::UserInstrumentIndex()
{
  user_instruments_dir = sm_get_documents_dir (DOCUMENTS_DIR_INSTRUMENTS);
}

void
UserInstrumentIndex::create_instrument_dir (const string& bank)
{
  /* if bank directory doesn't exist, create it */
  auto dir = user_instruments_dir + "/" + bank;
  g_mkdir_with_parents (dir.c_str(), 0775);
}

void
UserInstrumentIndex::update_instrument (const std::string& bank, int number, const Instrument& instrument)
{
  /* update on disk copy */
  string path = filename (bank, number);
  if (instrument.size())
    {
      // create directory only when needed (on write)
      create_instrument_dir (bank);

      ZipWriter zip_writer (path);
      instrument.save (zip_writer);
    }
  else
    {
      /* instrument without any samples -> remove */
      unlink (path.c_str());
    }

  /* update wav sources and UI */
  signal_instrument_updated (bank, number, &instrument);
  signal_instrument_list_updated (bank);
}

string
UserInstrumentIndex::filename (const string& bank, int number)
{
  return string_printf ("%s/%s/%d.sminst", user_instruments_dir.c_str(), bank.c_str(), number);
}

string
UserInstrumentIndex::label (const string& bank, int number)
{
  Instrument inst;

  Error error = inst.load (filename (bank, number), Instrument::LoadOptions::NAME_ONLY);
  if (!error)
    return string_printf ("%03d %s", number, inst.name().c_str());
  else
    return string_printf ("%03d ---", number);
}

void
UserInstrumentIndex::remove_bank (const string& bank)
{
  std::error_code ec;
  for (int i = 1; i <= 128; i++)
    {
      std::filesystem::remove (filename (bank, i), ec);
    }
  auto bank_directory = string_printf ("%s/%s", user_instruments_dir.c_str(), bank.c_str());
  std::filesystem::remove (bank_directory, ec);

  signal_banks_changed();
}

void
UserInstrumentIndex::create_bank (const string& bank)
{
  create_instrument_dir (bank);

  signal_banks_changed();
}

vector<string>
UserInstrumentIndex::list_banks()
{
  string inst_dir = sm_get_documents_dir (DOCUMENTS_DIR_INSTRUMENTS);
  vector<string> banks, dir_contents;
  read_dir (inst_dir, dir_contents); // ignore errors

  for (auto entry : dir_contents)
    {
      string full_path = inst_dir + "/" + entry;
      if (g_file_test (full_path.c_str(), G_FILE_TEST_IS_DIR))
        banks.push_back (entry);
    }

  if (find (banks.begin(), banks.end(), MorphWavSource::USER_BANK) == banks.end())
    banks.push_back (MorphWavSource::USER_BANK); // we always have a User bank

  sort (banks.begin(), banks.end(), [] (auto& b1, auto& b2) { return tolower (b1) < tolower (b2); });
  return banks;
}

int
UserInstrumentIndex::count (const string& bank)
{
  int n_instruments = 0;
  for (int i = 1; i <= 128; i++)
    {
      Instrument inst;

      Error error = inst.load (filename (bank, i), Instrument::LoadOptions::NAME_ONLY);
      if (!error)
        n_instruments++;
    }
  return n_instruments;
}

