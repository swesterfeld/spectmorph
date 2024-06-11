#pragma once

#include <iostream>

#include "preset-discovery-indexer.hh"

namespace clap { namespace helpers {

   template <MisbehaviourHandler h, CheckingLevel l>
   PresetDiscoveryIndexer<h, l>::PresetDiscoveryIndexer(const char *name,
                                                        const char *vendor,
                                                        const char *url,
                                                        const char *version)
      : _indexer({
           CLAP_VERSION,
           name,
           vendor,
           url,
           version,
           this,
           indexerDeclareFiletype,
           indexerDeclareLocation,
           indexerDeclareSoundPack,
           indexerGetExtension,
        }) {}

   template <MisbehaviourHandler h, CheckingLevel l>
   PresetDiscoveryIndexer<h, l> &
   PresetDiscoveryIndexer<h, l>::from(const clap_preset_discovery_indexer *indexer) noexcept {
      return *static_cast<PresetDiscoveryIndexer<h, l> *>(indexer->indexer_data);
   }

   template <MisbehaviourHandler h, CheckingLevel l>
   bool PresetDiscoveryIndexer<h, l>::indexerDeclareFiletype(
      const struct clap_preset_discovery_indexer *indexer,
      const clap_preset_discovery_filetype_t *filetype) {
      auto &self = from(indexer);
      return self.declareFiletype(filetype);
   }

   template <MisbehaviourHandler h, CheckingLevel l>
   bool PresetDiscoveryIndexer<h, l>::indexerDeclareLocation(
      const struct clap_preset_discovery_indexer *indexer,
      const clap_preset_discovery_location_t *location) {
      auto &self = from(indexer);
      return self.declareLocation(location);
   }

   template <MisbehaviourHandler h, CheckingLevel l>
   bool PresetDiscoveryIndexer<h, l>::indexerDeclareSoundPack(
      const struct clap_preset_discovery_indexer *indexer,
      const clap_preset_discovery_soundpack_t *soundpack) {
      auto &self = from(indexer);
      return self.declareSoundPack(soundpack);
   }

   template <MisbehaviourHandler h, CheckingLevel l>
   const void *
   PresetDiscoveryIndexer<h, l>::indexerGetExtension(const clap_preset_discovery_indexer *indexer,
                                                     const char *id) noexcept {
      auto &self = from(indexer);
      return self.extension(id);
   }
}} // namespace clap::helpers