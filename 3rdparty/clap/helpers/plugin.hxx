#include <cassert>
#include <cstring>
#include <exception>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <utility>

#include "host-proxy.hxx"
#include "plugin.hh"

namespace clap { namespace helpers {

   template <MisbehaviourHandler h, CheckingLevel l>
   const clap_plugin_render Plugin<h, l>::_pluginRender = {
      clapRenderHasHardRealtimeRequirement,
      clapRenderSetMode,
   };

   template <MisbehaviourHandler h, CheckingLevel l>
   const clap_plugin_thread_pool Plugin<h, l>::_pluginThreadPool = {
      clapThreadPoolExec,
   };

   template <MisbehaviourHandler h, CheckingLevel l>
   const clap_plugin_state Plugin<h, l>::_pluginState = {
      clapStateSave,
      clapStateLoad,
   };

   template <MisbehaviourHandler h, CheckingLevel l>
   const clap_plugin_state_context Plugin<h, l>::_pluginStateContext = {
      clapStateContextSave,
      clapStateContextLoad,
   };

   template <MisbehaviourHandler h, CheckingLevel l>
   const clap_plugin_preset_load Plugin<h, l>::_pluginPresetLoad = {
      clapPresetLoadFromLocation,
   };

   template <MisbehaviourHandler h, CheckingLevel l>
   const clap_plugin_track_info Plugin<h, l>::_pluginTrackInfo = {
      clapTrackInfoChanged,
   };

   template <MisbehaviourHandler h, CheckingLevel l>
   const clap_plugin_audio_ports Plugin<h, l>::_pluginAudioPorts = {clapAudioPortsCount,
                                                                    clapAudioPortsInfo};

   template <MisbehaviourHandler h, CheckingLevel l>
   const clap_plugin_audio_ports_config Plugin<h, l>::_pluginAudioPortsConfig = {
      clapAudioPortsConfigCount,
      clapAudioPortsGetConfig,
      clapAudioPortsSetConfig,
   };

   template <MisbehaviourHandler h, CheckingLevel l>
   const clap_plugin_audio_ports_activation Plugin<h, l>::_pluginAudioPortsActivation = {
      clapAudioPortsActivationCanActivateWhileProcessing,
      clapAudioPortsActivationSetActive,
   };

   template <MisbehaviourHandler h, CheckingLevel l>
   const clap_plugin_params Plugin<h, l>::_pluginParams = {
      clapParamsCount,
      clapParamsInfo,
      clapParamsValue,
      clapParamsValueToText,
      clapParamsTextToValue,
      clapParamsFlush,
   };

   template <MisbehaviourHandler h, CheckingLevel l>
   const clap_plugin_param_indication Plugin<h, l>::_pluginParamIndication = {
      clapParamIndicationSetMapping,
      clapParamIndicationSetAutomation,
   };

   template <MisbehaviourHandler h, CheckingLevel l>
   const clap_plugin_remote_controls Plugin<h, l>::_pluginRemoteControls = {
      clapRemoteControlsPageCount, clapRemoteControlsPageGet};

   template <MisbehaviourHandler h, CheckingLevel l>
   const clap_plugin_latency Plugin<h, l>::_pluginLatency = {
      clapLatencyGet,
   };

   template <MisbehaviourHandler h, CheckingLevel l>
   const clap_plugin_note_ports Plugin<h, l>::_pluginNotePorts = {clapNotePortsCount,
                                                                  clapNotePortsInfo};

   template <MisbehaviourHandler h, CheckingLevel l>
   const clap_plugin_note_name Plugin<h, l>::_pluginNoteName = {
      clapNoteNameCount,
      clapNoteNameGet,
   };

   template <MisbehaviourHandler h, CheckingLevel l>
   const clap_plugin_timer_support Plugin<h, l>::_pluginTimerSupport = {clapOnTimer};

   template <MisbehaviourHandler h, CheckingLevel l>
   const clap_plugin_posix_fd_support Plugin<h, l>::_pluginPosixFdSupport = {
      clapOnPosixFd,
   };

   template <MisbehaviourHandler h, CheckingLevel l>
   const clap_plugin_gui Plugin<h, l>::_pluginGui = {
      clapGuiIsApiSupported,
      clapGuiGetPreferredApi,
      clapGuiCreate,
      clapGuiDestroy,
      clapGuiSetScale,
      clapGuiGetSize,
      clapGuiCanResize,
      clapGuiGetResizeHints,
      clapGuiAdjustSize,
      clapGuiSetSize,
      clapGuiSetParent,
      clapGuiSetTransient,
      clapGuiSuggestTitle,
      clapGuiShow,
      clapGuiHide,
   };

   template <MisbehaviourHandler h, CheckingLevel l>
   const clap_plugin_context_menu Plugin<h, l>::_pluginContextMenu = {
      clapContextMenuPopulate,
      clapContextMenuPerform,
   };

   template <MisbehaviourHandler h, CheckingLevel l>
   const clap_plugin_resource_directory Plugin<h, l>::_pluginResourceDirectory = {
      clapResourceDirectorySetDirectory,
      clapResourceDirectoryCollect,
      clapResourceDirectoryGetFilesCount,
      clapResourceDirectoryGetFilePath,
   };

   template <MisbehaviourHandler h, CheckingLevel l>
   const clap_plugin_voice_info Plugin<h, l>::_pluginVoiceInfo = {
      clapVoiceInfoGet,
   };

   template <MisbehaviourHandler h, CheckingLevel l>
   const clap_plugin_tail Plugin<h, l>::_pluginTail = {
      clapTailGet,
   };

   template <MisbehaviourHandler h, CheckingLevel l>
   const clap_plugin_undo Plugin<h, l>::_pluginUndo = {
      clapUndoGetDeltaProperties,
      clapUndoCanUseDeltaFormatVersion,
      clapUndoApplyDelta,
      clapUndoSetContextInfo,
   };

   template <MisbehaviourHandler h, CheckingLevel l>
   Plugin<h, l>::Plugin(const clap_plugin_descriptor *desc, const clap_host *host) : _host(host) {
      _plugin.plugin_data = this;
      _plugin.desc = desc;
      _plugin.init = Plugin<h, l>::clapInit;
      _plugin.destroy = Plugin<h, l>::clapDestroy;
      _plugin.get_extension = Plugin<h, l>::clapExtension;
      _plugin.process = Plugin<h, l>::clapProcess;
      _plugin.activate = Plugin<h, l>::clapActivate;
      _plugin.deactivate = Plugin<h, l>::clapDeactivate;
      _plugin.start_processing = Plugin<h, l>::clapStartProcessing;
      _plugin.stop_processing = Plugin<h, l>::clapStopProcessing;
      _plugin.reset = Plugin<h, l>::clapReset;
      _plugin.on_main_thread = Plugin<h, l>::clapOnMainThread;
   }

   /////////////////////
   // CLAP Interfaces //
   /////////////////////

   //-------------//
   // clap_plugin //
   //-------------//
   template <MisbehaviourHandler h, CheckingLevel l>
   bool Plugin<h, l>::clapInit(const clap_plugin *plugin) noexcept {
      auto &self = from(plugin, false);

      if (l >= CheckingLevel::Minimal && self._wasInitialized) {
         std::cerr << "clap_plugin.init() has been called twice" << std::endl;
         if (h == MisbehaviourHandler::Terminate)
            std::terminate();
         return true;
      }

      self._wasInitialized = true;

      self._host.init();
      self.ensureMainThread("clap_plugin.init");
      return self.init();
   }

   template <MisbehaviourHandler h, CheckingLevel l>
   void Plugin<h, l>::ensureInitialized(const char *method) const noexcept {
      if (l == CheckingLevel::None || _wasInitialized)
         return;

      std::cerr << "clap_plugin." << method << "() was called before clap_plugin.init()"
                << std::endl;

      if (h == MisbehaviourHandler::Terminate)
         std::terminate();
   }

   template <MisbehaviourHandler h, CheckingLevel l>
   void Plugin<h, l>::clapDestroy(const clap_plugin *plugin) noexcept {
      auto &self = from(plugin, false);
      self.ensureMainThread("clap_plugin.destroy");
      self._isBeingDestroyed = true;

      if (self._isGuiCreated) {
         if (l >= CheckingLevel::Minimal)
            self._host.hostMisbehaving("host forgot to destroy the gui");
         clapGuiDestroy(plugin);
      }

      if (self._isActive) {
         if (l >= CheckingLevel::Minimal)
            self._host.hostMisbehaving("host forgot to deactivate the plugin before destroying it");
         clapDeactivate(plugin);
      }
      assert(!self._isActive);

      self.runCallbacksOnMainThread();

      delete &self;
   }

   template <MisbehaviourHandler h, CheckingLevel l>
   void Plugin<h, l>::clapOnMainThread(const clap_plugin *plugin) noexcept {
      auto &self = from(plugin);
      self.ensureInitialized("on_maint_thread");
      self.ensureMainThread("clap_plugin.on_main_thread");

      self.runCallbacksOnMainThread();
      self.onMainThread();
   }

   template <MisbehaviourHandler h, CheckingLevel l>
   void Plugin<h, l>::runCallbacksOnMainThread() {
      if (l >= CheckingLevel::Minimal) {
         if (_host.canUseThreadCheck() && !_host.isMainThread()) {
            _host.pluginMisbehaving(
               "plugin called runCallbacksOnMainThread(), but not on the main thread!");
            return;
         }
      }

      while (true) {
         std::function<void()> cb;

         {
            std::lock_guard<std::mutex> guard(_mainThreadCallbacksLock);
            if (_mainThreadCallbacks.empty())
               return;
            cb = std::move(_mainThreadCallbacks.front());
            _mainThreadCallbacks.pop();
         }

         if (cb)
            cb();
      }
   }

   template <MisbehaviourHandler h, CheckingLevel l>
   bool Plugin<h, l>::clapActivate(const clap_plugin *plugin,
                                   double sample_rate,
                                   uint32_t minFrameCount,
                                   uint32_t maxFrameCount) noexcept {
      auto &self = from(plugin);
      self.ensureInitialized("activate");
      self.ensureMainThread("clap_plugin.activate");

      if (l >= CheckingLevel::Minimal) {
         if (self._isActive) {
            self.hostMisbehaving("Plugin was activated twice");

            if (sample_rate != self._sampleRate) {
               std::ostringstream msg;
               msg << "The plugin was activated twice and with different sample rates: "
                   << self._sampleRate << " and " << sample_rate
                   << ". The host must deactivate the plugin first." << std::endl
                   << "Simulating deactivation.";
               self.hostMisbehaving(msg.str());
               clapDeactivate(plugin);
            }
         }

         if (sample_rate <= 0) {
            std::ostringstream msg;
            msg << "The plugin was activated with an invalid sample rates: " << sample_rate;
            self.hostMisbehaving(msg.str());
            return false;
         }

         if (minFrameCount < 1) {
            std::ostringstream msg;
            msg << "The plugin was activated with an invalid minimum frame count: "
                << minFrameCount;
            self.hostMisbehaving(msg.str());
            return false;
         }

         if (maxFrameCount > INT32_MAX) {
            std::ostringstream msg;
            msg << "The plugin was activated with an invalid maximum frame count: "
                << maxFrameCount;
            self.hostMisbehaving(msg.str());
            return false;
         }

         if (minFrameCount > maxFrameCount) {
            std::ostringstream msg;
            msg << "The plugin was activated with an invalid minmum and maximum frame count: min > "
                   "max: "
                << minFrameCount << " > " << maxFrameCount;
            self.hostMisbehaving(msg.str());
            return false;
         }
      }

      assert(!self._isActive);
      assert(self._sampleRate == 0);

      self._isBeingActivated = true;
      if (!self.activate(sample_rate, minFrameCount, maxFrameCount)) {
         self._isBeingActivated = false;
         assert(!self._isActive);
         assert(self._sampleRate == 0);
         return false;
      }

      self._isBeingActivated = false;
      self._isActive = true;
      self._sampleRate = sample_rate;
      return true;
   }

   template <MisbehaviourHandler h, CheckingLevel l>
   void Plugin<h, l>::clapDeactivate(const clap_plugin *plugin) noexcept {
      auto &self = from(plugin);
      self.ensureInitialized("deactivate");
      self.ensureMainThread("clap_plugin.deactivate");

      if (l >= CheckingLevel::Minimal) {
         if (!self._isActive) {
            self.hostMisbehaving("The plugin was deactivated twice.");
            return;
         }
      }

      self.deactivate();
      self._isActive = false;
      self._sampleRate = 0;
   }

   template <MisbehaviourHandler h, CheckingLevel l>
   bool Plugin<h, l>::clapStartProcessing(const clap_plugin *plugin) noexcept {
      auto &self = from(plugin);
      self.ensureInitialized("start_processing");
      self.ensureAudioThread("clap_plugin.start_processing");

      if (l >= CheckingLevel::Minimal) {
         if (!self._isActive) {
            self.hostMisbehaving(
               "Host called clap_plugin.start_processing() on a deactivated plugin");
            return false;
         }

         if (self._isProcessing) {
            self.hostMisbehaving("Host called clap_plugin.start_processing() twice");
            return true;
         }
      }

      self._isProcessing = self.startProcessing();
      return self._isProcessing;
   }

   template <MisbehaviourHandler h, CheckingLevel l>
   void Plugin<h, l>::clapStopProcessing(const clap_plugin *plugin) noexcept {
      auto &self = from(plugin);
      self.ensureInitialized("stop_processing");
      self.ensureAudioThread("clap_plugin.stop_processing");

      if (l >= CheckingLevel::Minimal) {
         if (!self._isActive) {
            self.hostMisbehaving(
               "Host called clap_plugin.stop_processing() on a deactivated plugin");
            return;
         }

         if (!self._isProcessing) {
            self.hostMisbehaving("Host called clap_plugin.stop_processing() twice");
            return;
         }
      }

      self.stopProcessing();
      self._isProcessing = false;
   }

   template <MisbehaviourHandler h, CheckingLevel l>
   void Plugin<h, l>::clapReset(const clap_plugin *plugin) noexcept {
      auto &self = from(plugin);
      self.ensureInitialized("reset");
      self.ensureAudioThread("clap_plugin.reset");

      if (l >= CheckingLevel::Minimal) {
         if (!self._isActive) {
            self.hostMisbehaving("Host called clap_plugin.reset() on a deactivated plugin");
            return;
         }
      }

      self.reset();
   }

   template <MisbehaviourHandler h, CheckingLevel l>
   clap_process_status Plugin<h, l>::clapProcess(const clap_plugin *plugin,
                                                 const clap_process *process) noexcept {
      auto &self = from(plugin);
      self.ensureInitialized("process");
      self.ensureAudioThread("clap_plugin.process");

      if (l >= CheckingLevel::Minimal) {
         if (!self._isActive) {
            self.hostMisbehaving("Host called clap_plugin.process() on a deactivated plugin");
            return CLAP_PROCESS_ERROR;
         }

         if (!self._isProcessing) {
            self.hostMisbehaving(
               "Host called clap_plugin.process() without calling clap_plugin.start_processing()");
            return CLAP_PROCESS_ERROR;
         }
      }

      return self.process(process);
   }

   template <MisbehaviourHandler h, CheckingLevel l>
   const void *Plugin<h, l>::clapExtension(const clap_plugin *plugin, const char *id) noexcept {
      auto &self = from(plugin);
      self.ensureInitialized("extension");

      if (!strcmp(id, CLAP_EXT_STATE) && self.implementsState())
         return &_pluginState;
      if (!strcmp(id, CLAP_EXT_STATE_CONTEXT) && self.implementsStateContext() && self.implementsState())
         return &_pluginStateContext;
      if ((!strcmp(id, CLAP_EXT_PRESET_LOAD) || !strcmp(id, CLAP_EXT_PRESET_LOAD_COMPAT)) &&
          self.implementsPresetLoad())
         return &_pluginPresetLoad;
      if (!strcmp(id, CLAP_EXT_RENDER) && self.implementsRender())
         return &_pluginRender;
      if ((!strcmp(id, CLAP_EXT_TRACK_INFO) || !strcmp(id, CLAP_EXT_TRACK_INFO_COMPAT)) &&
          self.implementsTrackInfo())
         return &_pluginTrackInfo;
      if (!strcmp(id, CLAP_EXT_LATENCY) && self.implementsLatency())
         return &_pluginLatency;
      if (!strcmp(id, CLAP_EXT_AUDIO_PORTS) && self.implementsAudioPorts())
         return &_pluginAudioPorts;
      if ((!strcmp(id, CLAP_EXT_AUDIO_PORTS_ACTIVATION) ||
           !strcmp(id, CLAP_EXT_AUDIO_PORTS_ACTIVATION_COMPAT)) &&
          self.implementsAudioPorts())
         return &_pluginAudioPortsActivation;
      if (!strcmp(id, CLAP_EXT_AUDIO_PORTS_CONFIG) && self.implementsAudioPortsConfig())
         return &_pluginAudioPortsConfig;
      if (!strcmp(id, CLAP_EXT_PARAMS) && self.implementsParams())
         return &_pluginParams;
      if ((!strcmp(id, CLAP_EXT_PARAM_INDICATION) ||
           !strcmp(id, CLAP_EXT_PARAM_INDICATION_COMPAT)) &&
          self.implementsParamIndication())
         return &_pluginParamIndication;
      if ((!strcmp(id, CLAP_EXT_REMOTE_CONTROLS) || !strcmp(id, CLAP_EXT_REMOTE_CONTROLS_COMPAT)) &&
          self.implementRemoteControls())
         return &_pluginRemoteControls;
      if (!strcmp(id, CLAP_EXT_NOTE_PORTS) && self.implementsNotePorts())
         return &_pluginNotePorts;
      if (!strcmp(id, CLAP_EXT_NOTE_NAME) && self.implementsNoteName())
         return &_pluginNoteName;
      if (!strcmp(id, CLAP_EXT_THREAD_POOL) && self.implementsThreadPool())
         return &_pluginThreadPool;
      if (!strcmp(id, CLAP_EXT_TIMER_SUPPORT) && self.implementsTimerSupport())
         return &_pluginTimerSupport;
      if (!strcmp(id, CLAP_EXT_POSIX_FD_SUPPORT) && self.implementsPosixFdSupport())
         return &_pluginPosixFdSupport;
      if (!strcmp(id, CLAP_EXT_GUI) && self.implementsGui())
         return &_pluginGui;
      if (!strcmp(id, CLAP_EXT_VOICE_INFO) && self.implementsVoiceInfo())
         return &_pluginVoiceInfo;
      if (!strcmp(id, CLAP_EXT_TAIL) && self.implementsTail())
         return &_pluginTail;
      if ((!strcmp(id, CLAP_EXT_CONTEXT_MENU) || !strcmp(id, CLAP_EXT_CONTEXT_MENU_COMPAT)) &&
          self.implementsContextMenu())
         return &_pluginContextMenu;

      if (self.enableDraftExtensions()) {
         if (!strcmp(id, CLAP_EXT_RESOURCE_DIRECTORY) && self.implementsResourceDirectory())
            return &_pluginResourceDirectory;
         if (!strcmp(id, CLAP_EXT_UNDO) && self.implementsUndo())
            return &_pluginUndo;
      }

      return self.extension(id);
   }

   //-------------------//
   // clap_plugin_state //
   //-------------------//
   template <MisbehaviourHandler h, CheckingLevel l>
   uint32_t Plugin<h, l>::clapLatencyGet(const clap_plugin *plugin) noexcept {
      auto &self = from(plugin);
      self.ensureMainThread("clap_plugin_latency.get");

      if (l >= CheckingLevel::Minimal) {
         if (!self._isActive && !self._isBeingActivated)
            self.hostMisbehaving("It is wrong to query the latency before the plugin is activated, "
                                 "because if the plugin dosen't know the sample rate, it can't "
                                 "know the number of samples of latency.");
      }

      return self.latencyGet();
   }

   //------------------//
   // clap_plugin_tail //
   //------------------//
   template <MisbehaviourHandler h, CheckingLevel l>
   uint32_t Plugin<h, l>::clapTailGet(const clap_plugin_t *plugin) noexcept {
      auto &self = from(plugin);

      if (l >= CheckingLevel::Minimal) {
         if (!self._isActive)
            self.hostMisbehaving("It is wrong to query the tail before the plugin is activated, "
                                 "because if the plugin dosen't know the sample rate, it can't "
                                 "know the tail length in samples.");
      }

      return self.tailGet();
   }

   //--------------------//
   // clap_plugin_render //
   //--------------------//
   template <MisbehaviourHandler h, CheckingLevel l>
   bool Plugin<h, l>::clapRenderHasHardRealtimeRequirement(const clap_plugin *plugin) noexcept {
      auto &self = from(plugin);
      self.ensureMainThread("clap_plugin_render.has_hard_realtime_requirement");
      return self.renderHasHardRealtimeRequirement();
   }

   template <MisbehaviourHandler h, CheckingLevel l>
   bool Plugin<h, l>::clapRenderSetMode(const clap_plugin *plugin,
                                        clap_plugin_render_mode mode) noexcept {
      auto &self = from(plugin);
      self.ensureMainThread("clap_plugin_render.set_mode");

      switch (mode) {
      case CLAP_RENDER_REALTIME:
         return self.renderSetMode(mode);

      case CLAP_RENDER_OFFLINE: {
         if (self.renderHasHardRealtimeRequirement())
            return false;
         return self.renderSetMode(mode);
      }

      default: {
         std::ostringstream msg;
         msg << "host called clap_plugin_render.set_mode with an unknown mode : " << mode;
         self.hostMisbehaving(msg.str());
         return false;
      }
      }
   }

   //-------------------------//
   // clap_plugin_thread_pool //
   //-------------------------//
   template <MisbehaviourHandler h, CheckingLevel l>
   void Plugin<h, l>::clapThreadPoolExec(const clap_plugin *plugin, uint32_t task_index) noexcept {
      auto &self = from(plugin);

      self.threadPoolExec(task_index);
   }

   //-------------------//
   // clap_plugin_state //
   //-------------------//
   template <MisbehaviourHandler h, CheckingLevel l>
   bool Plugin<h, l>::clapStateSave(const clap_plugin *plugin,
                                    const clap_ostream *stream) noexcept {
      auto &self = from(plugin);
      self.ensureMainThread("clap_plugin_state.save");

      return self.stateSave(stream);
   }

   template <MisbehaviourHandler h, CheckingLevel l>
   bool Plugin<h, l>::clapStateLoad(const clap_plugin *plugin,
                                    const clap_istream *stream) noexcept {
      auto &self = from(plugin);
      self.ensureMainThread("clap_plugin_state.load");

      return self.stateLoad(stream);
   }

   //---------------------------//
   // clap_plugin_state_context //
   //---------------------------//
   template <MisbehaviourHandler h, CheckingLevel l>
   bool Plugin<h, l>::clapStateContextSave(const clap_plugin *plugin,
                                           const clap_ostream *stream,
                                           uint32_t context) noexcept {
      auto &self = from(plugin);
      self.ensureMainThread("clap_plugin_state_context.save");

      return self.stateContextSave(stream, context);
   }

   template <MisbehaviourHandler h, CheckingLevel l>
   bool Plugin<h, l>::clapStateContextLoad(const clap_plugin *plugin,
                                           const clap_istream *stream,
                                           uint32_t context) noexcept {
      auto &self = from(plugin);
      self.ensureMainThread("clap_plugin_state_context.load");

      return self.stateContextLoad(stream, context);
   }

   //-------------------------//
   // clap_plugin_preset_load //
   //-------------------------//
   template <MisbehaviourHandler h, CheckingLevel l>
   bool Plugin<h, l>::clapPresetLoadFromLocation(const clap_plugin *plugin,
                                                 uint32_t location_kind,
                                                 const char *location,
                                                 const char *load_key) noexcept {
      auto &self = from(plugin);
      self.ensureMainThread("clap_plugin_preset_load.from_location");

      if (l >= CheckingLevel::Minimal) {
         if (location_kind == CLAP_PRESET_DISCOVERY_LOCATION_FILE && !location) {
            self.hostMisbehaving(
               "host called clap_plugin_preset_load.from_location with a null uri, for a preset "
               "with location_kind CLAP_PRESET_DISCOVERY_LOCATION_FILE");
            return false;
         }
      }

      return self.presetLoadFromLocation(location_kind, location, load_key);
   }

   //------------------------//
   // clap_plugin_track_info //
   //------------------------//
   template <MisbehaviourHandler h, CheckingLevel l>
   void Plugin<h, l>::clapTrackInfoChanged(const clap_plugin *plugin) noexcept {
      auto &self = from(plugin);
      self.ensureMainThread("clap_plugin_track_info.changed");

      if (l >= CheckingLevel::Minimal) {
         if (!self._host.canUseTrackInfo()) {
            self.hostMisbehaving(
               "host called clap_plugin_track_info.changed() but does not provide a "
               "complete clap_host_track_info interface");
            return;
         }
      }

      self.trackInfoChanged();
   }

   //-------------------------//
   // clap_plugin_audio_ports //
   //-------------------------//
   template <MisbehaviourHandler h, CheckingLevel l>
   uint32_t Plugin<h, l>::clapAudioPortsCount(const clap_plugin *plugin, bool is_input) noexcept {
      auto &self = from(plugin);
      self.ensureMainThread("clap_plugin_audio_ports.count");

      return self.audioPortsCount(is_input);
   }

   template <MisbehaviourHandler h, CheckingLevel l>
   bool Plugin<h, l>::clapAudioPortsInfo(const clap_plugin *plugin,
                                         uint32_t index,
                                         bool is_input,
                                         clap_audio_port_info *info) noexcept {
      auto &self = from(plugin);
      self.ensureMainThread("clap_plugin_audio_ports.info");

      if (l >= CheckingLevel::Minimal) {
         auto count = clapAudioPortsCount(plugin, is_input);
         if (index >= count) {
            std::ostringstream msg;
            msg << "Host called clap_plugin_audio_ports.info() with an index out of bounds: "
                << index << " >= " << count;
            self.hostMisbehaving(msg.str());
            return false;
         }
      }

      return self.audioPortsInfo(index, is_input, info);
   }

   template <MisbehaviourHandler h, CheckingLevel l>
   uint32_t Plugin<h, l>::clapAudioPortsConfigCount(const clap_plugin *plugin) noexcept {
      auto &self = from(plugin);
      self.ensureMainThread("clap_plugin_audio_ports.config_count");
      return self.audioPortsConfigCount();
   }

   template <MisbehaviourHandler h, CheckingLevel l>
   bool Plugin<h, l>::clapAudioPortsGetConfig(const clap_plugin *plugin,
                                              uint32_t index,
                                              clap_audio_ports_config *config) noexcept {
      auto &self = from(plugin);
      self.ensureMainThread("clap_plugin_audio_ports.get_config");

      if (l >= CheckingLevel::Minimal) {
         auto count = clapAudioPortsConfigCount(plugin);
         if (index >= count) {
            std::ostringstream msg;
            msg << "called clap_plugin_audio_ports.get_config with an index out of bounds: "
                << index << " >= " << count;
            self.hostMisbehaving(msg.str());
            return false;
         }
      }
      return self.audioPortsGetConfig(index, config);
   }

   template <MisbehaviourHandler h, CheckingLevel l>
   bool Plugin<h, l>::clapAudioPortsSetConfig(const clap_plugin *plugin,
                                              clap_id config_id) noexcept {
      auto &self = from(plugin);
      self.ensureMainThread("clap_plugin_audio_ports.get_config");

      if (l >= CheckingLevel::Minimal) {
         if (self.isActive())
            self.hostMisbehaving(
               "it is illegal to call clap_audio_ports.set_config if the plugin is active");
      }

      return self.audioPortsSetConfig(config_id);
   }

   template <MisbehaviourHandler h, CheckingLevel l>
   bool Plugin<h, l>::clapAudioPortsActivationCanActivateWhileProcessing(
      const clap_plugin_t *plugin) noexcept {
      auto &self = from(plugin);
      self.ensureMainThread("clap_plugin_audio_ports_activation.can_activate_while_processing");

      return self.audioPortsActivationCanActivateWhileProcessing();
   }

   template <MisbehaviourHandler h, CheckingLevel l>
   bool Plugin<h, l>::clapAudioPortsActivationSetActive(const clap_plugin_t *plugin,
                                                        bool is_input,
                                                        uint32_t port_index,
                                                        bool is_active,
                                                        uint32_t sample_size) noexcept {
      auto &self = from(plugin);
      self.ensureMainThread("clap_plugin_audio_ports_activation.set_active");

      if (l >= CheckingLevel::Minimal) {
         if (self.isActive() && !self.audioPortsActivationCanActivateWhileProcessing()) {
            self.hostMisbehaving(
               "it is illegal to call clap_audio_ports_activation.set_active() if the plugin is "
               "active if "
               "clap_plugin_audio_ports_activation.can_activate_while_processing() returns false");
         }
      }

      return self.audioPortsActivationSetActive(is_input, port_index, is_active, sample_size);
   }

   template <MisbehaviourHandler h, CheckingLevel l>
   uint32_t Plugin<h, l>::clapParamsCount(const clap_plugin *plugin) noexcept {
      auto &self = from(plugin);
      self.ensureMainThread("clap_plugin_params.count");

      return self.paramsCount();
   }

   //--------------------//
   // clap_plugin_params //
   //--------------------//
   template <MisbehaviourHandler h, CheckingLevel l>
   bool Plugin<h, l>::clapParamsInfo(const clap_plugin *plugin,
                                     uint32_t param_index,
                                     clap_param_info *param_info) noexcept {
      auto &self = from(plugin);
      self.ensureMainThread("clap_plugin_params.info");

      if (l >= CheckingLevel::Minimal) {
         auto count = clapParamsCount(plugin);
         if (param_index >= count) {
            std::ostringstream msg;
            msg << "called clap_plugin_params.info with an index out of bounds: " << param_index
                << " >= " << count;
            self.hostMisbehaving(msg.str());
            return false;
         }
      }

      const auto res = self.paramsInfo(param_index, param_info);

      if (l >= CheckingLevel::Maximal && !res) {
         std::ostringstream os;
         os << "clap_plugin_params.info(" << param_index << ") failed";
         self._host.pluginMisbehaving(os.str());
      }

      return res;
   }

   template <MisbehaviourHandler h, CheckingLevel l>
   bool Plugin<h, l>::clapParamsValue(const clap_plugin *plugin,
                                      clap_id paramId,
                                      double *value) noexcept {
      auto &self = from(plugin);
      self.ensureMainThread("clap_plugin_params.value");

      if (l >= CheckingLevel::Minimal) {
         if (!self.isValidParamId(paramId)) {
            std::ostringstream msg;
            msg << "clap_plugin_params.value called with invalid param_id: " << paramId;
            self.hostMisbehaving(msg.str());
            return false;
         }
      }

      const bool succeed = self.paramsValue(paramId, value);

      if (l >= CheckingLevel::Maximal) {
         clap_param_info info;
         if (self.getParamInfoForParamId(paramId, &info)) {
            if (*value < info.min_value || info.max_value < *value) {
               std::ostringstream msg;
               msg << "clap_plugin_params.value(" << paramId << ") = " << *value
                   << ", is out of range [" << info.min_value << " .. " << info.max_value << "]";
               self._host.pluginMisbehaving(msg.str());
            }
         }
      }

      return succeed;
   }

   template <MisbehaviourHandler h, CheckingLevel l>
   void Plugin<h, l>::clapParamsFlush(const clap_plugin *plugin,
                                      const clap_input_events *in,
                                      const clap_output_events *out) noexcept {
      auto &self = from(plugin);
      self.ensureParamThread("clap_plugin_params.flush");

      if (l >= CheckingLevel::Maximal) {
         if (!in)
            self.hostMisbehaving(
               "clap_plugin_params.flush called with an null input parameter change list");
         else {
            uint32_t N = in->size(in);
            for (uint32_t i = 0; i < N; ++i) {
               auto ev = in->get(in, i);
               if (!ev) {
                  std::ostringstream msg;
                  msg << "clap_plugin_params.flush called null event inside the input list at "
                         "index: "
                      << i;
                  self.hostMisbehaving(msg.str());
                  continue;
               }

               const bool isCoreSpace = ev->space_id == CLAP_CORE_EVENT_SPACE_ID;
               const bool isParamEvent =
                  ev->type == CLAP_EVENT_PARAM_VALUE || ev->type == CLAP_EVENT_PARAM_MOD;
               if (!isCoreSpace || !isParamEvent) {
                  std::ostringstream msg;
                  msg << "host called clap_plugin_params.flush() with space_id = " << ev->space_id
                      << ", and type = " << ev->type
                      << " but this one must only contain CLAP_EVENT_PARAM_VALUE or "
                         "CLAP_EVENT_PARAM_MOD"
                         " event type from CLAP_CORE_EVENT_SPACE_ID.";
                  self.hostMisbehaving(msg.str());
                  continue;
               }

               auto pev = reinterpret_cast<const clap_event_param_value *>(ev);

               if (self._host.canUseThreadCheck() && self._host.isMainThread() &&
                   !self.isValidParamId(pev->param_id)) {
                  std::ostringstream msg;
                  msg << "clap_plugin_params.flush called unknown paramId: " << pev->param_id;
                  self.hostMisbehaving(msg.str());
                  continue;
               }

               clap_param_info info;
               if (self.getParamInfoForParamId(pev->param_id, &info)) {
                  if (pev->value < info.min_value || info.max_value < pev->value) {
                     std::ostringstream msg;
                     msg << "clap_plugin_params.flush() produced the value " << pev->value
                         << " for parameter " << pev->param_id << " which is out of bounds: ["
                         << info.min_value << " .. " << info.max_value << "]";
                     self._host.pluginMisbehaving(msg.str());
                  }
               }
            }
         }

         if (!out)
            self.hostMisbehaving(
               "clap_plugin_params.flush called with an null output parameter change list");
      }

      self.paramsFlush(in, out);
   }

   template <MisbehaviourHandler h, CheckingLevel l>
   bool Plugin<h, l>::clapParamsValueToText(const clap_plugin *plugin,
                                            clap_id param_id,
                                            double value,
                                            char *display,
                                            uint32_t size) noexcept {
      auto &self = from(plugin);
      self.ensureMainThread("clap_plugin_params.value_to_text");

      if (l >= CheckingLevel::Minimal) {
         if (!self.isValidParamId(param_id)) {
            std::ostringstream msg;
            msg << "clap_plugin_params.value_to_text called with invalid param_id: " << param_id;
            self.hostMisbehaving(msg.str());
            return false;
         }

         if (l >= CheckingLevel::Maximal) {
            clap_param_info info;
            if (self.getParamInfoForParamId(param_id, &info)) {
               if (value < info.min_value || info.max_value < value) {
                  std::ostringstream msg;
                  msg << "clap_plugin_params.value_to_text() the value " << value
                      << " for parameter " << param_id << " is out of bounds: [" << info.min_value
                      << " .. " << info.max_value << "]";
                  self.hostMisbehaving(msg.str());
               }
            }
         }

         if (!display) {
            self.hostMisbehaving(
               "clap_plugin_params.value_to_text called with a null display pointer");
            return false;
         }

         if (size <= 1) {
            self.hostMisbehaving(
               "clap_plugin_params.value_to_text called with a empty buffer (less "
               "than one character)");
            return false;
         }
      }

      return self.paramsValueToText(param_id, value, display, size);
   }

   template <MisbehaviourHandler h, CheckingLevel l>
   bool Plugin<h, l>::clapParamsTextToValue(const clap_plugin *plugin,
                                            clap_id param_id,
                                            const char *display,
                                            double *value) noexcept {
      auto &self = from(plugin);
      self.ensureMainThread("clap_plugin_params.text_to_value");

      if (l >= CheckingLevel::Minimal) {
         if (!self.isValidParamId(param_id)) {
            std::ostringstream msg;
            msg << "clap_plugin_params.text_to_value called with invalid param_id: " << param_id;
            self.hostMisbehaving(msg.str());
            return false;
         }

         if (!display) {
            self.hostMisbehaving(
               "clap_plugin_params.text_to_value called with a null display pointer");
            return false;
         }

         if (!value) {
            self.hostMisbehaving(
               "clap_plugin_params.text_to_value called with a null value pointer");
            return false;
         }
      }

      if (!self.paramsTextToValue(param_id, display, value))
         return false;

      if (l >= CheckingLevel::Maximal) {
         clap_param_info info;
         if (self.getParamInfoForParamId(param_id, &info)) {
            if (*value < info.min_value || info.max_value < *value) {
               std::ostringstream msg;
               msg << "clap_plugin_params.text_to_value() produced the value " << value
                   << " for parameter " << param_id << " which is out of bounds: ["
                   << info.min_value << " .. " << info.max_value << "]";
               self._host.pluginMisbehaving(msg.str());
            }
         }
      }
      return true;
   }

   template <MisbehaviourHandler h, CheckingLevel l>
   int32_t Plugin<h, l>::getParamIndexForParamId(clap_id param_id) const noexcept {
      checkMainThread();

      const auto count = paramsCount();
      clap_param_info info;
      for (uint32_t i = 0; i < count; ++i) {
         if (!clapParamsInfo(&_plugin, i, &info))
            continue;

         if (info.id == param_id)
            return static_cast<int32_t>(i);
      }

      return -1;
   }

   template <MisbehaviourHandler h, CheckingLevel l>
   bool Plugin<h, l>::getParamInfoForParamId(clap_id paramId,
                                             clap_param_info *info) const noexcept {
      checkMainThread();

      const auto count = paramsCount();
      for (uint32_t i = 0; i < count; ++i)
         if (!clapParamsInfo(&_plugin, i, info) && info->id == paramId)
            return true;

      return false;
   }

   template <MisbehaviourHandler h, CheckingLevel l>
   bool Plugin<h, l>::isValidParamId(clap_id param_id) const noexcept {
      checkMainThread();

      return getParamIndexForParamId(param_id) != -1;
   }

   //------------------------------//
   // clap_plugin_param_indication //
   //------------------------------//

   template <MisbehaviourHandler h, CheckingLevel l>
   void Plugin<h, l>::clapParamIndicationSetMapping(const clap_plugin_t *plugin,
                                                    clap_id param_id,
                                                    bool has_mapping,
                                                    const clap_color_t *color,
                                                    const char *label,
                                                    const char *description) noexcept {
      auto &self = from(plugin);
      self.checkMainThread();

      if (l >= CheckingLevel::Minimal) {
         if (!self.isValidParamId(param_id)) {
            std::ostringstream msg;
            msg << "clap_plugin_param_indication.set_mapping() called with invalid param_id: "
                << param_id;
            self.hostMisbehaving(msg.str());
            return;
         }
      }

      self.paramIndicationSetMapping(param_id, has_mapping, color, label, description);
   }

   template <MisbehaviourHandler h, CheckingLevel l>
   void Plugin<h, l>::clapParamIndicationSetAutomation(const clap_plugin_t *plugin,
                                                       clap_id param_id,
                                                       uint32_t automation_state,
                                                       const clap_color_t *color) noexcept {
      auto &self = from(plugin);
      self.checkMainThread();

      if (l >= CheckingLevel::Minimal) {
         if (!self.isValidParamId(param_id)) {
            std::ostringstream msg;
            msg << "clap_plugin_param_indication.set_automation() called with invalid param_id: "
                << param_id;
            self.hostMisbehaving(msg.str());
            return;
         }
      }

      self.paramIndicationSetAutomation(param_id, automation_state, color);
   }

   //----------------------------//
   // clap_plugin_remote_controls //
   //----------------------------//
   template <MisbehaviourHandler h, CheckingLevel l>
   uint32_t Plugin<h, l>::clapRemoteControlsPageCount(const clap_plugin *plugin) noexcept {
      auto &self = from(plugin);
      self.ensureMainThread("clap_plugin_remote_controls.page_count");

      return self.remoteControlsPageCount();
   }

   template <MisbehaviourHandler h, CheckingLevel l>
   bool Plugin<h, l>::clapRemoteControlsPageGet(const clap_plugin *plugin,
                                                uint32_t page_index,
                                                clap_remote_controls_page *page) noexcept {
      auto &self = from(plugin);
      self.ensureMainThread("clap_plugin_remote_controls.page_info");

      if (l >= CheckingLevel::Minimal) {
         uint32_t count = clapRemoteControlsPageCount(plugin);
         if (page_index >= count) {
            std::ostringstream msg;
            msg << "Host called clap_plugin_remote_controls.page_info() with an index out of "
                   "bounds: "
                << page_index << " >= " << count;
            self.hostMisbehaving(msg.str());
            return false;
         }
      }

      return self.remoteControlsPageGet(page_index, page);
   }

   //------------------------//
   // clap_plugin_note_ports //
   //------------------------//
   template <MisbehaviourHandler h, CheckingLevel l>
   uint32_t Plugin<h, l>::clapNotePortsCount(const clap_plugin *plugin, bool is_input) noexcept {
      auto &self = from(plugin);
      self.ensureMainThread("clap_plugin_note_port.count");
      return self.notePortsCount(is_input);
   }

   template <MisbehaviourHandler h, CheckingLevel l>
   bool Plugin<h, l>::clapNotePortsInfo(const clap_plugin *plugin,
                                        uint32_t index,
                                        bool is_input,
                                        clap_note_port_info *info) noexcept {
      auto &self = from(plugin);
      self.ensureMainThread("clap_plugin_note_ports.info");

      if (l >= CheckingLevel::Minimal) {
         auto count = clapNotePortsCount(plugin, is_input);
         if (index >= count) {
            std::ostringstream msg;
            msg << "Host called clap_plugin_note_ports.info() with an index out of bounds: "
                << index << " >= " << count;
            self.hostMisbehaving(msg.str());
            return false;
         }
      }

      return self.notePortsInfo(index, is_input, info);
   }

   //-----------------------//
   // clap_plugin_note_name //
   //-----------------------//
   template <MisbehaviourHandler h, CheckingLevel l>
   uint32_t Plugin<h, l>::clapNoteNameCount(const clap_plugin *plugin) noexcept {
      auto &self = from(plugin);
      self.ensureMainThread("clap_plugin_note_name.count");
      return self.noteNameCount();
   }

   template <MisbehaviourHandler h, CheckingLevel l>
   bool Plugin<h, l>::clapNoteNameGet(const clap_plugin *plugin,
                                      uint32_t index,
                                      clap_note_name *note_name) noexcept {
      auto &self = from(plugin);
      self.ensureMainThread("clap_plugin_note_name.get");

      if (l >= CheckingLevel::Minimal) {
         auto count = clapNoteNameCount(plugin);
         if (index >= count) {
            std::ostringstream msg;
            msg << "host called clap_plugin_note_name.get with an index out of bounds: " << index
                << " >= " << count;
            self.hostMisbehaving(msg.str());
            return false;
         }
      }

      return self.noteNameGet(index, note_name);
   }

   //------------------------//
   // clap_plugin_event_loop //
   //------------------------//
   template <MisbehaviourHandler h, CheckingLevel l>
   void Plugin<h, l>::clapOnTimer(const clap_plugin *plugin, clap_id timer_id) noexcept {
      auto &self = from(plugin);
      self.ensureMainThread("clap_plugin_timer_support.on_timer");

      if (l >= CheckingLevel::Minimal) {
         if (timer_id == CLAP_INVALID_ID) {
            self.hostMisbehaving(
               "Host called clap_plugin_timer_support.on_timer with an invalid timer_id");
            return;
         }
      }

      self.onTimer(timer_id);
   }

   template <MisbehaviourHandler h, CheckingLevel l>
   void Plugin<h, l>::clapOnPosixFd(const clap_plugin *plugin,
                                    int fd,
                                    clap_posix_fd_flags_t flags) noexcept {
      auto &self = from(plugin);
      self.ensureMainThread("clap_plugin_event_loop.on_fd");

      self.onPosixFd(fd, flags);
   }

   //------------------------//
   // clap_plugin_voice_info //
   //------------------------//

   template <MisbehaviourHandler h, CheckingLevel l>
   bool Plugin<h, l>::clapVoiceInfoGet(const clap_plugin *plugin, clap_voice_info *info) noexcept {
      auto &self = from(plugin);
      self.ensureMainThread("clap_plugin_voice_info.get");

      if (l >= CheckingLevel::Minimal) {
         if (!self._isActive) {
            self.hostMisbehaving(
               "clap_plugin_voice_info.get() requires the plugin to be activated");
            return false;
         }
      }

      return self.voiceInfoGet(info);
   }

   //-----------------//
   // clap_plugin_gui //
   //-----------------//
   template <MisbehaviourHandler h, CheckingLevel l>
   bool Plugin<h, l>::clapGuiGetSize(const clap_plugin *plugin,
                                     uint32_t *width,
                                     uint32_t *height) noexcept {
      auto &self = from(plugin);
      self.ensureMainThread("clap_plugin_gui.size");

      if (l >= CheckingLevel::Minimal) {
         if (!self._isGuiCreated) {
            self.hostMisbehaving("clap_plugin_gui.size() was called without a prior call to "
                                 "clap_plugin_gui.create()");
            return false;
         }
      }

      if (!self.guiGetSize(width, height))
         return false;

      if (l >= CheckingLevel::Maximal && self.guiCanResize()) {
         uint32_t testWidth = *width;
         uint32_t testHeight = *height;

         if (!self.guiAdjustSize(&testWidth, &testHeight))
            self._host.pluginMisbehaving(
               "the plugin claims to be resizable but the value returned"
               " by guiGetSize() needs can't be adjusted using guiAdjustSize()");
      }

      return true;
   }

   template <MisbehaviourHandler h, CheckingLevel l>
   bool Plugin<h, l>::clapGuiCanResize(const clap_plugin *plugin) noexcept {
      auto &self = from(plugin);
      self.ensureMainThread("clap_plugin_gui.can_resize");

      if (l >= CheckingLevel::Minimal) {
         if (!self._isGuiCreated) {
            self.hostMisbehaving("clap_plugin_gui.can_resize() was called without a prior call to "
                                 "clap_plugin_gui.create()");
            return false;
         }
      }

      return self.guiCanResize();
   }

   template <MisbehaviourHandler h, CheckingLevel l>
   bool Plugin<h, l>::clapGuiGetResizeHints(const clap_plugin_t *plugin,
                                            clap_gui_resize_hints_t *hints) noexcept {
      auto &self = from(plugin);
      self.ensureMainThread("clap_plugin_gui.get_resize_hints");

      if (l >= CheckingLevel::Minimal) {
         if (!self._isGuiCreated) {
            self.hostMisbehaving(
               "clap_plugin_gui.get_resize_hints() was called without a prior call to "
               "clap_plugin_gui.create()");
            return false;
         }
      }

      return self.guiGetResizeHints(hints);
   }

   template <MisbehaviourHandler h, CheckingLevel l>
   bool Plugin<h, l>::clapGuiAdjustSize(const clap_plugin *plugin,
                                        uint32_t *width,
                                        uint32_t *height) noexcept {
      auto &self = from(plugin);
      self.ensureMainThread("clap_plugin_gui.adjust_size");

      if (l >= CheckingLevel::Minimal) {
         if (!self._isGuiCreated) {
            self.hostMisbehaving("clap_plugin_gui.adjust_size() was called without a prior call to "
                                 "clap_plugin_gui.create()");
            return false;
         }
      }

      const uint32_t inputWidth = *width;
      const uint32_t inputHeight = *height;

      if (!self.guiAdjustSize(width, height))
         return false;

      if (l >= CheckingLevel::Maximal) {
         uint32_t testWidth = *width;
         uint32_t testHeight = *height;

         if (!self.guiAdjustSize(&testWidth, &testHeight)) {
            self._host.pluginMisbehaving(
               "clap_plugin_gui.adjust_size() failed when called with adjusted values");
            return true;
         }

         if (testWidth != *width || testHeight != *height) {
            std::ostringstream os;
            os << "clap_plugin_gui.adjust_size() isn't stable:" << std::endl
               << "  (" << inputWidth << ", " << inputHeight << ") -> (" << *width << ", "
               << *height << ")" << std::endl
               << "  (" << *width << ", " << *height << ") -> (" << testWidth << ", " << testHeight
               << ")" << std::endl
               << "  !! Check you're rounding math!";
            self._host.pluginMisbehaving(os.str());
         }
      }

      return true;
   }

   template <MisbehaviourHandler h, CheckingLevel l>
   bool Plugin<h, l>::clapGuiSetSize(const clap_plugin *plugin,
                                     uint32_t width,
                                     uint32_t height) noexcept {
      auto &self = from(plugin);
      self.ensureMainThread("clap_plugin_gui.set_size");

      if (l >= CheckingLevel::Minimal) {
         if (!self._isGuiCreated) {
            self.hostMisbehaving("clap_plugin_gui.set_size() was called without a prior call to "
                                 "clap_plugin_gui.create()");
            return false;
         }

         if (!self.guiCanResize()) {
            self.hostMisbehaving(
               "clap_plugin_gui.set_size() was called but the gui is not resizable");
            return false;
         }

         uint32_t testWidth = width;
         uint32_t testHeight = height;

         if (self.guiAdjustSize(&testWidth, &testHeight) &&
             (width != testWidth || height != testHeight)) {
            std::ostringstream os;
            os << "clap_plugin_gui.set_size() was called with a size which was not adjusted by "
                  "clap_plugin_gui.adjust_size(): "
               << width << "x" << height << " vs " << testWidth << "x" << testHeight;
            self.hostMisbehaving(os.str());
         }
      }

      return self.guiSetSize(width, height);
   }

   template <MisbehaviourHandler h, CheckingLevel l>
   bool Plugin<h, l>::clapGuiSetScale(const clap_plugin *plugin, double scale) noexcept {
      auto &self = from(plugin);
      self.ensureMainThread("clap_plugin_gui.set_scale");

      if (l >= CheckingLevel::Minimal) {
         if (!self._isGuiCreated) {
            self.hostMisbehaving("clap_plugin_gui.set_scale() was called without a prior call to "
                                 "clap_plugin_gui.create()");
            return false;
         }
      }

      return self.guiSetScale(scale);
   }

   template <MisbehaviourHandler h, CheckingLevel l>
   bool Plugin<h, l>::clapGuiShow(const clap_plugin *plugin) noexcept {
      auto &self = from(plugin);
      self.ensureMainThread("clap_plugin_gui.show");

      if (l >= CheckingLevel::Minimal) {
         if (!self._isGuiCreated) {
            self.hostMisbehaving("clap_plugin_gui.show() was called without a prior call to "
                                 "clap_plugin_gui.create()");
            return false;
         }
      }

      return self.guiShow();
   }

   template <MisbehaviourHandler h, CheckingLevel l>
   bool Plugin<h, l>::clapGuiHide(const clap_plugin *plugin) noexcept {
      auto &self = from(plugin);
      self.ensureMainThread("clap_plugin_gui.hide");

      if (l >= CheckingLevel::Minimal) {
         if (!self._isGuiCreated) {
            self.hostMisbehaving("clap_plugin_gui.hide() was called without a prior call to "
                                 "clap_plugin_gui.create()");
            return false;
         }
      }

      return self.guiHide();
   }

   template <MisbehaviourHandler h, CheckingLevel l>
   bool Plugin<h, l>::clapGuiIsApiSupported(const clap_plugin *plugin,
                                            const char *api,
                                            bool isFloating) noexcept {
      auto &self = from(plugin);
      self.ensureMainThread("clap_plugin_gui.is_api_supported");

      return self.guiIsApiSupported(api, isFloating);
   }

   template <MisbehaviourHandler h, CheckingLevel l>
   bool Plugin<h, l>::clapGuiGetPreferredApi(const clap_plugin_t *plugin,
                                             const char **api,
                                             bool *isFloating) noexcept {
      auto &self = from(plugin);
      self.ensureMainThread("clap_plugin_gui.get_preferred_api");

      return self.guiGetPreferredApi(api, isFloating);
   }

   template <MisbehaviourHandler h, CheckingLevel l>
   bool Plugin<h, l>::clapGuiCreate(const clap_plugin *plugin,
                                    const char *api,
                                    bool isFloating) noexcept {
      auto &self = from(plugin);
      self.ensureMainThread("clap_plugin_gui.create");

      if (l >= CheckingLevel::Minimal) {
         if (self._isGuiCreated) {
            self.hostMisbehaving(
               "clap_plugin_gui.create() was called while the plugin gui was already created");
            return false;
         }

         if (!isFloating && !api) {
            self.hostMisbehaving(
               "clap_plugin_gui.create() was called with a null api and a non floating window");
            return false;
         }
      }

      self._guiApi = api;
      self._isGuiFloating = isFloating;
      self._isGuiEmbedded = false;

      if (!self.guiCreate(api, isFloating))
         return false;

      self._isGuiCreated = true;
      return true;
   }

   template <MisbehaviourHandler h, CheckingLevel l>
   void Plugin<h, l>::clapGuiDestroy(const clap_plugin *plugin) noexcept {
      auto &self = from(plugin);
      self.ensureMainThread("clap_plugin_gui.destroy");

      if (l >= CheckingLevel::Minimal) {
         if (!self._isGuiCreated) {
            self.hostMisbehaving(
               "clap_plugin_gui.destroy() was called while the plugin gui not created");
            return;
         }
      }

      self.guiDestroy();
      self._isGuiCreated = false;
      self._isGuiEmbedded = false;
      self._isGuiFloating = false;
   }

   template <MisbehaviourHandler h, CheckingLevel l>
   bool Plugin<h, l>::clapGuiSetParent(const clap_plugin *plugin,
                                       const clap_window *window) noexcept {
      auto &self = from(plugin);
      self.ensureMainThread("clap_plugin_gui.set_parent");

      if (l >= CheckingLevel::Minimal) {
         if (!self._isGuiCreated) {
            self.hostMisbehaving(
               "clap_plugin_gui.set_parent() was called while the plugin gui was not created");
            return false;
         }

         if (self._isGuiFloating) {
            self.hostMisbehaving("clap_plugin_gui.set_parent() was called while the plugin gui is "
                                 "a floating window, use set_transient() instead");
            return false;
         }
      }

      if (!self.guiSetParent(window))
         return false;

      self._isGuiEmbedded = true;
      return true;
   }

   template <MisbehaviourHandler h, CheckingLevel l>
   bool Plugin<h, l>::clapGuiSetTransient(const clap_plugin *plugin,
                                          const clap_window *window) noexcept {
      auto &self = from(plugin);
      self.ensureMainThread("clap_plugin_gui.set_transient");

      if (l >= CheckingLevel::Minimal) {
         if (!self._isGuiCreated) {
            self.hostMisbehaving(
               "clap_plugin_gui.set_transient() was called while the plugin gui was not created");
            return false;
         }

         if (!self._isGuiFloating) {
            self.hostMisbehaving("clap_plugin_gui.set_transient() was called while the plugin gui "
                                 "isn't a floating window, use set_parent() instead");
            return false;
         }
      }

      return self.guiSetTransient(window);
   }

   template <MisbehaviourHandler h, CheckingLevel l>
   void Plugin<h, l>::clapGuiSuggestTitle(const clap_plugin *plugin, const char *title) noexcept {
      auto &self = from(plugin);
      self.ensureMainThread("clap_plugin_gui.suggest_title");

      if (l >= CheckingLevel::Minimal) {
         if (!self._isGuiCreated) {
            self.hostMisbehaving(
               "clap_plugin_gui.suggest_title() was called without a prior call to "
               "clap_plugin_gui.create()");
            return;
         }

         if (!self._isGuiFloating) {
            self.hostMisbehaving(
               "clap_plugin_gui.suggest_title() but the gui was not created as a floating window");
            return;
         }

         if (!title) {
            self.hostMisbehaving("clap_plugin_gui.suggest_title() was called with a null title");
            return;
         }
      }

      return self.guiSuggestTitle(title);
   }

   //--------------------------//
   // clap_plugin_context_menu //
   //--------------------------//
   template <MisbehaviourHandler h, CheckingLevel l>
   bool Plugin<h, l>::clapContextMenuPopulate(const clap_plugin_t *plugin,
                                              const clap_context_menu_target_t *target,
                                              const clap_context_menu_builder_t *builder) noexcept {
      auto &self = from(plugin);
      self.ensureMainThread("clap_plugin_context_menu.populate");

      return self.contextMenuPopulate(target, builder);
   }

   template <MisbehaviourHandler h, CheckingLevel l>
   bool Plugin<h, l>::clapContextMenuPerform(const clap_plugin_t *plugin,
                                             const clap_context_menu_target_t *target,
                                             clap_id action_id) noexcept {
      auto &self = from(plugin);
      self.ensureMainThread("clap_plugin_context_menu.perform");

      return self.contextMenuPerform(target, action_id);
   }

   //--------------------------------//
   // clap_plugin_resource_directory //
   //--------------------------------//
   template <MisbehaviourHandler h, CheckingLevel l>
   void Plugin<h, l>::clapResourceDirectorySetDirectory(const clap_plugin_t *plugin,
                                                        const char *path,
                                                        bool is_shared) noexcept {
      auto &self = from(plugin);
      self.ensureMainThread("clap_plugin_resource_directory.set_directory");
      self.resourceDirectorySetDirectory(path, is_shared);
   }

   template <MisbehaviourHandler h, CheckingLevel l>
   void Plugin<h, l>::clapResourceDirectoryCollect(const clap_plugin_t *plugin, bool all) noexcept {
      auto &self = from(plugin);
      self.ensureMainThread("clap_plugin_resource_directory.collect");
      self.resourceDirectoryCollect(all);
   }

   template <MisbehaviourHandler h, CheckingLevel l>
   uint32_t Plugin<h, l>::clapResourceDirectoryGetFilesCount(const clap_plugin_t *plugin) noexcept {
      auto &self = from(plugin);
      self.ensureMainThread("clap_plugin_resource_directory.get_files_count");
      return self.resourceDirectoryGetFilesCount();
   }

   template <MisbehaviourHandler h, CheckingLevel l>
   int32_t Plugin<h, l>::clapResourceDirectoryGetFilePath(const clap_plugin_t *plugin,
                                                          uint32_t index,
                                                          char *path,
                                                          uint32_t path_size) noexcept {
      auto &self = from(plugin);
      self.ensureMainThread("clap_plugin_resource_directory.get_file_path");
      return self.resourceDirectoryGetFilePath(index, path, path_size);
   }

   //------------------//
   // clap_plugin_undo //
   //------------------//
   template <MisbehaviourHandler h, CheckingLevel l>
   void
   Plugin<h, l>::clapUndoGetDeltaProperties(const clap_plugin_t *plugin,
                                            clap_undo_delta_properties_t *properties) noexcept {
      auto &self = from(plugin);
      self.ensureMainThread("clap_undo.get_delta_properties");
      return self.undoGetDeltaProperties(properties);
   }

   template <MisbehaviourHandler h, CheckingLevel l>
   bool Plugin<h, l>::clapUndoCanUseDeltaFormatVersion(const clap_plugin_t *plugin,
                                                       clap_id format_version) noexcept {
      auto &self = from(plugin);
      self.ensureMainThread("clap_undo.can_use_delta_format_version");
      return self.undoCanUseDeltaFormatVersion(format_version);
   }

   template <MisbehaviourHandler h, CheckingLevel l>
   bool Plugin<h, l>::clapUndoApplyDelta(const clap_plugin_t *plugin,
                                         clap_id format_version,
                                         const void *delta,
                                         size_t delta_size) noexcept {
      auto &self = from(plugin);
      self.ensureMainThread("clap_undo.apply_delta");
      return self.undoApplyDelta(format_version, delta, delta_size);
   }

   template <MisbehaviourHandler h, CheckingLevel l>
   void Plugin<h, l>::clapUndoSetContextInfo(const clap_plugin_t *plugin,
                                             uint64_t flags,
                                             const char *undo_name,
                                             const char *redo_name) noexcept {
      auto &self = from(plugin);
      self.ensureMainThread("clap_undo.set_context_info");
      return self.undoSetContextInfo(flags, undo_name, redo_name);
   }

   /////////////
   // Logging //
   /////////////
   template <MisbehaviourHandler h, CheckingLevel l>
   void Plugin<h, l>::log(clap_log_severity severity, const char *msg) const noexcept {
      logTee(severity, msg);
      _host.log(severity, msg);
   }

   template <MisbehaviourHandler h, CheckingLevel l>
   void Plugin<h, l>::hostMisbehaving(const char *msg) const noexcept {
      log(CLAP_LOG_HOST_MISBEHAVING, msg);

      if (h == MisbehaviourHandler::Terminate)
         std::terminate();
   }

   /////////////////////
   // Thread Checking //
   /////////////////////
   template <MisbehaviourHandler h, CheckingLevel l>
   void Plugin<h, l>::checkMainThread() const noexcept {
      if (l == CheckingLevel::None)
         return;

      if (!_host.canUseThreadCheck() || _host.isMainThread())
         return;

      std::cerr << "thread-error: this code must be running on the main thread" << std::endl;

      if (h == MisbehaviourHandler::Terminate)
         std::terminate();
   }

   template <MisbehaviourHandler h, CheckingLevel l>
   void Plugin<h, l>::checkAudioThread() const noexcept {
      if (l == CheckingLevel::None)
         return;

      if (!_host.canUseThreadCheck() || _host.isAudioThread())
         return;

      std::cerr << "thread-error: this code must be running on the audio thread" << std::endl;

      if (h == MisbehaviourHandler::Terminate)
         std::terminate();
   }

   template <MisbehaviourHandler h, CheckingLevel l>
   void Plugin<h, l>::checkParamThread() const noexcept {
      if (l == CheckingLevel::None)
         return;

      if (isActive())
         checkAudioThread();
      else
         checkMainThread();
   }

   template <MisbehaviourHandler h, CheckingLevel l>
   void Plugin<h, l>::ensureParamThread(const char *method) const noexcept {
      if (l == CheckingLevel::None)
         return;

      if (isActive())
         ensureAudioThread(method);
      else
         ensureMainThread(method);
   }

   template <MisbehaviourHandler h, CheckingLevel l>
   void Plugin<h, l>::ensureMainThread(const char *method) const noexcept {
      if (l == CheckingLevel::None)
         return;

      if (!_host.canUseThreadCheck() || _host.isMainThread())
         return;

      std::ostringstream msg;
      msg << "Host called the method " << method
          << "() on wrong thread! It must be called on main thread!";
      hostMisbehaving(msg.str());
   }

   template <MisbehaviourHandler h, CheckingLevel l>
   void Plugin<h, l>::ensureAudioThread(const char *method) const noexcept {
      if (l == CheckingLevel::None)
         return;

      if (!_host.canUseThreadCheck() || _host.isAudioThread())
         return;

      std::ostringstream msg;
      msg << "Host called the method " << method
          << "() on wrong thread! It must be called on audio thread!";
      hostMisbehaving(msg.str());
   }

   ///////////////
   // Utilities //
   ///////////////
   template <MisbehaviourHandler h, CheckingLevel l>
   Plugin<h, l> &Plugin<h, l>::from(const clap_plugin *plugin, bool requireInitialized) noexcept {
      if (l >= CheckingLevel::Minimal) {
         if (!plugin) {
            std::cerr << "called with a null clap_plugin pointer!" << std::endl;
            std::terminate();
         }

         if (!plugin->plugin_data) {
            std::cerr << "called with a null clap_plugin->plugin_data pointer! The host must never "
                         "change this pointer!"
                      << std::endl;
            std::terminate();
         }

         auto &self = *static_cast<Plugin *>(plugin->plugin_data);
         if (requireInitialized && !self._wasInitialized) {
            self.hostMisbehaving("Host is required to call clap_plugin.init() first");
            if (h == MisbehaviourHandler::Terminate)
               std::terminate();
         }
      }

      return *static_cast<Plugin *>(plugin->plugin_data);
   }

   template <MisbehaviourHandler h, CheckingLevel l>
   void Plugin<h, l>::runOnMainThread(std::function<void()> callback) {
      if (_host.canUseThreadCheck() && _host.isMainThread()) {
         callback();
         return;
      }

      std::lock_guard<std::mutex> guard(_mainThreadCallbacksLock);
      _mainThreadCallbacks.emplace(std::move(callback));
      _host.requestCallback();
   }

   template <MisbehaviourHandler h, CheckingLevel l>
   uint32_t Plugin<h, l>::compareAudioPortsInfo(const clap_audio_port_info &a,
                                                const clap_audio_port_info &b) noexcept {
      uint32_t flags = 0;

      if (strncmp(a.name, b.name, sizeof(a.name)))
         flags |= CLAP_AUDIO_PORTS_RESCAN_NAMES;

      if (a.flags != b.flags)
         flags |= CLAP_AUDIO_PORTS_RESCAN_FLAGS;

      if (a.channel_count != b.channel_count)
         flags |= CLAP_AUDIO_PORTS_RESCAN_CHANNEL_COUNT;

      if (strcmp(a.port_type, b.port_type))
         flags |= CLAP_AUDIO_PORTS_RESCAN_PORT_TYPE;

      if (a.in_place_pair != b.in_place_pair)
         flags |= CLAP_AUDIO_PORTS_RESCAN_IN_PLACE_PAIR;

      return flags;
   }
}} // namespace clap::helpers
