#pragma once

#include <clap/clap.h>

#include "checking-level.hh"
#include "misbehaviour-handler.hh"
#include "host.hh"

namespace clap { namespace helpers {
   template <MisbehaviourHandler h, CheckingLevel l>
   class PluginProxy {
   public:
      PluginProxy(const clap_plugin& plugin, const Host<h, l>& host) : _host{host}, _plugin{plugin} {}

      /////////////////
      // clap_plugin //
      /////////////////
      const clap_plugin* clapPlugin() const { return &_plugin; }
      bool init() noexcept;
      template <typename T>
      void getExtension(const T *&ptr, const char *id) const noexcept;
      void destroy() noexcept;
      bool activate(double sampleRate, uint32_t minFramesCount, uint32_t maxFramesCount) noexcept;
      void deactivate() noexcept;
      bool startProcessing() noexcept;
      void stopProcessing() noexcept;
      void reset() const noexcept;
      clap_process_status process(const clap_process_t *process) const noexcept;
      void onMainThread() const noexcept;

      /////////////////////////////
      // clap_plugin_audio_ports //
      /////////////////////////////
      bool canUseAudioPorts() const noexcept;
      uint32_t audioPortsCount(bool isInput) const noexcept;
      bool audioPortsGet(uint32_t index, bool isInput, clap_audio_port_info_t *info) const noexcept;

      /////////////////////
      // clap_plugin_gui //
      /////////////////////
      bool canUseGui() const noexcept;
      bool guiIsApiSupported(const char *api, bool isFloating) const noexcept;
      bool guiGetPreferredApi(const char **api, bool *isFloating) const noexcept;
      bool guiCreate(const char *api, bool isFloating) const noexcept;
      void guiDestroy() const noexcept;
      bool guiSetScale(double scale) const noexcept;
      bool guiGetSize(uint32_t *width, uint32_t *height) const noexcept;
      bool guiCanResize() const noexcept;
      bool guiGetResizeHints(clap_gui_resize_hints_t *hints) const noexcept;
      bool guiAdjustSize(uint32_t *width, uint32_t *height) const noexcept;
      bool guiSetSize(uint32_t width, uint32_t height) const noexcept;
      bool guiSetParent(const clap_window_t *window) const noexcept;
      bool guiSetTransient(const clap_window_t *window) const noexcept;
      void guiSuggestTitle(const char *title) const noexcept;
      bool guiShow() const noexcept;
      bool guiHide() const noexcept;

      /////////////////////////
      // clap_plugin_latency //
      /////////////////////////
      bool canUseLatency() const noexcept;
      uint32_t latencyGet() const noexcept;

      ////////////////////////////
      // clap_plugin_note_ports //
      ////////////////////////////
      bool canUseNotePorts() const noexcept;
      uint32_t notePortsCount(bool is_input) const noexcept;
      bool notePortsGet(uint32_t               index,
                        bool                   is_input,
                        clap_note_port_info_t *info) const noexcept;

      ////////////////////////
      // clap_plugin_params //
      ////////////////////////
      bool canUseParams() const noexcept;
      uint32_t paramsCount() const noexcept;
      bool paramsGetInfo(uint32_t paramIndex, clap_param_info_t *paramInfo) const noexcept;
      bool paramsGetValue(clap_id paramId, double *outValue) const noexcept;
      bool paramsValueToText(clap_id paramId,
                             double value,
                             char *outBuffer,
                             uint32_t outBufferCapacity) const noexcept;
      bool paramsTextToValue(clap_id paramId,
                             const char *paramValueText,
                             double *outValue) const noexcept;
      void paramsFlush(const clap_input_events_t *in, const clap_output_events_t *out) const noexcept;

      //////////////////////////////////
      // clap_plugin_posix_fd_support //
      //////////////////////////////////
      bool canUsePosixFdSupport() const noexcept;
      void posixFdSupportOnFd(int fd, clap_posix_fd_flags_t flags) const noexcept;

      /////////////////////////////
      // clap_plugin_preset_load //
      /////////////////////////////
      bool canUsePresetLoad() const noexcept;
      bool presetLoadFromLocation(uint32_t locationKind,
                                  const char *location,
                                  const char *loadKey)  const noexcept;

      /////////////////////////////////
      // clap_plugin_remote_controls //
      /////////////////////////////////
      bool canUseRemoteControls() const noexcept;
      uint32_t remoteControlsCount() const noexcept;
      bool remoteControlsGet(uint32_t                     pageIndex,
                             clap_remote_controls_page_t *page) const noexcept;

      ////////////////////////
      // clap_plugin_render //
      ////////////////////////
      bool canUseRender() const noexcept;
      bool renderHasHardRealtimeRequirement() const noexcept;
      bool renderSet(clap_plugin_render_mode mode) const noexcept;

      ///////////////////////
      // clap_plugin_state //
      ///////////////////////
      bool canUseState() const noexcept;
      bool stateSave(const clap_ostream_t *stream) const noexcept;
      bool stateLoad(const clap_istream_t *stream) const noexcept;

      //////////////////////
      // clap_plugin_tail //
      //////////////////////
      bool canUseTail() const noexcept;
      uint32_t tailGet() const noexcept;

      /////////////////////////////
      // clap_plugin_thread_pool //
      /////////////////////////////
      bool canUseThreadPool() const noexcept;
      void threadPoolExec(uint32_t taskIndex) const noexcept;

      ///////////////////////////////
      // clap_plugin_timer_support //
      ///////////////////////////////
      bool canUseTimerSupport() const noexcept;
      void timerSupportOnTimer(clap_id timerId) const noexcept;

   protected:
      /////////////////////
      // Thread Checking //
      /////////////////////
      void ensureMainThread(const char *method) const noexcept;
      void ensureAudioThread(const char *method) const noexcept;
      void ensureActivated(const char *method, bool expectedState) const noexcept;
      void ensureProcessing(const char *method, bool expectedState) const noexcept;

      const Host<h, l>& _host;

      const clap_plugin& _plugin;

      const clap_plugin_audio_ports *_pluginAudioPorts = nullptr;
      const clap_plugin_gui *_pluginGui = nullptr;
      const clap_plugin_latency *_pluginLatency = nullptr;
      const clap_plugin_note_ports *_pluginNotePorts = nullptr;
      const clap_plugin_params *_pluginParams = nullptr;
      const clap_plugin_posix_fd_support *_pluginPosixFdSupport = nullptr;
      const clap_plugin_preset_load *_pluginPresetLoad = nullptr;
      const clap_plugin_remote_controls *_pluginRemoteControls = nullptr;
      const clap_plugin_render *_pluginRender = nullptr;
      const clap_plugin_state *_pluginState = nullptr;
      const clap_plugin_tail *_pluginTail = nullptr;
      const clap_plugin_thread_pool *_pluginThreadPool = nullptr;
      const clap_plugin_timer_support *_pluginTimerSupport = nullptr;

      // state
      bool _isActive = false;
      bool _isProcessing = false;
   };
}} // namespace clap::helpers
