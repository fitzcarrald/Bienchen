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

#ifndef PST_H
#define PST_H

#include "array_rn.h"
#include "fen.h"
#include "main.h"
#include "move.h"

namespace pst {

  typedef array_rn<int, N_PT, N_SQ, N_SQ> delta_t;

  // clang-format off
  static constexpr int pst[] = {

    // mod of PeSTO's PSTable
    // https://www.chessprogramming.org/PeSTO's_Evaluation_Function
    
    // mg pawn
      0,   0,   0,   0,   0,   0,   0,   0,
     98, 134,  61,  95,  68, 126,  34, -11,
     -6,   7,  26,  31,  65,  56,  25, -20,
    -14,  13,   6,  21,  23,  12,  17, -23,
    -27,  -2,  -5,  12,  17,   6,  10, -25,
    -26,  -4,  -4, -10,   3,   3,  33, -12,
    -35,  -1, -20, -23, -15,  24,  38, -22,
      0,   0,   0,   0,   0,   0,   0,   0,

    // eg pawn
      0,   0,   0,   0,   0,   0,   0,   0,
    178, 173, 158, 134, 147, 132, 165, 187,
     94, 100,  85,  67,  56,  53,  82,  84,
     32,  24,  13,   5,  -2,   4,  17,  17,
     13,   9,  -3,  -7,  -7,  -8,   3,  -1,
      4,   7,  -6,   1,   0,  -5,  -1,  -8,
     13,   8,   8,  10,  13,   0,   2,  -7,
      0,   0,   0,   0,   0,   0,   0,   0,

    // mg knight
   -167, -89, -34, -49, 61, -97, -15, -107,
    -73, -41,  72,  36, 23,  62,   7,  -17,
    -47,  60,  37,  65, 84, 129,  73,   44,
     -9,  17,  19,  53, 37,  69,  18,   22,
    -13,   4,  16,  13, 28,  19,  21,   -8,
    -23,  -9,  12,  10, 19,  17,  25,  -16,
    -29, -53, -12,  -3, -1,  18, -14,  -19,
   -105, -21, -58, -33, -17, -28, -19, -23,

    // eg knight
    -58, -38, -13, -28, -31, -27, -63, -99,
    -25,  -8, -25,  -2,  -9, -25, -24, -52,
    -24, -20,  10,   9,  -1,  -9, -19, -41,
    -17,   3,  22,  22,  22,  11,   8, -18,
    -18,  -6,  16,  25,  16,  17,   4, -18,
    -23,  -3,  -1,  15,  10,  -3, -20, -22,
    -42, -20, -10,  -5,  -2, -20, -23, -44,
    -29, -51, -23, -15, -22, -18, -50, -64,

    // mg bishop
    -29,   4, -82, -37, -25, -42,   7,  -8,
    -26,  16, -18, -13,  30,  59,  18, -47,
    -16,  37,  43,  40,  35,  50,  37,  -2,
     -4,   5,  19,  50,  37,  37,   7,  -2,
     -6,  13,  13,  26,  34,  12,  10,   4,
      0,  15,  15,  15,  14,  27,  18,  10,
      4,  15,  16,   0,   7,  21,  33,   1,
    -33,  -3, -14, -21, -13, -12, -39, -21,

    // eg bishop
    -14, -21, -11,  -8,  -7,  -9, -17, -24,
     -8,  -4,   7, -12,  -3, -13,  -4, -14,
      2,  -8,   0,  -1,  -2,   6,   0,   4,
     -3,   9,  12,   9,  14,  10,   3,   2,
     -6,   3,  13,  19,   7,  10,  -3,  -9,
    -12,  -3,   8,  10,  13,   3,  -7, -15,
    -14, -18,  -7,  -1,   4,  -9, -15, -27,
    -23,  -9, -23,  -5,  -9, -16,  -5, -17,

    // mg rook
     32,  42,  32,  51,  63,   9,  31,  43,
     27,  32,  58,  62,  80,  67,  26,  44,
     -5,  19,  26,  36,  17,  45,  61,  16,
    -24, -11,   7,  26,  24,  35,  -8, -20,
    -36, -26, -12,  -1,   9,  -7,   6, -23,
    -45, -25, -16, -17,   3,   0,  -5, -33,
    -44, -16, -20,  -9,  -1,  11,  -6, -71,
    -19, -13,   1,  17,  16,   7, -37, -26,

    // eg rook
     13,  10,  18,  15,  12,  12,   8,   5,
     11,  13,  13,  11,  -3,   3,   8,   3,
      7,   7,   7,   5,   4,  -3,  -5,  -3,
      4,   3,  13,   1,   2,   1,  -1,   2,
      3,   5,   8,   4,  -5,  -6,  -8, -11,
     -4,   0,  -5,  -1,  -7, -12,  -8, -16,
     -6,  -6,   0,   2,  -9,  -9, -11,  -3,
     -9,   2,   3,  -1,  -5, -13,   4, -20,

    // mg queen
    -28,   0,  29,  12,  59,  44,  43,  45,
    -24, -39,  -5,   1, -16,  57,  28,  54,
    -13, -17,   7,   8,  29,  56,  47,  57,
    -27, -27, -16, -16,  -1,  17,  -2,   1,
     -9, -26,  -9, -10,  -2,  -4,   3,  -3,
    -14,   2, -11,  -2,  -5,   2,  14,   5,
    -35,  -8,  11,   2,   8,  15,  -3,   1,
     -1, -18,  -9,  10, -15, -25, -31, -50,

    // eg queen
     -9,  22,  22,  27,  27,  19,  10,  20,
    -17,  20,  32,  41,  58,  25,  30,   0,
    -20,   6,   9,  49,  47,  35,  19,   9,
      3,  22,  24,  45,  57,  40,  57,  36,
    -18,  28,  19,  47,  31,  34,  39,  23,
    -16, -27,  15,   6,   9,  17,  10,   5,
    -22, -23, -30, -16, -16, -23, -36, -32,
    -33, -28, -22, -43,  -5, -32, -20, -41,

    // mg king
    -65,  23,  16, -15, -56, -34,   2,  13,
     29,  -1, -20,  -7,  -8,  -4, -38, -29,
     -9,  24,   2, -16, -20,   6,  22, -22,
    -17, -20, -12, -27, -30, -25, -14, -36,
    -49,  -1, -27, -39, -46, -44, -33, -51,
    -14, -14, -22, -46, -44, -30, -15, -27,
      1,   7,  -8, -64, -43, -16,   9,   8,
    -15,  36,  12, -54,   8, -28,  24,  14,

    // eg king
    -74, -35, -18, -18, -11,  15,   4, -17,
    -12,  17,  14,  17,  17,  38,  23,  11,
     10,  17,  23,  15,  20,  45,  44,  13,
     -8,  22,  24,  27,  26,  33,  26,   3,
    -18,  -4,  21,  24,  27,  23,   9, -11,
    -19,  -3,  11,  21,  23,  16,   7,  -9,
    -27, -11,   4,  13,  14,   4,  -5, -17,
    -53, -34, -21, -11, -28, -14, -24, -43,

    // piece values mg
    0, 82, 337, 365, 477, 1025, 0,

    // piece values eg
    0, 94, 281, 297, 512, 936, 0};


  // piece counter:
  // LE   BYTE[0] black
  //      BYTE[1] white
  //      BYTE[2] pawns, (0xF black, 0xF0 white)
  //      BYTE[3] knights, (0xF black, 0xF0 white)
  //      .
  //      .
  // yes, we count kings too :-p
  static constexpr uint64_t P_CNT_MASK[N_PT] = {

    0x00000000000000FF,
    0x000000000000FF00,
    0x00000000000F0000,
    0x0000000000F00000,
    0x000000000F000000,
    0x00000000F0000000,
    0x0000000F00000000,
    0x000000F000000000,
    0x00000F0000000000,
    0x0000F00000000000,
    0x000F000000000000,
    0x00F0000000000000,
    0x0F00000000000000,
    0xF000000000000000};

  // operand for color & piece
  static constexpr uint64_t P_CNT[N_PT] = {

    0x0000000000000000,
    0x0000000000000000,
    0x0000000000010001,
    0x0000000000100100,
    0x0000000001000001,
    0x0000000010000100,
    0x0000000100000001,
    0x0000001000000100,
    0x0000010000000001,
    0x0000100000000100,
    0x0001000000000001,
    0x0010000000000100,
    0x0100000000000001,
    0x1000000000000100};

  // clang-format on
  // r-shift for count ...
  static constexpr int SHIFT[N_PT] = { 0,  8,  16, 20, 24, 28, 32,
                                       36, 40, 44, 48, 52, 56, 60 };

  template <bool STAGE>
  static constexpr int index(int p, int sq) {
    assert(p >= BP && p <= WK);
    assert(sq >= A1 && sq <= H8);

    int s = (p & 1) ? fen::index(sq) : sq;
    return ((p >> 1) - 1) * 128 + s + (STAGE ? 64 : 0);
  }
  template <bool STAGE>
  static constexpr int val(int p, int sq) {

    assert(p >= BP && p <= WK);
    assert(sq >= 0 && sq < 64);

    return (p & 1)
           ? pst[index<STAGE>(p, sq)] + pst[(p >> 1) + (STAGE ? 775 : 768)]
           : -pst[index<STAGE>(p, sq)] - pst[(p >> 1) + (STAGE ? 775 : 768)];
  }
  template <bool STAGE>
  static constexpr delta_t make_delta_pst() {

    delta_t r;
    for (int p = BP; p <= WK; p++)
      for (int fr = A1; fr <= H8; fr++)
        for (int to = A1; to <= H8; to++) {
          r[p][fr][to] = val<STAGE>(p, to) - val<STAGE>(p, fr);
        }

    return r;
  }

  static const delta_t delta_pst_mg = make_delta_pst<MG>();
  static const delta_t delta_pst_eg = make_delta_pst<EG>();

  static consteval int move_score(int p, Move m) {
    return delta_pst_mg[p][m.fr()][m.to()];
  }

  /// piece square table
  /// changes @any bord action (set, clear, move)

  class Table {

   public:
    constexpr Table() : p_count_(0), score_mg_(0), score_eg_(0) {
    }
    constexpr Table(const Table& o)
      : p_count_(o.p_count_), score_mg_(o.score_mg_), score_eg_(o.score_eg_) {
    }
    constexpr Table& operator=(const Table& o) {
      p_count_  = o.p_count_;
      score_mg_ = o.score_mg_;
      score_eg_ = o.score_eg_;
      return *this;
    }
    constexpr void clear() {
      p_count_  = 0;
      score_mg_ = 0;
      score_eg_ = 0;
    }
    constexpr void push(int p, int sq) {
      assert(p >= BP && p <= WK);
      assert(sq >= A1 && sq <= H8);

      p_count_ += P_CNT[p];
      score_mg_ += val<MG>(p, sq);
      score_eg_ += val<EG>(p, sq);
    }
    constexpr void pop(int p, int sq) {
      assert(p >= BP && p <= WK);
      assert(sq >= A1 && sq <= H8);

      p_count_ -= P_CNT[p];
      score_mg_ -= val<MG>(p, sq);
      score_eg_ -= val<EG>(p, sq);
    }
    constexpr void move(int p, int fr, int to) {
      assert(p >= BP && p <= WK);
      assert(fr >= A1 && fr <= H8);
      assert(to >= A1 && to <= H8);

      score_mg_ += delta_pst_mg[p][fr][to];
      score_eg_ += delta_pst_eg[p][fr][to];
    }
    constexpr int cnt(int p) const {
      return (p_count_ & P_CNT_MASK[p]) >> SHIFT[p];
    }
    constexpr int phase() const {
      // clang-format off
      int q =   cnt(BN) + cnt(WN)
            +   cnt(BB) + cnt(WB)
            + ((cnt(BR) + cnt(WR)) << 1)
            + ((cnt(BQ) + cnt(WQ)) << 2);
      // clang-format on
      return (q > 24) ? 24 : q;
    }
    constexpr int mix() const {
      int ph = phase();
      return (mg() * ph + eg() * (24 - ph)) / 24;
    }
    constexpr int mix(int m, int e) const {
      int ph = phase();
      return ((mg() + m) * ph + (eg() + e) * (24 - ph)) / 24;
    }
    constexpr bool null_ok(int side) const {
      return (cnt(side) - cnt((PAWN << 1) | side)) > 2;
    }
    constexpr bool is_material_draw() const {
      if (cnt(WP) | cnt(BP))
        return false;

      if (cnt(WQ) + cnt(BQ) + cnt(WR) + cnt(BR) == 0) {
        if (cnt(WB) + cnt(BB) == 0) {
          if (cnt(WN) < 3 && cnt(BN) < 3)
            return true;
        } else if (cnt(WN) + cnt(BN) == 0) {
          if ((cnt(WB) + cnt(BB)) < 2)
            return true;
        } else if ((cnt(WN) < 3 && !cnt(BN)) || (cnt(WB) == 1 && !cnt(WN))) {
          if ((cnt(BN) < 3 && !cnt(WN)) || (cnt(BB) == 1 && !cnt(BN)))
            return true;
        }
      } else if (cnt(WQ) + cnt(BQ) == 0) {
        if (cnt(WR) == 1 && cnt(BR) == 1) {
          if (cnt(WB) + cnt(WN) < 2 && cnt(BB) + cnt(BN) < 2)
            return true;
        } else if (cnt(WR) == 1 && cnt(BR) == 0) {
          if (cnt(WB) + cnt(WN) == 0)
            if ((cnt(BB) + cnt(BN) == 1) || (cnt(BB) + cnt(BN) == 2))
              return true;
        } else if (cnt(WR) == 0 && cnt(BR) == 1) {
          if (cnt(BB) + cnt(BN) == 0)
            if ((cnt(WB) + cnt(WN) == 1) || (cnt(WB) + cnt(WN) == 2))
              return true;
        }
      }
      return false;
    }

   private:
    uint64_t p_count_;
    int score_mg_;
    int score_eg_;

    constexpr int mg() const {
      return score_mg_;
    }
    constexpr int eg() const {
      return score_eg_;
    }
  };
} // namespace pst

#endif