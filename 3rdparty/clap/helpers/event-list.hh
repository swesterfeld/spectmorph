#pragma once

#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <vector>
#include <functional>
#include <algorithm>

#include <clap/events.h>

#include "heap.hh"

namespace clap { namespace helpers {

   class EventList {
   public:
      using SpaceResolver = std::function<clap_id(clap_id)>;

      explicit EventList(uint32_t initialHeapSize = 4096,
                         uint32_t initialEventsCapacity = 128,
                         uint32_t maxEventSize = 1024,
                         const SpaceResolver& spaceResolver = {})
         : _maxEventSize(maxEventSize), _heap(initialHeapSize) {
         _events.reserve(initialEventsCapacity);

         // TODO: resolve known events space
      }

      EventList(const EventList &) = delete;
      EventList(EventList &&) = delete;
      EventList &operator=(const EventList &) = delete;
      EventList &operator=(EventList &&) = delete;

      static const constexpr size_t SAFE_ALIGN = 8;

      void reserveEvents(size_t capacity) { _events.reserve(capacity); }

      void reserveHeap(size_t size) { _heap.reserve(size); }

      clap_event_header *allocate(size_t align, size_t size) {
         assert(size >= sizeof(clap_event_header));
         if (size > _maxEventSize)
            throw std::invalid_argument("size is greater than the maximum allowed event size");

         // ensure we have space to store into the vector
         if (_events.capacity() == _events.size())
            _events.reserve(_events.capacity() * 2);

         auto ptr = _heap.allocate(align, size);
         _events.push_back(static_cast<uint32_t>(_heap.offsetFromBase(ptr)));
         auto hdr = static_cast<clap_event_header *>(ptr);
         hdr->size = static_cast<uint32_t>(size);
         return hdr;
      }

      clap_event_header *tryAllocate(size_t align, size_t size) {
         assert(size >= sizeof(clap_event_header));
         if (size > _maxEventSize)
            return nullptr;

         // ensure we have space to store into the vector
         if (_events.capacity() == _events.size())
            return nullptr;

         auto ptr = _heap.tryAllocate(align, size);
         if (!ptr)
            return nullptr;

         _events.push_back(static_cast<uint32_t>(_heap.offsetFromBase(ptr)));
         auto hdr = static_cast<clap_event_header *>(ptr);
         hdr->size = static_cast<uint32_t>(size);
         return hdr;
      }

      template <typename T>
      T *allocate() {
         return allocate(alignof(T), sizeof(T));
      }

      void push(const clap_event_header *h) {
         if (_events.size() == _events.capacity())
            _events.reserve(_events.capacity() * 2);

         if (!tryPush(h)) {
            growHeap(h->size);

            if (!tryPush(h)) {
               // It is very likely that grow heap didn't allocate enough space, so check for a bug
               // in growHeap()
               throw std::bad_alloc();
            }
         }
      }

      bool tryPush(const clap_event_header *h) {
         auto ptr = tryAllocate(SAFE_ALIGN, h->size);
         if (!ptr)
            return false;

         std::memcpy(ptr, h, h->size);

         if (h->space_id == CLAP_CORE_EVENT_SPACE_ID) {
            switch (h->type) {
            case CLAP_EVENT_MIDI_SYSEX: {
               auto ev = reinterpret_cast<clap_event_midi_sysex *>(ptr);
               auto buffer = static_cast<uint8_t *>(_heap.tryAllocate(1, ev->size));
               if (!buffer) {
                  _events.pop_back();
                  return false;
               }
               std::copy_n(ev->buffer, ev->size, buffer);
               ev->buffer = buffer;
               _canReallocHeap = false;
            } break;
            }
         }

         return true;
      }

      clap_event_header *get(uint32_t index) const {
         const auto offset = _events.at(index);
         auto const ptr = _heap.ptrFromBase(offset);
         return static_cast<clap_event_header *>(ptr);
      }

      size_t size() const { return _events.size(); }

      bool empty() const { return _events.empty(); }

      void clear() {
         _heap.clear();
         _events.clear();
         _canReallocHeap = true;
      }

      const clap_input_events *clapInputEvents() const noexcept { return &_inputEvents; }

      const clap_output_events *clapOutputEvents() const noexcept { return &_outputEvents; }

      const clap_output_events *clapBoundedOutputEvents() const noexcept {
         return &_boundedOutputEvents;
      }

      Heap &heap() { return _heap; }
      const Heap &heap() const { return _heap; }

   private:
      static uint32_t clapSize(const struct clap_input_events *list) {
         auto *self = static_cast<const EventList *>(list->ctx);
         return static_cast<uint32_t>(self->size());
      }

      static const clap_event_header_t *clapGet(const struct clap_input_events *list,
                                                uint32_t index) {
         auto *self = static_cast<const EventList *>(list->ctx);
         return self->get(index);
      }

      static bool clapPushBack(const struct clap_output_events *list,
                               const clap_event_header *event) {
         auto *self = static_cast<EventList *>(list->ctx);
         self->push(event);
         return true;
      }

      static bool clapBoundedPushBack(const struct clap_output_events *list,
                                      const clap_event_header_t *event) {
         auto *self = static_cast<EventList *>(list->ctx);
         return self->tryPush(event);
      }

      void growHeap(size_t minFreeSpace) {
         const size_t oldSize = _heap.size();
         const size_t newSize = oldSize + std::max(oldSize, 16 * minFreeSpace);

         if (_canReallocHeap) {
            _heap.reserve(newSize);
            return;
         }

         std::vector<uint32_t> events(std::move(_events));
         _events = std::vector<uint32_t>(events.capacity());

         Heap heap(std::move(_heap));
         _heap.reserve(newSize);

         // we need to perform a full copy of all the events in case there was some pointers in
         for (auto offset : events) {
            auto const ptr = heap.ptrFromBase(offset);
            auto hdr = static_cast<clap_event_header *>(ptr);
            push(hdr);
         }
      }

      const clap_input_events _inputEvents = {this, &clapSize, &clapGet};
      const clap_output_events _outputEvents = {this, &clapPushBack};
      const clap_output_events _boundedOutputEvents = {this, &clapBoundedPushBack};

      const uint32_t _maxEventSize;

      Heap _heap;
      std::vector<uint32_t> _events;

      // If the heap can be realloc()'ated or not.
      // If any complex event involving pointers, like sysex are in the list then it can't be
      // simply reallocated, and needs to go through the creation of a new heap and vector and
      // re-insert all elements again.
      bool _canReallocHeap = true;
   };

}} // namespace clap::helpers
