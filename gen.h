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

#ifndef GEN_H
#define GEN_H

#include "array_rn.h"
#include "core.h"
#include "main.h"
#include "move.h"

namespace gen {

  constexpr int piece_to_index(int p, int frto) {
    return p | ((frto >> 6) & 0x3F) << 4;
  }

  // history
  class Heuristic {

   public:
    Heuristic() : size_(0) {
      clear();
    }
    void clear() {
      killer_.fill(0);
      history_.fill(0);
      table_move_.fill(0);
      size_ = 0;
    }
    void add_killer(uint16_t m) {
      if (killer(0) != m && killer(1) != m) {
        killer_[1][size_] = killer_[0][size_];
        killer_[0][size_] = m;
      }
    }
    uint16_t killer(int i) const {
      return killer_[i][size_];
    }
    void clear_gc() {
      for (int i = size_ + 2; i < MAX_DEPTH; ++i) {
        killer_[0][i] = 0;
        killer_[1][i] = 0;
      }
    }
    void add_history(uint16_t m, int p, int sc) {
      int& h = history_[piece_to_index(p, m)];
      h += sc - h * std::abs(sc) / 30'000;
    }
    int history(uint16_t m, int p) const {
      return history_[piece_to_index(p, m)];
    }
    void push() {
      size_++;
    }
    void pop() {
      size_--;
    }
    int sply() const {
      return size_;
    }
    uint16_t tt_move() const {
      return table_move_[size_];
    }
    void set_tt_move(uint16_t m) {
      table_move_[size_] = m;
    }
    void set_pv(const PV& pv) {
      table_move_.fill(0);
      int i = pv.size() - 1;
      int j = size_;
      while (i--)
        table_move_[j--] = pv[i].frto();
      assert(i == 0);
      assert(j == 0);
    }
    void process_serchinfo(uint16_t m, int p, int d, int sc, int a, int b) {

      if (sc > a) {
        add_history(m, p, d * d);
        if (sc >= b) {
          add_killer(m);
          return;
        }
      } else {
        if (killer(0) == m)
          killer_[0][size_] = 0;
        else if (killer(1) == m)
          killer_[1][size_] = 0;
      }
    }

   private:
    array_rn<int, 1024> history_;
    array_rn<uint16_t, N_CLR, MAX_DEPTH> killer_;
    array_rn<uint16_t, MAX_DEPTH> table_move_;

    int size_;

    int side() const {
      return size_ & 1;
    }
  };
  class G {

   public:
    G(Heuristic& h) : h_(h), moves_{ 0 }, checks_{ 0 }, proms_{ 0 } {
    }

    bool checked(Move m) const {
      return checks_[m.fr()] & bit(m.to());
    }
    // bool abzug(Move m) const { return abzug_[m.fr()] & bit(m.to()); }

    void all(MoveList& list, core::Pos& pos) {
      if (pos.check())
        gen_esc(list, pos);
      // pos.esc(list);
      else {
        // gen_all(list, pos);
        // pos.all(list);
        gen_tactical(list, pos);
        gen_non_tactical(list, pos);
      }
    }
    void tactical(MoveList& list, core::Pos& pos, bool do_checks) {
      if (pos.check())
        // pos.esc(list);
        gen_esc(list, pos);
      else if (!do_checks)
        // pos.tactical(list);
        gen_cp_prom(list, pos);
      else {
        // pos.tactical(list);
        // pos.checks(list);
        gen_tactical(list, pos);
      }
    }
    void tactical(MoveList& list, core::Pos& pos) {
      if (pos.check())
        // pos.esc(list);
        gen_esc(list, pos);
      else {
        // pos.tactical(list);
        // pos.checks(list);
        gen_tactical(list, pos);
      }
    }

   private:
    Heuristic& h_;
    // note: sequenz -> first moves_ then Checks & Proms

    std::array<uint64_t, 64> moves_;
    std::array<uint64_t, 64> checks_;
    std::array<uint64_t, 64> proms_;

    void find_proms(const core::Pos& pos);
    void find_checks(const core::Pos& pos);

    void add_move(Move m, MoveList& list, core::Pos& pos);
    void add_tac(Move m, MoveList& list, core::Pos& pos);

    void gen(int fr, u64 dest, MoveList& list, core::Pos& pos);
    void gen(int fr, MoveList& list, core::Pos& pos);
    void gen_esc(MoveList& list, core::Pos& pos);
    void gen_tactical(MoveList& list, core::Pos& pos);
    void gen_cp_prom(MoveList& list, core::Pos& pos);
    void gen_non_tactical(MoveList& list, core::Pos& pos);

    int score(int pc, int cp, int pp) {

      assert(pc >= PAWN && pc <= KING);
      assert(cp >= EMPTY && cp <= KING);
      assert(pp >= EMPTY && pp <= QUEEN);

      return (cp * 6) + (5 - pc) + (pp * 5) + 31000;
    }
  };
} // namespace gen

#endif