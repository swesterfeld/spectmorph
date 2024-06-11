#pragma once

#include <cassert>
#include <iostream>

#include "preset-discovery-provider.hh"

namespace clap { namespace helpers {

   template <MisbehaviourHandler h, CheckingLevel l>
   PresetDiscoveryProvider<h, l>::PresetDiscoveryProvider(
      const clap_preset_discovery_provider_descriptor *desc,
      const clap_preset_discovery_indexer *indexer)
      : _provider({
           desc,
           this,
           providerInit,
           providerDestroy,
           providerGetMetadata,
           providerGetExtension,
        }),
        _indexer(indexer) {}

   template <MisbehaviourHandler h, CheckingLevel l>
   bool PresetDiscoveryProvider<h, l>::providerInit(
      const clap_preset_discovery_provider *provider) noexcept {
      auto &self = from(provider);

      if (l >= CheckingLevel::Minimal) {
         if (self._wasInitialized) {
            std::cerr << "clap_preset_discovery_provider.init() was called twice" << std::endl;
            if (h == MisbehaviourHandler::Terminate)
               std::terminate();
            return true;
         }
      }

      assert(!self._wasInitialized);
      auto res = self.init();
      self._wasInitialized = true;
      return res;
   }

   template <MisbehaviourHandler h, CheckingLevel l>
   void PresetDiscoveryProvider<h, l>::providerDestroy(
      const clap_preset_discovery_provider *provider) noexcept {
      auto &self = from(provider);

      if (l >= CheckingLevel::Minimal) {
         if (self._isBeingDestroyed) {
            std::cerr << "clap_preset_discovery_provider.destroy() was called twice" << std::endl;
            if (h == MisbehaviourHandler::Terminate)
               std::terminate();
            return;
         }
      }

      self._isBeingDestroyed = true;
      delete &self;
   }

   template <MisbehaviourHandler h, CheckingLevel l>
   bool PresetDiscoveryProvider<h, l>::providerGetMetadata(
      const clap_preset_discovery_provider *provider,
      uint32_t location_kind,
      const char *location,
      const clap_preset_discovery_metadata_receiver_t *metadata_receiver) noexcept {
      auto &self = from(provider);
      self.ensureInitialized("get_metadata");
      return self.getMetadata(location_kind, location, metadata_receiver);
   }

   template <MisbehaviourHandler h, CheckingLevel l>
   const void *PresetDiscoveryProvider<h, l>::providerGetExtension(
      const clap_preset_discovery_provider *provider, const char *id) noexcept {
      auto &self = from(provider);
      self.ensureInitialized("get_extension");
      return self.extension(id);
   }

   template <MisbehaviourHandler h, CheckingLevel l>
   PresetDiscoveryProvider<h, l> &
   PresetDiscoveryProvider<h, l>::from(const clap_preset_discovery_provider *provider) noexcept {
      return *static_cast<PresetDiscoveryProvider<h, l> *>(provider->provider_data);
   }

   template <MisbehaviourHandler h, CheckingLevel l>
   void PresetDiscoveryProvider<h, l>::ensureInitialized(const char *method) const noexcept {
      if (l >= CheckingLevel::Minimal) {
         if (!_wasInitialized) {
            std::cerr << "clap_preset_discovery_provider." << method
                      << "() was called before init()." << std::endl;
            if (h == MisbehaviourHandler::Terminate)
               std::terminate();
         }
      }
   }
}} // namespace clap::helpers
