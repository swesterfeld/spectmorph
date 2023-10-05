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

Error
UserInstrumentIndex::create_bank_directory (const string& bank)
{
  /* if bank directory doesn't exist, create it */
  std::error_code ec;

  std::filesystem::create_directories (bank_directory (bank), ec);
  if (ec)
    return Error (ec.message());
  else
    return Error::Code::NONE;
}

Error
UserInstrumentIndex::update_instrument (const std::string& bank, int number, const Instrument& instrument)
{
  Error error = Error::Code::NONE;

  /* update on disk copy */
  string path = filename (bank, number);
  if (instrument.size())
    {
      // create directory only when needed (on write)
      error = create_bank_directory (bank);

      if (!error)
        {
          ZipWriter zip_writer (path);
          error = instrument.save (zip_writer);
        }
    }
  else
    {
      /* instrument without any samples -> remove */
      std::error_code ec;
      std::filesystem::remove (path, ec);
      if (ec)
        error = Error (ec.message());
    }

  /* update wav sources and UI */
  signal_instrument_updated (bank, number, &instrument);
  signal_instrument_list_updated (bank);

  return error;
}

string
UserInstrumentIndex::filename (const string& bank, int number)
{
  return string_printf ("%s/%d.sminst", bank_directory (bank).c_str(), number);
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

Error
UserInstrumentIndex::remove_bank (const string& bank)
{
  Error error = Error::Code::NONE;
  auto update_error = [&] (std::error_code ec) {
    if (ec && !error)
      error = Error (ec.message());
  };

  std::error_code ec;
  for (int i = 1; i <= 128; i++)
    {
      std::filesystem::remove (filename (bank, i), ec);
      update_error (ec);
    }
  std::filesystem::remove (bank_directory (bank), ec);
  update_error (ec);

  signal_bank_removed (bank);
  signal_banks_changed();

  return error;
}

string
UserInstrumentIndex::bank_directory (const string& bank)
{
  return string_printf ("%s/%s", user_instruments_dir.c_str(), bank.c_str());
}

Error
UserInstrumentIndex::create_bank (const string& bank)
{
  Error error = create_bank_directory (bank);

  signal_banks_changed();

  return error;
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

