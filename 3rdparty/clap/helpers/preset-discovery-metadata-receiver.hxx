#pragma once

#include <iostream>

#include "preset-discovery-metadata-receiver.hh"

namespace clap { namespace helpers {

   template <MisbehaviourHandler h, CheckingLevel l>
   PresetDiscoveryMetadataReceiver<h, l>::PresetDiscoveryMetadataReceiver()
      : _receiver({
           this,
           receiverOnError,
           receiverBeginPreset,
           receiverAddPluginId,
           receiverSetCollectionId,
           receiverSetFlags,
           receiverAddCreator,
           receiverSetDescription,
           receiverSetTimestamps,
           receiverAddFeature,
           receiverAddExtraInfo,
        }) {}

   template <MisbehaviourHandler h, CheckingLevel l>
   PresetDiscoveryMetadataReceiver<h, l> &PresetDiscoveryMetadataReceiver<h, l>::from(
      const clap_preset_discovery_metadata_receiver *receiver) noexcept {
      return *static_cast<PresetDiscoveryMetadataReceiver<h, l> *>(receiver->receiver_data);
   }

   template <MisbehaviourHandler h, CheckingLevel l>
   void PresetDiscoveryMetadataReceiver<h, l>::receiverOnError(
      const struct clap_preset_discovery_metadata_receiver *receiver,
      int32_t os_error,
      const char *error_message) noexcept {
      auto &self = from(receiver);
      return self.onError(os_error, error_message);
   }

   template <MisbehaviourHandler h, CheckingLevel l>
   bool PresetDiscoveryMetadataReceiver<h, l>::receiverBeginPreset(
      const struct clap_preset_discovery_metadata_receiver *receiver,
      const char *name,
      const char *load_key) noexcept {
      auto &self = from(receiver);
      return self.beginPreset(name, load_key);
   }

   template <MisbehaviourHandler h, CheckingLevel l>
   void PresetDiscoveryMetadataReceiver<h, l>::receiverAddPluginId(
      const struct clap_preset_discovery_metadata_receiver *receiver,
      const clap_universal_plugin_id *plugin_id) noexcept {
      auto &self = from(receiver);
      return self.addPluginId(plugin_id);
   }

   template <MisbehaviourHandler h, CheckingLevel l>
   void PresetDiscoveryMetadataReceiver<h, l>::receiverSetCollectionId(
      const struct clap_preset_discovery_metadata_receiver *receiver,
      const char *collection_id) noexcept {
      auto &self = from(receiver);
      return self.setCollectionId(collection_id);
   }

   template <MisbehaviourHandler h, CheckingLevel l>
   void PresetDiscoveryMetadataReceiver<h, l>::receiverSetFlags(
      const struct clap_preset_discovery_metadata_receiver *receiver, uint32_t flags) noexcept {
      auto &self = from(receiver);
      return self.setFlags(flags);
   }

   template <MisbehaviourHandler h, CheckingLevel l>
   void PresetDiscoveryMetadataReceiver<h, l>::receiverAddCreator(
      const struct clap_preset_discovery_metadata_receiver *receiver,
      const char *creator) noexcept {
      auto &self = from(receiver);
      return self.addCreator(creator);
   }

   template <MisbehaviourHandler h, CheckingLevel l>
   void PresetDiscoveryMetadataReceiver<h, l>::receiverSetDescription(
      const struct clap_preset_discovery_metadata_receiver *receiver,
      const char *description) noexcept {
      auto &self = from(receiver);
      return self.setDescription(description);
   }

   template <MisbehaviourHandler h, CheckingLevel l>
   void PresetDiscoveryMetadataReceiver<h, l>::receiverSetTimestamps(
      const struct clap_preset_discovery_metadata_receiver *receiver,
      clap_timestamp creation_time,
      clap_timestamp modification_time) noexcept {
      auto &self = from(receiver);
      return self.setTimestamps(creation_time, modification_time);
   }

   template <MisbehaviourHandler h, CheckingLevel l>
   void PresetDiscoveryMetadataReceiver<h, l>::receiverAddFeature(
      const struct clap_preset_discovery_metadata_receiver *receiver,
      const char *feature) noexcept {
      auto &self = from(receiver);
      return self.addFeature(feature);
   }

   template <MisbehaviourHandler h, CheckingLevel l>
   void PresetDiscoveryMetadataReceiver<h, l>::receiverAddExtraInfo(
      const struct clap_preset_discovery_metadata_receiver *receiver,
      const char *key,
      const char *value) noexcept {
      auto &self = from(receiver);
      return self.addExtraInfo(key, value);
   }
}} // namespace clap::helpers
