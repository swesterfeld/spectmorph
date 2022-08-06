// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include <clap/helpers/plugin.hh>
#include <clap/helpers/plugin.hxx>
#include <clap/helpers/host-proxy.hh>
#include <clap/helpers/host-proxy.hxx>
#include <cstring>
#include <algorithm>
#include <array>

#include "smmain.hh"
#include "smproject.hh"
#include "smmidisynth.hh"
#include "smsynthinterface.hh"
#include "smmorphplanwindow.hh"
#include "smeventloop.hh"

#define CLAP_DEBUG(...) Debug::debug ("clap", __VA_ARGS__)

namespace SpectMorph
{

const char *features[] = {CLAP_PLUGIN_FEATURE_INSTRUMENT, CLAP_PLUGIN_FEATURE_SYNTHESIZER, nullptr};
clap_plugin_descriptor clap_plugin_desc = {CLAP_VERSION,
                                            "org.spectmorph.spectmorph",
                                            "SpectMorph",
                                            "Stefan Westerfeld",
                                            "https://www.spectmorph.org",
                                            "",
                                            "",
                                            "XXX1.0.0",
                                            "Spectral Audio Morphing.",
                                            features};

class ClapPlugin;

class ClapUI final : public SignalReceiver
{
  std::unique_ptr<EventLoop>       event_loop;
  std::unique_ptr<MorphPlanWindow> window;
  MorphPlanPtr                     morph_plan;
  ClapPlugin                      *plugin;
public:
  ClapUI (MorphPlanPtr plan, ClapPlugin *plugin) :
    morph_plan (plan),
    plugin (plugin)
  {
  }
  void
  set_parent (PuglNativeWindow win_id)
  {
    event_loop.reset (new EventLoop());
    window.reset (new MorphPlanWindow (*event_loop, "SpectMorph VST", win_id, false, morph_plan));
    connect (window->signal_update_size, this, &ClapUI::on_update_window_size);

    window->show();
  }
  void on_update_window_size();

  void
  idle()
  {
    if (event_loop)
      event_loop->process_events();
  }
};

class ClapPlugin final : public clap::helpers::Plugin<clap::helpers::MisbehaviourHandler::Terminate,
                                                      clap::helpers::CheckingLevel::Maximal>
{
  Project project;
public:
  ClapPlugin (const clap_host *host) :
    clap::helpers::Plugin<clap::helpers::MisbehaviourHandler::Terminate,
    clap::helpers::CheckingLevel::Maximal> (&clap_plugin_desc, host)
  {
    for (size_t index = 0; index < parameters.size(); index++)
      {
        clap_param_info info = { 0, };
        paramsInfo (index, &info);
        parameters[index] = info.default_value;
      }
  }

  /*--- ports --- */
  bool
  implementsAudioPorts() const noexcept override
  {
    return true;
  }
  uint32_t
  audioPortsCount (bool isInput) const noexcept override
  {
    return isInput ? 0 : 1;
  }
  bool
  audioPortsInfo (uint32_t index, bool isInput, clap_audio_port_info *info) const noexcept override
  {
    if (isInput || index != 0)
      return false;

    info->id = 0;
    info->in_place_pair = CLAP_INVALID_ID;
    strncpy(info->name, "main", sizeof(info->name));
    info->flags = CLAP_AUDIO_PORT_IS_MAIN;
    info->channel_count = 2;
    info->port_type = CLAP_PORT_STEREO;
    return true;
  }
  bool
  implementsNotePorts() const noexcept override
  {
    return true;
  }
  uint32_t
  notePortsCount (bool isInput) const noexcept override
  {
    return isInput ? 1 : 0;
  }
  bool
  notePortsInfo (uint32_t index, bool isInput, clap_note_port_info *info) const noexcept override
  {
    if (isInput)
      {
        info->id = 1;
        info->supported_dialects = CLAP_NOTE_DIALECT_MIDI | CLAP_NOTE_DIALECT_CLAP;
        info->preferred_dialect = CLAP_NOTE_DIALECT_CLAP;
        strncpy(info->name, "NoteInput", CLAP_NAME_SIZE);
        return true;
      }
    return false;
  }
  /*--- params --- */
  static constexpr int FIRST_PARAM_ID = 100;
  static constexpr int PARAM_COUNT = 4;
  std::array<double, PARAM_COUNT> parameters;
  bool
  implementsParams() const noexcept override
  {
    return true;
  }
  uint32_t
  paramsCount() const noexcept override
  {
    return parameters.size();
  }
  bool
  isValidParamId (clap_id paramId) const noexcept override
  {
    return paramId >= FIRST_PARAM_ID && paramId < FIRST_PARAM_ID + parameters.size();
  }
  bool
  paramsInfo (uint32_t paramIndex, clap_param_info *info) const noexcept override
  {
    if (paramIndex >= parameters.size())
      return false;

    info->flags = CLAP_PARAM_IS_AUTOMATABLE;
    info->id = paramIndex + FIRST_PARAM_ID;
    strncpy (info->name, string_printf ("Control #%d", paramIndex + 1).c_str(), CLAP_NAME_SIZE);
    strncpy (info->module, "Controls", CLAP_NAME_SIZE);
    info->min_value = -1;
    info->max_value = 1;
    info->default_value = 0;
    return true;
  }
  bool
  paramsValueToText (clap_id paramId, double value, char *display, uint32_t size) noexcept override
  {
    if (!isValidParamId (paramId))
      return false;

    strncpy (display, string_printf ("%.5f", value).c_str(), size);
    return true;
  }
  bool
  paramsValue (clap_id paramId, double *value) noexcept override
  {
    if (!isValidParamId (paramId))
      return false;

    *value = parameters[paramId - FIRST_PARAM_ID];
    return true;
  }
  void
  paramsFlush (const clap_input_events *in, const clap_output_events *out) noexcept override
  {
    auto sz = in->size (in);

    for (uint index = 0; index < sz; index++)
      {
        auto event = in->get (in, index);

        if (event->type == CLAP_EVENT_PARAM_VALUE)
          {
            auto v = reinterpret_cast<const clap_event_param_value *> (event);

            if (isValidParamId (v->param_id))
              {
                auto index = v->param_id - FIRST_PARAM_ID;
                parameters[index] = v->value;
                project.midi_synth()->set_control_input (index, v->value);
                CLAP_DEBUG ("paramsFlush: set %d to %f\n", index, v->value);
              }
          }
      }
  }
  /*--- processing ---*/
  bool
  activate (double sampleRate, uint32_t minFrameCount, uint32_t maxFrameCount) noexcept override
  {
    project.set_mix_freq (sampleRate);
    return true;
  }
  clap_process_status
  process (const clap_process *process) noexcept override
  {
    // If I have no outputs, do nothing
    if (process->audio_outputs_count <= 0)
      return CLAP_PROCESS_SLEEP;

    // update plan with new parameters / new modules if necessary
    project.try_update_synth();

    MidiSynth *midi_synth = project.midi_synth();

    float **outputs = process->audio_outputs[0].data32;
    auto ev = process->in_events;
    auto sz = ev->size (ev);

    for (uint32 index = 0; index < sz; index++)
      {
        auto event = ev->get (ev, index);
        if (event->space_id != CLAP_CORE_EVENT_SPACE_ID)
          continue;

        if (event->type == CLAP_EVENT_MIDI)
          {
            auto midi_event = reinterpret_cast<const clap_event_midi *> (event);

            midi_synth->add_midi_event (event->time, midi_event->data);
          }
        else if (event->type == CLAP_EVENT_NOTE_ON)
          {
            auto note_event = reinterpret_cast<const clap_event_note *>(event);

            unsigned char midi[3];
            midi[0] = 0x90 + note_event->channel;
            midi[1] = note_event->key;
            midi[2] = sm_clamp<int> (note_event->velocity * 127, 0, 127);

            midi_synth->add_midi_event (event->time, midi);
          }
        else if (event->type == CLAP_EVENT_NOTE_OFF)
          {
            auto note_event = reinterpret_cast<const clap_event_note *>(event);

            unsigned char midi[3];
            midi[0] = 0x80 + note_event->channel;
            midi[1] = note_event->key;
            midi[2] = sm_clamp<int> (note_event->velocity * 127, 0, 127);

            midi_synth->add_midi_event (event->time, midi);
          }
        else if (event->type == CLAP_EVENT_PARAM_VALUE)
          {
            auto v = reinterpret_cast<const clap_event_param_value *> (event);

            if (isValidParamId (v->param_id))
              {
                /* FIXME: not sample accurate */
                auto index = v->param_id - FIRST_PARAM_ID;
                parameters[index] = v->value;
                midi_synth->set_control_input (index, v->value);
                CLAP_DEBUG ("process: set %d to %f\n", index, v->value);
              }
          }
      }
    midi_synth->process (outputs[0], process->frames_count);

    std::copy (outputs[0], outputs[0] + process->frames_count, outputs[1]);
    /* this can be optimized */
    return CLAP_PROCESS_CONTINUE;
  }
  /*--- gui --- */
  std::unique_ptr<ClapUI> ui;
  bool
  implementsGui() const noexcept override
  {
    return true;
  }
  bool
  guiIsApiSupported (const char *api, bool isFloating) noexcept override
  {
    if (isFloating)
      return false;

#ifdef SM_OS_LINUX
    if (strcmp (api, CLAP_WINDOW_API_X11) == 0)
      return true;
#endif
    /* FIXME: non-linux support */

    return false;
  }
  bool
  guiCreate (const char *api, bool isFloating) noexcept override
  {
    if (!ui)
      {
        clap_id id;
        _host.timerSupportRegister (16, &id);
      }
    ui.reset (new ClapUI (project.morph_plan(), this));
    return ui != nullptr;
  }
  void
  guiDestroy() noexcept override
  {
    ui.reset (nullptr);
  }
  bool
  guiSetParent (const clap_window *window) noexcept override
  {
    ui->set_parent (window->x11);
    return true;
  }
  bool
  guiGetSize (uint32_t *width, uint32_t *height) noexcept override
  {
    int w, h;
    MorphPlanWindow::static_scaled_size (&w, &h);
    *width = w;
    *height = h;
    return true;
  }
  void
  updateWindowSize (int width, int height)
  {
    if (_host.canUseGui())
      {
        _host.guiRequestResize (width, height);
      }
  }
  bool
  implementsTimerSupport() const noexcept override
  {
    return true;
  }
  void
  onTimer (clap_id timerId) noexcept override
  {
    if (ui)
      ui->idle();
  }
};

void
ClapUI::on_update_window_size()
{
  int width, height;
  window->get_scaled_size (&width, &height);

  plugin->updateWindowSize (width, height);
}

uint32_t clap_get_plugin_count(const clap_plugin_factory *f) { return 1; }
const clap_plugin_descriptor *clap_get_plugin_descriptor(const clap_plugin_factory *f, uint32_t w)
{
  return &clap_plugin_desc;
}

static const clap_plugin *clap_create_plugin(const clap_plugin_factory *f, const clap_host *host,
                                             const char *plugin_id)
{
  if (strcmp (plugin_id, clap_plugin_desc.id))
    {
      fprintf (stderr, "SpectMorph: CLAP asked for plugin_id '%s' and our plugin ID is '%s'\n", plugin_id, clap_plugin_desc.id);
      return nullptr;
    }
  // I know it looks like a leak right? but the clap-plugin-helpers basically
  // take ownership and destroy the wrapper when the host destroys the
  // underlying plugin (look at Plugin<h, l>::clapDestroy if you don't believe me!)
  auto p = new ClapPlugin (host);
  return p->clapPlugin();
}

const struct clap_plugin_factory clap_factory = {
  clap_get_plugin_count,
  clap_get_plugin_descriptor,
  clap_create_plugin,
};

static const void *get_factory(const char *factory_id) { return &clap_factory; }

bool
clap_init (const char *p)
{
  Debug::set_filename ("smclapplugin.log");

  sm_plugin_init();

#ifdef SM_STATIC_LINUX
  set_static_linux_data_dir();
#endif

  return true;
}

void clap_deinit()
{
  sm_plugin_cleanup();
}

}

extern "C" {

const CLAP_EXPORT struct clap_plugin_entry clap_entry = {
   CLAP_VERSION,
   SpectMorph::clap_init,
   SpectMorph::clap_deinit,
   SpectMorph::get_factory
};

}
