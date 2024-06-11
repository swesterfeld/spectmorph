#pragma once

#include <clap/clap.h>

#include "checking-level.hh"
#include "misbehaviour-handler.hh"

namespace clap { namespace helpers {

   /// @brief C++ glue and checks
   template <MisbehaviourHandler h, CheckingLevel l>
   class PresetDiscoveryProvider {
   public:
      // not copyable, not moveable
      PresetDiscoveryProvider(const PresetDiscoveryProvider &) = delete;
      PresetDiscoveryProvider(PresetDiscoveryProvider &&) = delete;
      PresetDiscoveryProvider &operator=(const PresetDiscoveryProvider &) = delete;
      PresetDiscoveryProvider &operator=(PresetDiscoveryProvider &&) = delete;

      const clap_preset_discovery_provider *provider() const noexcept { return &_provider; }
      const clap_preset_discovery_indexer *indexer() const noexcept { return _indexer; }

   protected:
      PresetDiscoveryProvider(const clap_preset_discovery_provider_descriptor *desc,
                              const clap_preset_discovery_indexer *indexer);
      virtual ~PresetDiscoveryProvider() = default;

      /////////////////////////
      // Methods to override //
      /////////////////////////

      //--------------------------------//
      // clap_preset_discovery_provider //
      //--------------------------------//
      virtual bool init() noexcept { return true; }
      virtual bool
      getMetadata(uint32_t location_kind,
                  const char *location,
                  const clap_preset_discovery_metadata_receiver_t *metadata_receiver) noexcept {
         return false;
      }
      virtual const void *extension(const char *id) noexcept { return nullptr; }

      ///////////////
      // Utilities //
      ///////////////
      static PresetDiscoveryProvider &from(const clap_preset_discovery_provider *provider) noexcept;

      bool wasInitialized() const noexcept { return _wasInitialized; }
      bool isBeingDestroyed() const noexcept { return _isBeingDestroyed; }

   private:
      void ensureInitialized(const char *method) const noexcept;

      /////////////////////
      // CLAP Interfaces //
      /////////////////////

      const clap_preset_discovery_provider _provider;
      const clap_preset_discovery_indexer *const _indexer;

      // clap_preset_discovery_provider
      static bool providerInit(const clap_preset_discovery_provider *provider) noexcept;
      static void providerDestroy(const clap_preset_discovery_provider *provider) noexcept;
      static bool providerGetMetadata(
         const clap_preset_discovery_provider *provider,
         uint32_t location_kind,
         const char *location,
         const clap_preset_discovery_metadata_receiver_t *metadata_receiver) noexcept;
      static const void *providerGetExtension(const clap_preset_discovery_provider *provider,
                                              const char *id) noexcept;

      // state
      bool _wasInitialized = false;
      bool _isBeingDestroyed = false;
   };
}} // namespace clap::helpers
