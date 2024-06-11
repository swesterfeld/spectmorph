#pragma once

#include <clap/clap.h>

#include "checking-level.hh"
#include "misbehaviour-handler.hh"

namespace clap { namespace helpers {

   /// @brief C++ glue and checks
   template <MisbehaviourHandler h, CheckingLevel l>
   class PresetDiscoveryMetadataReceiver {
   public:
      const clap_preset_discovery_metadata_receiver *receiver() const noexcept {
         return &_receiver;
      }

      // not copyable, not moveable
      PresetDiscoveryMetadataReceiver(const PresetDiscoveryMetadataReceiver &) = delete;
      PresetDiscoveryMetadataReceiver(PresetDiscoveryMetadataReceiver &&) = delete;
      PresetDiscoveryMetadataReceiver &operator=(const PresetDiscoveryMetadataReceiver &) = delete;
      PresetDiscoveryMetadataReceiver &operator=(PresetDiscoveryMetadataReceiver &&) = delete;

   protected:
      PresetDiscoveryMetadataReceiver();
      virtual ~PresetDiscoveryMetadataReceiver() = default;

      /////////////////////////
      // Methods to override //
      /////////////////////////

      //--------------------------------//
      // clap_preset_discovery_provider //
      //--------------------------------//
      virtual void onError(int32_t os_error, const char *error_message) noexcept {}

      virtual bool beginPreset(const char *name, const char *load_key) noexcept { return false; }

      virtual void addPluginId(const clap_universal_plugin_id *plugin_id) noexcept {}

      virtual void setCollectionId(const char *collection_id) noexcept {}

      virtual void setFlags(uint32_t flags) noexcept {}

      virtual void addCreator(const char *creator) noexcept {}

      virtual void setDescription(const char *description) noexcept {}

      virtual void setTimestamps(clap_timestamp creation_time,
                                 clap_timestamp modification_time) noexcept {}

      virtual void addFeature(const char *feature) noexcept {}

      virtual void addExtraInfo(const char *key, const char *value) noexcept {}

      ///////////////
      // Utilities //
      ///////////////
      static PresetDiscoveryMetadataReceiver &
      from(const clap_preset_discovery_metadata_receiver *receiver) noexcept;

   private:
      /////////////////////
      // CLAP Interfaces //
      /////////////////////

      const clap_preset_discovery_metadata_receiver _receiver;

      static void receiverOnError(const struct clap_preset_discovery_metadata_receiver *receiver,
                                  int32_t os_error,
                                  const char *error_message) noexcept;

      static bool
      receiverBeginPreset(const struct clap_preset_discovery_metadata_receiver *receiver,
                          const char *name,
                          const char *load_key) noexcept;

      static void
      receiverAddPluginId(const struct clap_preset_discovery_metadata_receiver *receiver,
                          const clap_universal_plugin_id *plugin_id) noexcept;

      static void
      receiverSetCollectionId(const struct clap_preset_discovery_metadata_receiver *receiver,
                              const char *collection_id) noexcept;

      static void receiverSetFlags(const struct clap_preset_discovery_metadata_receiver *receiver,
                                   uint32_t flags) noexcept;

      static void receiverAddCreator(const struct clap_preset_discovery_metadata_receiver *receiver,
                                     const char *creator) noexcept;

      static void
      receiverSetDescription(const struct clap_preset_discovery_metadata_receiver *receiver,
                             const char *description) noexcept;

      static void
      receiverSetTimestamps(const struct clap_preset_discovery_metadata_receiver *receiver,
                            clap_timestamp creation_time,
                            clap_timestamp modification_time) noexcept;

      static void receiverAddFeature(const struct clap_preset_discovery_metadata_receiver *receiver,
                                     const char *feature) noexcept;

      static void
      receiverAddExtraInfo(const struct clap_preset_discovery_metadata_receiver *receiver,
                           const char *key,
                           const char *value) noexcept;
   };
}} // namespace clap::helpers
