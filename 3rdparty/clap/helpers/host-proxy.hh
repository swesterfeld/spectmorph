#pragma once

#include <string>

#include <clap/all.h>

#include "checking-level.hh"
#include "misbehaviour-handler.hh"

namespace clap { namespace helpers {
   template <MisbehaviourHandler h, CheckingLevel l>
   class HostProxy {
   public:
      HostProxy(const clap_host *host);

      // not copyable, not moveable
      HostProxy(const HostProxy &) = delete;
      HostProxy(HostProxy &&) = delete;
      HostProxy &operator=(const HostProxy &) = delete;
      HostProxy &operator=(HostProxy &&) = delete;

      void init();

      const clap_host *host() const noexcept { return _host; }

      ///////////////
      // clap_host //
      ///////////////
      template <typename T>
      void getExtension(const T *&ptr, const char *id) noexcept;
      void requestCallback() noexcept;
      void requestRestart() noexcept;
      void requestProcess() noexcept;

      ///////////////////
      // clap_host_log //
      ///////////////////
      bool canUseHostLog() const noexcept;
      void log(clap_log_severity severity, const char *msg) const noexcept;
      void hostMisbehaving(const char *msg) const noexcept;
      void hostMisbehaving(const std::string &msg) const noexcept { hostMisbehaving(msg.c_str()); }
      void pluginMisbehaving(const char *msg) const noexcept;
      void pluginMisbehaving(const std::string &msg) const noexcept {
         pluginMisbehaving(msg.c_str());
      }

      ////////////////////////////
      // clap_host_thread_check //
      ////////////////////////////
      bool canUseThreadCheck() const noexcept;
      bool isMainThread() const noexcept;
      bool isAudioThread() const noexcept;

      //////////////////////////////////
      // clap_host_audio_ports_config //
      //////////////////////////////////
      bool canUseAudioPortsConfig() const noexcept;
      void audioPortsConfigRescan() const noexcept;

      ///////////////////////////
      // clap_host_audio_ports //
      ///////////////////////////
      bool canUseAudioPorts() const noexcept;
      void audioPortsRescan(uint32_t flags) const noexcept;

      //////////////////////////
      // clap_host_note_ports //
      //////////////////////////
      bool canUseNotePorts() const noexcept;
      void notePortsRescan(uint32_t flags) const noexcept;

      /////////////////////
      // clap_host_state //
      /////////////////////
      bool canUseState() const noexcept;
      void stateMarkDirty() const noexcept;

      ///////////////////////
      // clap_host_latency //
      ///////////////////////
      bool canUseLatency() const noexcept;
      void latencyChanged() const noexcept;

      ////////////////////
      // clap_host_tail //
      ////////////////////
      bool canUseTail() const noexcept;
      void tailChanged() const noexcept;

      /////////////////////////
      // clap_host_note_name //
      /////////////////////////
      bool canUseNoteName() const noexcept;
      void noteNameChanged() const noexcept;

      //////////////////////
      // clap_host_params //
      //////////////////////
      bool canUseParams() const noexcept;
      void paramsRescan(clap_param_rescan_flags flags) const noexcept;
      void paramsClear(clap_id param_id, clap_param_clear_flags flags) const noexcept;
      void paramsRequestFlush() const noexcept;

      //////////////////////////
      // clap_host_track_info //
      //////////////////////////
      bool canUseTrackInfo() const noexcept;
      bool trackInfoGet(clap_track_info *info) const noexcept;

      ///////////////////
      // clap_host_gui //
      ///////////////////
      bool canUseGui() const noexcept;
      void guiResizeHintsChanged() const noexcept;
      bool guiRequestResize(uint32_t width, uint32_t height) const noexcept;
      bool guiRequestShow() const noexcept;
      bool guiRequestHide() const noexcept;
      void guiClosed(bool wasDestroyed) const noexcept;

      /////////////////////////////
      // clap_host_timer_support //
      /////////////////////////////
      bool canUseTimerSupport() const noexcept;
      bool timerSupportRegister(uint32_t period_ms, clap_id *timer_id) const noexcept;
      bool timerSupportUnregister(clap_id timer_id) const noexcept;

      //////////////////////////
      // clap_host_fd_support //
      //////////////////////////
      bool canUsePosixFdSupport() const noexcept;
      bool posixFdSupportRegister(int fd, clap_posix_fd_flags_t flags) const noexcept;
      bool posixFdSupportModify(int fd, clap_posix_fd_flags_t flags) const noexcept;
      bool posixFdSupportUnregister(int fd) const noexcept;

      //////////////////////////////
      // clap_host_remote_controls //
      //////////////////////////////
      bool canUseRemoteControls() const noexcept;
      void remoteControlsChanged() const noexcept;
      void remoteControlsSuggestPage(clap_id page_id) const noexcept;

      ///////////////////////////
      // clap_host_thread_pool //
      ///////////////////////////
      bool canUseThreadPool() const noexcept;
      bool threadPoolRequestExec(uint32_t numTasks) const noexcept;

      //////////////////////////
      // clap_host_voice_info //
      //////////////////////////
      bool canUseVoiceInfo() const noexcept;
      void voiceInfoChanged() const noexcept;

      ////////////////////////////
      // clap_host_context_menu //
      ////////////////////////////
      bool canUseContextMenu() const noexcept;
      bool contextMenuPopulate(const clap_context_menu_target_t *target,
                               const clap_context_menu_builder_t *builder) const noexcept;
      bool contextMenuPerform(const clap_context_menu_target_t *target,
                              clap_id action_id) const noexcept;
      bool contextMenuCanPopup() const noexcept;
      bool contextMenuPopup(const clap_context_menu_target_t *target,
                            int32_t screen_index,
                            int32_t x,
                            int32_t y) const noexcept;

      ///////////////////////////
      // clap_host_preset_load //
      ///////////////////////////
      bool canUsePresetLoad() const noexcept;
      void presetLoadOnError(uint32_t location_kind,
                             const char *location,
                             const char *load_key,
                             int32_t os_error,
                             const char *msg) const noexcept;
      void presetLoadLoaded(uint32_t location_kind,
                            const char *location,
                            const char *load_key) const noexcept;

      //////////////////////////////////
      // clap_host_resource_directory //
      //////////////////////////////////
      bool canUseResourceDirectory() const noexcept;
      bool requestDirectory(bool isShared) const noexcept;
      void releaseDirectory(bool isShared) const noexcept;

      ////////////////////
      // clap_host_undo //
      ////////////////////
      bool canUseUndo() const noexcept;
      void undoBeginChange() const noexcept;
      void undoCancelChange() const noexcept;
      void undoChangeMade(const char *name,
                          const void *redo_delta,
                          size_t redo_delta_size,
                          const void *undo_delta,
                          size_t undo_delta_size) const noexcept;
      void undoUndo(const clap_host_t *host) const noexcept;
      void undoRedo(const clap_host_t *host) const noexcept;
      void undoSetContextInfoSubscription(const clap_host_t *host, bool wants_info) const noexcept;

   protected:
      void ensureMainThread(const char *method) const noexcept;
      void ensureAudioThread(const char *method) const noexcept;

      const clap_host *const _host;

      const clap_host_log *_hostLog = nullptr;
      const clap_host_thread_check *_hostThreadCheck = nullptr;
      const clap_host_thread_pool *_hostThreadPool = nullptr;
      const clap_host_audio_ports *_hostAudioPorts = nullptr;
      const clap_host_audio_ports_config *_hostAudioPortsConfig = nullptr;
      const clap_host_note_ports *_hostNotePorts = nullptr;
      const clap_host_resource_directory *_hostResourceDirectory = nullptr;
      const clap_host_latency *_hostLatency = nullptr;
      const clap_host_gui *_hostGui = nullptr;
      const clap_host_timer_support *_hostTimerSupport = nullptr;
      const clap_host_posix_fd_support *_hostPosixFdSupport = nullptr;
      const clap_host_params *_hostParams = nullptr;
      const clap_host_track_info *_hostTrackInfo = nullptr;
      const clap_host_state *_hostState = nullptr;
      const clap_host_note_name *_hostNoteName = nullptr;
      const clap_host_remote_controls *_hostRemoteControls = nullptr;
      const clap_host_voice_info *_hostVoiceInfo = nullptr;
      const clap_host_tail *_hostTail = nullptr;
      const clap_host_context_menu *_hostContextMenu = nullptr;
      const clap_host_preset_load *_hostPresetLoad = nullptr;
      const clap_host_undo *_hostUndo = nullptr;
   };
}} // namespace clap::helpers
