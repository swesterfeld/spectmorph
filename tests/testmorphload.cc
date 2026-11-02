// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smmain.hh"
#include "smfft.hh"
#include "smproject.hh"
#include "smsynthinterface.hh"
#include "smmorphplan.hh"
#include "sminfile.hh"
#include "smoutfile.hh"
#include "smmemout.hh"
#include "smmmapin.hh"
#include "smzip.hh"

#include <cassert>
#include <memory>
#include <set>

using namespace SpectMorph;
using std::string;
using std::vector;

static vector<unsigned char>
save_plan (MorphPlan& plan)
{
  vector<unsigned char> data;
  assert (!plan.save (MemOut::open (&data)));
  return data;
}

static vector<unsigned char>
operator_data (MorphOperator& op)
{
  vector<unsigned char> data;
  {
    OutFile out (MemOut::open (&data), op.type(), SPECTMORPH_BINARY_FILE_VERSION);
    assert (op.save (out));
  }
  return data;
}

static vector<unsigned char>
plan_data (const string& type, const vector<unsigned char>& blob, bool reference_only = false)
{
  vector<unsigned char> data;
  {
    OutFile out (MemOut::open (&data), "SpectMorph::MorphPlan", SPECTMORPH_BINARY_FILE_VERSION);
    // This blob is not operator data, so the plan loader will not cache it.
    // The following operator payload becomes an unresolved BLOB_REF.
    if (reference_only)
      out.write_blob ("unused", blob.data(), blob.size());
    for (int i = 0; i < 2; i++)
      {
        out.begin_section ("operator");
        out.write_string ("type", type);
        out.write_blob ("data", blob.data(), blob.size());
        out.end_section();
      }
  }
  return data;
}

class ExtraParameters : public MorphPlan::ExtraParameters
{
public:
  string section() override { return "extra"; }
  void save (OutFile&) override {}
  void handle_event (InFile& in) override
  {
    // Stream errors must be handled before dispatching optional parameters.
    assert (in.event() != InFile::READ_ERROR);
  }
};

// Model a subfile-open failure (for example, failure to reopen a stdio file)
// after one operator has already been loaded successfully.
class FailingBlobIn : public GenericIn
{
  GenericInP input;
  int blob_opens = 0;
public:
  explicit FailingBlobIn (GenericInP input) : input (input) {}
  int get_byte() override { return input->get_byte(); }
  size_t read (void *ptr, size_t size) override { return input->read (ptr, size); }
  bool skip (size_t size) override { return input->skip (size); }
  size_t get_pos() override { return input->get_pos(); }
  size_t get_remaining() override { return input->get_remaining(); }
  const unsigned char *mmap_mem (size_t& remaining) override { return input->mmap_mem (remaining); }
  GenericInP open_subfile (size_t pos, size_t len) override
  {
    if (++blob_opens == 2)
      return nullptr;
    return input->open_subfile (pos, len);
  }
  int open_count() const { return blob_opens; }
};

int
main (int argc, char **argv)
{
  Main main (&argc, &argv);
  FFT::debug_in_test_program (true);
  Project project;
  project.set_mix_freq (48000);
  MorphPlan plan (project);
  plan.add_operator (MorphOperator::create ("SpectMorph::MorphOutput", &plan));

  const auto types = MorphOperator::list_types();
  assert (!types.empty());
  assert (!MorphOperator::create ("SpectMorph::Unknown", &plan));
  std::set<string> seen_types;
  for (const auto& type : types)
    {
      assert (seen_types.insert (type).second);
      std::unique_ptr<MorphOperator> op (MorphOperator::create (type, &plan));
      assert (op && op->type() == type);
      const auto data = operator_data (*op);
      {
        InFile in (MMapIn::open_vector (data));
        assert (op->load (in));
        assert (in.event() == InFile::END_OF_FILE);
      }
      // Covers partial headers, fields, and the missing final terminator.
      for (size_t size : { size_t (0), size_t (1), data.size() / 2, data.size() - 1 })
        {
          vector<unsigned char> truncated (data.begin(), data.begin() + size);
          std::unique_ptr<MorphOperator> partial (MorphOperator::create (type, &plan));
          InFile in (MMapIn::open_vector (truncated));
          assert (!partial->load (in));
          assert (in.event() == InFile::READ_ERROR);

          auto malformed = plan_data (type, truncated);
          auto previous = save_plan (plan);
          assert (plan.load (MMapIn::open_vector (malformed)).code() == Error::Code::PARSE_ERROR);
          assert (save_plan (plan) == previous);
        }

      // Valid duplicate payloads exercise both BLOB and BLOB_REF loading.
      auto valid = plan_data (type, data);
      assert (!plan.load (MMapIn::open_vector (valid)));
      assert (plan.operators().size() == 2);
      auto previous = save_plan (plan);
      auto missing_ref = plan_data (type, data, true);
      assert (plan.load (MMapIn::open_vector (missing_ref)).code() == Error::Code::PARSE_ERROR);
      assert (save_plan (plan) == previous);
    }

  // Use the actual plan writer, including repeated payloads (BLOB_REF), and
  // check every suffix truncation without constructing invalid field values.
  MorphPlan written_plan (project);
  for (const auto& type : types)
    for (int i = 0; i < 2; i++)
      written_plan.add_operator (MorphOperator::create (type, &written_plan));
  const auto written = save_plan (written_plan);
  const auto before_load = save_plan (plan);
  for (size_t size = 0; size < written.size(); size++)
    {
      vector<unsigned char> truncated (written.begin(), written.begin() + size);
      assert (plan.load (MMapIn::open_vector (truncated)).code() == Error::Code::PARSE_ERROR);
      assert (save_plan (plan) == before_load);
    }

  auto failing_input = std::make_shared<FailingBlobIn> (MMapIn::open_vector (written));
  assert (plan.load (failing_input).code() == Error::Code::PARSE_ERROR);
  assert (failing_input->open_count() == 2);
  assert (save_plan (plan) == before_load);

  assert (!plan.load (MMapIn::open_vector (written)));
  assert (save_plan (plan) == written);

  // Unknown fields retain the existing compatibility behavior.
  vector<unsigned char> unknown;
  {
    OutFile out (MemOut::open (&unknown), "SpectMorph::MorphSource", SPECTMORPH_BINARY_FILE_VERSION);
    out.write_string ("future_field", "value");
  }
  auto compatible = plan_data ("SpectMorph::MorphSource", unknown);
  assert (!plan.load (MMapIn::open_vector (compatible)));

  vector<unsigned char> truncated_extra;
  {
    OutFile out (MemOut::open (&truncated_extra), "SpectMorph::MorphPlan", SPECTMORPH_BINARY_FILE_VERSION);
    out.begin_section ("extra");
    out.write_string ("setting", "value");
  }
  truncated_extra.pop_back(); // remove terminator while inside the extra section
  ExtraParameters extra;
  auto previous = save_plan (plan);
  assert (plan.load (MMapIn::open_vector (truncated_extra), &extra).code() == Error::Code::PARSE_ERROR);
  assert (save_plan (plan) == previous);

  // Project loading must report the nested failure and restore its plan too.
  auto blob = operator_data (*plan.operators()[0]);
  blob.pop_back();
  ZipWriter zip;
  zip.add ("plan.smplan", plan_data ("SpectMorph::MorphSource", blob));
  zip.close();
  assert (!zip.error());
  ZipReader reader (zip.data());
  auto project_previous = save_plan (*project.morph_plan());
  assert (project.load (reader, nullptr).code() == Error::Code::PARSE_ERROR);
  assert (save_plan (*project.morph_plan()) == project_previous);
}
