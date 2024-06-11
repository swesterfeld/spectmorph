#pragma once

#include <clap/ext/context-menu.h>

namespace clap { namespace helpers {

   class ContextMenuBuilder {
   public:
      // not copyable, not moveable
      ContextMenuBuilder(const ContextMenuBuilder &) = delete;
      ContextMenuBuilder(ContextMenuBuilder &&) = delete;
      ContextMenuBuilder &operator=(const ContextMenuBuilder &) = delete;
      ContextMenuBuilder &operator=(ContextMenuBuilder &&) = delete;

      virtual bool addItem(clap_context_menu_item_kind_t item_kind, const void *item_data) = 0;

      virtual bool supports(clap_context_menu_item_kind_t item_kind) const noexcept = 0;

      const clap_context_menu_builder *builder() const noexcept { return &_builder; }

   protected:
      ContextMenuBuilder() = default;
      virtual ~ContextMenuBuilder() = default;

   private:
      static bool clapAddItem(const struct clap_context_menu_builder *builder,
                              clap_context_menu_item_kind_t item_kind,
                              const void *item_data) noexcept {
         auto *self = static_cast<ContextMenuBuilder *>(builder->ctx);
         return self->addItem(item_kind, item_data);
      }

      static bool clapSupports(const struct clap_context_menu_builder *builder,
                               clap_context_menu_item_kind_t item_kind) noexcept {
         auto *self = static_cast<ContextMenuBuilder *>(builder->ctx);
         return self->supports(item_kind);
      }

      const clap_context_menu_builder _builder = {this, &clapAddItem, &clapSupports};
   };

}} // namespace clap::helpers
