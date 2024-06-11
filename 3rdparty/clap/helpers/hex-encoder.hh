#pragma once

#include <cassert>
#include <cstring>
#include <stdexcept>
#include <string>
#include <vector>

namespace clap { namespace helpers {
   inline std::string hex_encode(const void *data, std::size_t size) {
      static const char dict[] = "0123456789abcdef";
      std::string result;
      result.reserve(2 * size);

      auto start = static_cast<const uint8_t *>(data);
      auto end = start + size;

      for (auto it = start; it < end; ++it) {
         const auto v = *it;
         result.push_back(dict[(v >> 4) & 0xF]);
         result.push_back(dict[v & 0xF]);
      }
      return result;
   }

   inline uint8_t hex_decode_char(char c) {
      if ('0' <= c && c <= '9')
         return c - '0';
      if ('a' <= c && c <= 'f')
         return c - 'a' + 0xa;
      if ('A' <= c && c <= 'F')
         return c - 'A' + 0xa;

      throw std::invalid_argument("not an hexadecimal character");
   }

   inline std::vector<uint8_t> hex_decode(const char *str, size_t len) {
      static const char dict[] = "0123456789abcdef";

      if (len & 1)
         throw std::invalid_argument("the string length must be a multiple of two");

      std::vector<uint8_t> result;
      result.reserve(len / 2);

      for (size_t i = 0; i + 1 < len; i += 2) {
         const uint8_t v1 = hex_decode_char(str[i]);
         const uint8_t v2 = hex_decode_char(str[i + 1]);

         assert(0 <= v1 && v1 <= 0xF);
         assert(0 <= v2 && v2 <= 0xF);

         const uint8_t v = (v1 << 4) | v2;
         result.push_back(v);
      }
      return result;
   }

   inline std::vector<uint8_t> hex_decode(const char *str) {
      return hex_decode(str, std::strlen(str));
   }

   inline std::vector<uint8_t> hex_decode(const std::string& str) {
      return hex_decode(str.c_str(), str.size());
   }
}} // namespace clap::helpers