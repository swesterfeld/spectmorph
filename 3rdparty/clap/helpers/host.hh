#pragma once

#include <string>

#include <clap/all.h>

#include "version-check.hh"

#include "checking-level.hh"
#include "misbehaviour-handler.hh"

namespace clap { namespace helpers {
   template <MisbehaviourHandler h, CheckingLevel l>
   class Host {
   public:
      // not copyable, not moveable
      Host(const Host &) = delete;
      Host(Host &&) = delete;
      Host &operator=(const Host &) = delete;
      Host &operator=(Host &&) = delete;

      const clap_host *clapHost() const { return &_host; }

      ///////////////////////////
      // Misbehaviour handling //
      ///////////////////////////
      void pluginMisbehaving(const char *msg) const noexcept;
      void pluginMisbehaving(const std::string &msg) const noexcept { pluginMisbehaving(msg.c_str()); }
      void hostMisbehaving(const char *msg) const noexcept;
      void hostMisbehaving(const std::string &msg) const noexcept { hostMisbehaving(msg.c_str()); }

      /////////////////////////
      // Methods to override //
      /////////////////////////

      // clap_host_thread_check
      virtual bool threadCheckIsMainThread() const noexcept = 0;
      virtual bool threadCheckIsAudioThread() const noexcept = 0;

   protected:
      Host(const char *name, const char *vendor, const char *url, const char *version);
      virtual ~Host() = default;

      /////////////////////////
      // Methods to override //
      /////////////////////////

      // clap_host
      virtual void requestRestart() noexcept = 0;
      virtual void requestProcess() noexcept = 0;
      virtual void requestCallback() noexcept = 0;

      virtual bool enableDraftExtensions() const noexcept { return false; }

      // clap_host_audio_ports
      virtual bool implementsAudioPorts() const noexcept { return false; }
      virtual bool audioPortsIsRescanFlagSupported(uint32_t flag) noexcept { return false; }
      virtual void audioPortsRescan(uint32_t flags) noexcept {}

      // clap_host_gui
      virtual bool implementsGui() const noexcept { return false; }
      virtual void guiResizeHintsChanged() noexcept {}
      virtual bool guiRequestResize(uint32_t width, uint32_t height) noexcept { return false; }
      virtual bool guiRequestShow() noexcept { return false; }
      virtual bool guiRequestHide() noexcept { return false; }
      virtual void guiClosed(bool wasDestroyed) noexcept {}

      // clap_host_latency
      virtual bool implementsLatency() const noexcept { return false; }
      virtual void latencyChanged() noexcept {}

      // clap_host_log
      virtual bool implementsLog() const noexcept { return false; }
      virtual void logLog(clap_log_severity severity, const char *message) const noexcept {}

      // clap_host_params
      virtual bool implementsParams() const noexcept { return false; }
      virtual void paramsRescan(clap_param_rescan_flags flags) noexcept {}
      virtual void paramsClear(clap_id paramId, clap_param_clear_flags flags) noexcept {}
      virtual void paramsRequestFlush() noexcept {}

      // clap_host_posix_fd_support
      virtual bool implementsPosixFdSupport() const noexcept { return false; }
      virtual bool posixFdSupportRegisterFd(int fd, clap_posix_fd_flags_t flags) noexcept { return false; }
      virtual bool posixFdSupportModifyFd(int fd, clap_posix_fd_flags_t flags) noexcept { return false; }
      virtual bool posixFdSupportUnregisterFd(int fd) noexcept { return false; }

      // clap_host_remote_controls
      virtual bool implementsRemoteControls() const noexcept { return false; }
      virtual void remoteControlsChanged() noexcept {}
      virtual void remoteControlsSuggestPage(clap_id pageId) noexcept {}

      // clap_host_state
      virtual bool implementsState() const noexcept { return false; }
      virtual void stateMarkDirty() noexcept {}

      // clap_host_timer_support
      virtual bool implementsTimerSupport() const noexcept { return false; }
      virtual bool timerSupportRegisterTimer(uint32_t periodMs, clap_id *timerId) noexcept { return false; }
      virtual bool timerSupportUnregisterTimer(clap_id timerId) noexcept { return false; }

      // clap_host_tail
      virtual bool implementsTail() const noexcept { return false; }
      virtual void tailChanged() noexcept {}

      // clap_host_thread_pool
      virtual bool implementsThreadPool() const noexcept { return false; }
      virtual bool threadPoolRequestExec(uint32_t numTasks) noexcept { return false; }

      /////////////////////
      // Thread Checking //
      /////////////////////
      void ensureMainThread(const char *method) const noexcept;
      void ensureAudioThread(const char *method, bool expectedState = true) const noexcept;

      ///////////////
      // Utilities //
      ///////////////
      static Host &from(const clap_host *host) noexcept;

   private:
      const clap_host _host;

      /////////////////////
      // CLAP Interfaces //
      /////////////////////

      // clap_host
      static const void *clapGetExtension(const clap_host_t *host,
                                          const char *extension_id) noexcept;
      static void clapRequestRestart(const clap_host_t *host) noexcept;
      static void clapRequestProcess(const clap_host_t *host) noexcept;
      static void clapRequestCallback(const clap_host_t *host) noexcept;

      // clap_host_audio_ports
      static bool clapAudioPortsIsRescanFlagSupported(const clap_host_t *host, uint32_t flag) noexcept;
      static void clapAudioPortsRescan(const clap_host_t *host, uint32_t flags) noexcept;

      // clap_host_gui
      static void clapGuiResizeHintsChanged(const clap_host_t *host) noexcept;
      static bool clapGuiRequestResize(const clap_host_t *host,
                                       uint32_t width,
                                       uint32_t height) noexcept;
      static bool clapGuiRequestShow(const clap_host_t *host) noexcept;
      static bool clapGuiRequestHide(const clap_host_t *host) noexcept;
      static void clapGuiClosed(const clap_host_t *host, bool was_destroyed) noexcept;

      // clap_host_latency
      static void clapLatencyChanged(const clap_host_t *host) noexcept;

      // clap_host_log
      static void clapLogLog(const clap_host_t *host,
                             clap_log_severity severity,
                             const char *message) noexcept;

      // clap_host_params
      static void clapParamsRescan(const clap_host_t *host, clap_param_rescan_flags flags) noexcept;
      static void clapParamsClear(const clap_host_t *host,
                                  clap_id param_id,
                                  clap_param_clear_flags flags) noexcept;
      static void clapParamsRequestFlush(const clap_host_t *host) noexcept;

      // clap_host_posix_fd_support
      static bool clapPosixFdSupportRegisterFd(const clap_host *host, int fd, clap_posix_fd_flags_t flags) noexcept;
      static bool clapPosixFdSupportModifyFd(const clap_host *host, int fd, clap_posix_fd_flags_t flags) noexcept;
      static bool clapPosixFdSupportUnregisterFd(const clap_host *host, int fd) noexcept;

      // clap_host_remote_controls
      static void clapRemoteControlsChanged(const clap_host *host) noexcept;
      static void clapRemoteControlsSuggestPage(const clap_host *host, clap_id page_id) noexcept;

      // clap_host_state
      static void clapStateMarkDirty(const clap_host *host) noexcept;

      // clap_host_timer_support
      static bool clapTimerSupportRegisterTimer(const clap_host *host, uint32_t period_ms, clap_id *timer_id) noexcept;
      static bool clapTimerSupportUnregisterTimer(const clap_host *host, clap_id timer_id) noexcept;

      // clap_host_tail
      static void clapTailChanged(const clap_host_t *host) noexcept;

      // clap_host_thread_check
      static bool clapThreadCheckIsMainThread(const clap_host_t *host) noexcept;
      static bool clapThreadCheckIsAudioThread(const clap_host_t *host) noexcept;

      // clap_host_thread_pool
      static bool clapThreadPoolRequestExec(const clap_host *host, uint32_t num_tasks) noexcept;

      // interfaces
      static const clap_host_audio_ports _hostAudioPorts;
      static const clap_host_gui _hostGui;
      static const clap_host_latency _hostLatency;
      static const clap_host_log _hostLog;
      static const clap_host_params _hostParams;
      static const clap_host_posix_fd_support _hostPosixFdSupport;
      static const clap_host_remote_controls _hostRemoteControls;
      static const clap_host_state _hostState;
      static const clap_host_timer_support _hostTimerSupport;
      static const clap_host_tail _hostTail;
      static const clap_host_thread_check _hostThreadCheck;
      static const clap_host_thread_pool _hostThreadPool;
   };
}} // namespace clap::helpers
