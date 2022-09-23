/*---------------------------------------------------------------------

  Bienchen - UCI chess engine
  © 2022 Manuel Schenske

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program. If not, see <https://www.gnu.org/licenses/>.

---------------------------------------------------------------------*/

#ifndef BITS_H
#define BITS_H

#include <cstdint>
#include <ostream>

class Bit_iterator {

 public:
  constexpr Bit_iterator(uint64_t set) : m_(set) {
  }
  // lsb 0-based
  constexpr int operator*() const {
    return __builtin_ffsll(m_) - 1;
  }
  // lsb flip
  constexpr Bit_iterator& operator++() {
    m_ &= m_ - 1;
    return *this;
  }
  constexpr bool operator!=(const Bit_iterator& o) {
    return m_ != o.m_;
  }

 private:
  uint64_t m_;
};

class u64 {

 public:
  constexpr u64() : m_(0){};
  constexpr u64(uint64_t u) : m_(u) {
  }

  constexpr u64& operator=(const uint64_t u) {
    m_ = u;
    return *this;
  }
  constexpr u64& operator&=(const u64& o) {
    m_ &= o.m_;
    return *this;
  }
  constexpr u64& operator|=(const u64& o) {
    m_ |= o.m_;
    return *this;
  }
  constexpr u64& operator^=(const u64& o) {
    m_ ^= o.m_;
    return *this;
  }
  constexpr u64& operator<<=(int s) {
    m_ <<= s;
    return *this;
  }
  constexpr u64& operator>>=(int s) {
    m_ >>= s;
    return *this;
  }
  constexpr u64& shift_down(bool side) {
    m_ = side ? m_ >> 8 : m_ << 8;
    return *this;
  }
  constexpr u64& shift_up(bool side) {
    return shift_down(!side);
  }
  constexpr int lsbi() const {
    return __builtin_ffsll(m_) - 1;
  }
  constexpr int msbi() const {
    return 63 - __builtin_clzll(m_);
  }
  constexpr int bti() const {
    return __builtin_popcountll(m_ - 1);
  }
  constexpr int count() const {
    return __builtin_popcountll(m_);
  }
  constexpr Bit_iterator begin() const {
    return m_;
  }
  constexpr Bit_iterator end() const {
    return 0;
  }
  constexpr operator uint64_t() const {
    return m_;
  }

 private:
  uint64_t m_;
};

static constexpr u64 bit(int sq) {
  return 1ULL << sq;
}

std::ostream& operator<<(std::ostream& os, u64& u);

#endif