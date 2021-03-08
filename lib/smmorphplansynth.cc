// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmorphplansynth.hh"
#include "smmorphplanvoice.hh"
#include "smleakdebugger.hh"

using namespace SpectMorph;

using std::map;
using std::vector;
using std::string;

static LeakDebugger leak_debugger ("SpectMorph::MorphPlanSynth");

MorphPlanSynth::MorphPlanSynth (float mix_freq, size_t n_voices) :
  m_mix_freq (mix_freq)
{
  leak_debugger.add (this);

  for (size_t i = 0; i < n_voices; i++)
    voices.push_back (new MorphPlanVoice (m_mix_freq, this));
}

MorphPlanSynth::~MorphPlanSynth()
{
  leak_debugger.del (this);

  free_shared_state();

  for (size_t i = 0; i < voices.size(); i++)
    delete voices[i];

  voices.clear();
}

MorphPlanVoice *
MorphPlanSynth::voice (size_t i) const
{
  g_return_val_if_fail (i < voices.size(), nullptr);

  return voices[i];
}

static vector<string>
sorted_id_list (MorphPlanPtr plan)
{
  vector<string> ids;

  if (plan)
    {
      const vector<MorphOperator *>& ops = plan->operators();
      for (vector<MorphOperator *>::const_iterator oi = ops.begin(); oi != ops.end(); oi++)
        {
          ids.push_back ((*oi)->id());
        }
      sort (ids.begin(), ids.end());
    }
  return ids;
}

static bool
recursive_cycle_check (MorphOperator *start_op, int depth)
{
  /* check if processing would fail due to cycles
   *
   * this check should avoid crashes in this situation, although no audio will be produced
   */
  if (depth > 500)
    return true;

  for (auto op : start_op->dependencies())
    if (op && recursive_cycle_check (op, depth + 1))
      return true;

  return false;
}

MorphPlanSynth::UpdateP
MorphPlanSynth::prepare_update (MorphPlanPtr plan) /* main thread */
{
  UpdateP update = std::make_shared<Update>();

  update->have_cycle = false;
  for (auto op : plan->operators())
    {
      if (recursive_cycle_check (op, 0))
        update->have_cycle = true;
    }

  for (auto o : plan->operators())
    {
      MorphOperatorConfigP config (o->clone_config());
      update->new_configs.push_back (config);

      Update::Op op = {
        .ptr_id = o->ptr_id(),
        .type   = o->type(),
        .config = config.get()
      };
      update->ops.push_back (op);
    }
  sort (update->ops.begin(), update->ops.end(),
        [](const Update::Op& a, const Update::Op& b) { return a.ptr_id < b.ptr_id; });

  vector<string> update_ids = sorted_id_list (plan);

  update->cheap = (update_ids == m_last_update_ids) && (plan->id() == m_last_plan_id);
  m_last_update_ids = update_ids;
  m_last_plan_id = plan->id();

  return update;
}

void
MorphPlanSynth::apply_update (MorphPlanSynth::UpdateP update) /* audio thread */
{
  /* life time for configs:
   *  - configs required for current update should be kept alive (m_active_configs)
   *  - configs no longer needed should be freed, but not in audio thread
   */
  update->old_configs = std::move (m_active_configs);
  m_active_configs = std::move (update->new_configs);
  m_have_cycle = update->have_cycle;

  if (update->cheap)
    {
      for (size_t i = 0; i < voices.size(); i++)
        voices[i]->cheap_update (update);
    }
  else
    {
      free_shared_state();

      for (size_t i = 0; i < voices.size(); i++)
        voices[i]->full_update (update);
    }
}

void
MorphPlanSynth::update_shared_state (const TimeInfo& time_info)
{
  if (voices.empty())
    return;
  voices[0]->update_shared_state (time_info);
}

MorphModuleSharedState *
MorphPlanSynth::shared_state (MorphOperator::PtrID ptr_id)
{
  return m_shared_state[ptr_id];
}

void
MorphPlanSynth::set_shared_state (MorphOperator::PtrID ptr_id, MorphModuleSharedState *shared_state)
{
  m_shared_state[ptr_id] = shared_state;
}

float
MorphPlanSynth::mix_freq() const
{
  return m_mix_freq;
}

Random *
MorphPlanSynth::random_gen()
{
  return &m_random_gen;
}

bool
MorphPlanSynth::have_output() const
{
  if (voices.empty())
    return false;

  // all voices are the same: either each of them contains an output module, or none of them
  return voices[0]->output() != nullptr;
}

bool
MorphPlanSynth::have_cycle() const
{
  return m_have_cycle;
}

void
MorphPlanSynth::free_shared_state()
{
  for (auto si = m_shared_state.begin(); si != m_shared_state.end(); si++)
    delete si->second;
  m_shared_state.clear();
}
