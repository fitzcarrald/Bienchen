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

#ifndef DB_H
#define DB_H

#include "array_rn.h"
#include "main.h"

namespace db {

  /// lookup's
  typedef array_rn<u64, N_SQ> R_U;
  typedef array_rn<u64, N_CLR, N_SQ> R_2U;
  typedef array_rn<u64, N_SQ, N_SQ> R_UU;
  typedef array_rn<u64, N_SQ, 512> b_magic_t;
  typedef array_rn<u64, N_SQ, 4096> r_magic_t;

  enum { B, R };

  /// ini
  static constexpr u64 CLEAR_A = 0xFEFEFEFEFEFEFEFE;
  static constexpr u64 CLEAR_H = 0x7F7F7F7F7F7F7F7F;

  // std::abs isn't constexpr
  static constexpr int my_abs(int a) {

    return a < 0 ? -a : a;
  }

  /// between ////////////////////////////

  static constexpr u64 bb_between(u64 u) {
    if (u.count() != 2)
      return 0x0;

    int hi = u.lsbi();
    int lo = u.msbi();

    int fhi = hi & 7;
    int flo = lo & 7;
    int rhi = hi >> 3;
    int rlo = lo >> 3;

    u64 R = (R_MASK[hi] & R_MASK[lo]);
    u64 B = (B_MASK[hi] & B_MASK[lo]);

    u64 U = ~1ULL << lo;
    u64 L = (1ULL << hi) - 1;

    if (fhi == flo || rhi == rlo)
      return ~(U | L) & R;
    else if (my_abs(hi - lo) % 7 == 0 || my_abs(hi - lo) % 9 == 0)
      return ~(U | L) & B;

    return 0;
  }
  static constexpr R_UU make_between() {

    R_UU r;
    for (auto x : UNIVERSE)
      for (auto y : UNIVERSE)
        r[x][y] = bb_between((1ULL << x) | (1ULL << y));

    return r;
  }
  static constexpr R_UU between_ = make_between();

  /// magic's ///////////////////////////

  static constexpr u64 gen_rook(u64 op_sig, u64 bit) {
    u64 pos = bit;
    u64 to  = 0;

    while (pos <<= 8) {
      to |= pos;
      if (pos & op_sig)
        break;
    }

    pos = bit;
    while (pos >>= 8) {
      to |= pos;
      if (pos & op_sig)
        break;
    }

    pos = bit;
    while ((pos <<= 1) & CLEAR_A) {
      to |= pos;
      if (pos & op_sig)
        break;
    }

    pos = bit;
    while ((pos >>= 1) & CLEAR_H) {
      to |= pos;
      if (pos & op_sig)
        break;
    }
    return to;
  }
  static constexpr u64 gen_bishop(u64 op_sig, u64 bit) {
    u64 pos = bit;
    u64 to  = 0;

    while ((pos <<= 9) & CLEAR_A) {
      to |= pos;
      if (pos & op_sig)
        break;
    }

    pos = bit;
    while ((pos <<= 7) & CLEAR_H) {
      to |= pos;
      if (pos & op_sig)
        break;
    }

    pos = bit;
    while ((pos >>= 7) & CLEAR_A) {
      to |= pos;
      if (pos & op_sig)
        break;
    }

    pos = bit;
    while ((pos >>= 9) & CLEAR_H) {
      to |= pos;
      if (pos & op_sig)
        break;
    }
    return to;
  }
  static constexpr R_2U make_sig_mask() {

    R_2U r;

    u64 left   = FILE_MASK[0];
    u64 right  = FILE_MASK[7];
    u64 top    = RANK_MASK[7];
    u64 bottom = RANK_MASK[0];

    u64 corner = bit(A1) | bit(H1) | bit(A8) | bit(H8);

    for (auto i : UNIVERSE) {
      u64 sig = left | right | top | bottom;

      if (bit(i) & left)
        sig &= ~left;
      if (bit(i) & right)
        sig &= ~right;
      if (bit(i) & top)
        sig &= ~top;
      if (bit(i) & bottom)
        sig &= ~bottom;

      r[B][i] = B_MASK[i] & ~(bit(i)) & ~sig & ~corner;
      r[R][i] = R_MASK[i] & ~(bit(i)) & ~sig & ~corner;
    }
    return r;
  }
  static constexpr R_2U sig_mask_ = make_sig_mask();

  static constexpr b_magic_t make_b() {

    b_magic_t r;

    for (auto sq : UNIVERSE) {
      for (int occ = 0; occ < 512; ++occ) {
        u64 att = sig_mask_[B][sq];
        u64 blx = 0;
        int i   = 0;

        for (auto s : att)
          if (occ & bit(i++))
            blx |= bit(s);

        r[sq][blx * B_MAGIC[sq] >> 55] = gen_bishop(blx, bit(sq));
      }
    }
    return r;
  }
  static constexpr r_magic_t make_r() {

    r_magic_t r;

    for (auto sq : UNIVERSE) {
      for (int occ = 0; occ < 4096; occ++) {
        u64 att = sig_mask_[R][sq];
        u64 blx = 0;
        int i   = 0;

        for (auto s : att)
          if (occ & bit(i++))
            blx |= bit(s);

        r[sq][blx * R_MAGIC[sq] >> 52] = gen_rook(blx, bit(sq));
      }
    }
    return r;
  }
  static const b_magic_t bsop_atck_ = make_b();
  static const r_magic_t rook_atck_ = make_r();

  /// pawn's ///////////////////////////

  static consteval R_2U make_att_by_pawn() {

    R_2U r;
    for (auto i : UNIVERSE) {
      u64 b    = bit(i);
      r[B_][i] = ((b << 9) & CLEAR_A) | ((b << 7) & CLEAR_H);
      r[W_][i] = ((b >> 9) & CLEAR_H) | ((b >> 7) & CLEAR_A);
    }
    return r;
  }
  static consteval R_2U make_passed() {

    R_2U r;
    // no pawn squares
    for (auto sq : PROM)
      r[B_][sq] = r[W_][sq] = 0;
    // pawn squares
    for (auto sq : u64(~PROM)) {

      u64 w_mask = 0;
      u64 b_mask = 0;
      u64 wi     = bit(sq + 8);
      u64 bi     = bit(sq - 8);

      while (bi) {
        b_mask |= ((bi << 1) & CLEAR_A) | bi | ((bi >> 1) & CLEAR_H);
        bi >>= 8;
      }
      while (wi) {
        w_mask |= ((wi << 1) & CLEAR_A) | wi | ((wi >> 1) & CLEAR_H);
        wi <<= 8;
      }

      r[B_][sq] = b_mask;
      r[W_][sq] = w_mask;
    }
    return r;
  }
  static constexpr R_2U passed_ = make_passed();

  static consteval R_U make_isolated() {

    R_U r;
    // no pawn squares
    for (auto sq : PROM)
      r[sq] = 0;
    // pawn squares
    for (auto sq : u64(~PROM)) {

      int f = sq % 8;
      if (f == 0)
        r[sq] = FILE_MASK[1];
      else if (f > 0 && f < 7)
        r[sq] = FILE_MASK[f - 1] | FILE_MASK[f + 1];
      else if (f == 7)
        r[sq] = FILE_MASK[6];
      else
        assert(false);
    }
    return r;
  }
  static consteval R_2U make_forward_file() {

    R_2U r;
    for (auto sq : UNIVERSE) {
      r[B_][sq] = passed_[B_][sq] & FILE_MASK[sq & 7];
      r[W_][sq] = passed_[W_][sq] & FILE_MASK[sq & 7];
    }
    return r;
  }
  static consteval R_2U make_forward_rank() {

    R_2U r;
    for (auto sq : PROM)
      r[B_][sq] = r[W_][sq] = 0;
    for (auto sq : u64(~PROM)) {

      u64 wi = bit(sq + 8);
      u64 bi = bit(sq - 8);

      while (bi) {
        r[B_][sq] |= RANK_MASK[bi.bti() >> 3];
        bi >>= 8;
      }
      while (wi) {
        r[W_][sq] |= RANK_MASK[wi.bti() >> 3];
        wi <<= 8;
      }
    }
    return r;
  }

  static constexpr R_2U att_by_pawn_  = make_att_by_pawn();
  static constexpr R_U isolated_      = make_isolated();
  static constexpr R_2U forward_rank_ = make_forward_rank();
  static constexpr R_2U forward_file_ = make_forward_file();

  /// get /////////////////////////////

  static constexpr u64 between(int s1, int s2) {
    return between_[s1][s2];
  }
  static constexpr u64 att_by_pawn(int sq, bool s) {
    return att_by_pawn_[s][sq];
  }
  static constexpr u64 sig_bsop(int sq) {
    return sig_mask_[B][sq];
  }
  static constexpr u64 sig_rook(int sq) {
    return sig_mask_[R][sq];
  }
  static constexpr u64 magic_bsop(int sq, int sig) {
    return bsop_atck_[sq][sig];
  }
  static constexpr u64 magic_rook(int sq, int sig) {
    return rook_atck_[sq][sig];
  }
  static constexpr u64 passed(int sq, int s) {
    return passed_[s][sq];
  }
  static constexpr u64 isolated(int sq) {
    return isolated_[sq];
  }
  static constexpr u64 forward_file(int sq, bool s) {
    return forward_file_[s][sq];
  }
  static constexpr u64 forward_rank(int sq, bool s) {
    return forward_rank_[s][sq];
  }
} // namespace db

#endif