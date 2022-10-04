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

#ifndef HASH_H
#define HASH_H

#include "array_rn.h"
#include "main.h"
#include "move.h"

#include <random>

namespace hash {

  typedef array_rn<uint64_t, N_PT, N_SQ> hash_piece_t;
  typedef array_rn<uint64_t, N_PT, N_SQ, N_SQ> hash_move_t;

  static std::mt19937_64 rng(1632933495);

  static uint64_t rd_key() {
    uint64_t r;
    int bit_count;

    // filter high/low bit.counts
    do {
      r         = rng();
      bit_count = __builtin_popcountll(r);
    } while (bit_count < 3 || bit_count > 61);

    return r;
  }
  static hash_piece_t make_piece_keys() {

    hash_piece_t r;
    for (auto sq : UNIVERSE)
      for (int p = EMPTY; p <= WK; p++)
        r[p][sq] = rd_key();

    return r;
  }
  static const hash_piece_t psKey = make_piece_keys();
  static hash_move_t make_move_keys() {

    hash_move_t r;
    for (auto fr : UNIVERSE)
      for (auto to : UNIVERSE)
        for (int p = EMPTY; p <= WK; p++)
          r[p][fr][to] = psKey[p][fr] ^ psKey[p][to];

    return r;
  }
  static const hash_move_t moveKey = make_move_keys();

  static constexpr uint64_t key(int p, int sq) {
    return psKey[p][sq];
  }
  static constexpr uint64_t move_key(int p, int fr, int to) {
    return moveKey[p][fr][to];
  }
} // namespace hash

namespace ht {

  constexpr int SIZE_A = 15753211; // @all 256 MB

  inline int in(int sc, int ply) {
    assert(sc != SCORE_NONE);
    return sc >= MATT_IN_MAX ? sc + ply : sc <= -MATT_IN_MAX ? sc - ply : sc;
  }
  inline int out(int sc, int ply, int r50) {
    if (sc == SCORE_NONE)
      return SCORE_NONE;

    if (sc >= MATT_IN_MAX) {
      if (MATT - sc > 99 - r50)
        return MATT_IN_MAX - 1;
      return sc - ply;
    }
    if (sc <= -MATT_IN_MAX) {
      if (-MATT + sc > 99 - r50)
        return -MATT_IN_MAX + 1;
      return sc + ply;
    }
    return sc;
  }

  struct Entry {
    enum : uint8_t { UB = 1, LB = 2, XB = 3, PV = 4 };

    uint64_t key_   = 0;
    uint16_t move_  = 0;
    int16_t score_  = SCORE_NONE;
    int8_t depth_   = 0;
    uint8_t flag_   = 0;
  };

  class Table : public std::array<Entry, SIZE_A> {

   public:
    Table() {
    }
    void clear() {
      fill(Entry());
    }
    Entry* e(uint64_t key, int depth) {
      Entry* a = this->begin() + (key % SIZE_A);
      if (key != a->key_ || depth > a->depth_)
        return nullptr;
      return a;
    }
    void rem(uint64_t key, Move m, int depth, int sc, int flag) {
      Entry* a = this->begin() + (key % SIZE_A);
      if (key != a->key_ || depth > a->depth_) {
        a->key_   = key;
        a->move_  = m.frto();
        a->score_ = sc;
        a->depth_ = depth;
        a->flag_  = flag;
      }
    }
    uint16_t move(uint64_t key) {
      Entry* a = this->begin() + (key % SIZE_A);
      return a->key_ == key ? a->move_ : 0;
    }
  };
} // namespace ht

#endif