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

#ifndef CORE_H
#define CORE_H

#include "array_rn.h"
#include "db.h"
#include "fen.h"
#include "hash.h"
#include "main.h"
#include "move.h"
#include "pst.h"

namespace core {

  class State {

   public:
    State() : key_(0), ep_(0), r50_(0), cr_(0) {
    }
    void clear() {
      key_ = 0;
      ep_  = 0;
      r50_ = 0;
      cr_  = 0;
    }
    void set(int p, int sq) {
      assert(p >= BP && p <= WK);
      assert(sq >= A1 && sq <= H8);
      key_ ^= hash::key(p, sq);
    }
    void clear(int p, int sq) {
      assert(p >= BP && p <= WK);
      assert(sq >= A1 && sq <= H8);
      key_ ^= hash::key(p, sq);
    }
    void move(int p, int fr, int to) {
      assert(p >= BP && p <= WK);
      assert(fr >= A1 && fr <= H8);
      assert(to >= A1 && to <= H8);
      key_ ^= hash::move_key(p, fr, to);
    }

    uint64_t key(bool side) const {
      return key_ ^ hash::key(EMPTY, cr()) ^ hash::key(side, ep());
    }
    int ep() const {
      return ep_;
    }
    int cr() const {
      return cr_;
    }
    int r50() const {
      return r50_;
    }
    void clear_ep() {
      ep_ = 0;
    }
    void clear_cr() {
      cr_ = 0;
    }
    void clear_r50() {
      r50_ = 0;
    }
    void next_r50() {
      r50_++;
    }
    void set_r50(const int i) {
      r50_ = i;
    }
    void set_ep(const int i) {
      ep_ = i;
    }
    void set_cr(const int f) {
      cr_ = f;
    }
    void set_cr(const int fr, const int to) {
      cr_ &= CR[fr] & CR[to];
    }

   private:
    uint64_t key_;
    uint8_t ep_;
    uint8_t r50_;
    uint8_t cr_;

    // castle flag modifyer
    static int crx(int fr, int to) {
      return CR[fr] & CR[to];
    }
  };

  class Stack : public std::vector<State> {
   public:
    Stack() {
      reserve(1024);
      reset();
    }
    // copy state @do_move
    void rem() {
      push_back(back());
    }
    // trash it @undo_move
    void res() {
      pop_back();
    }
    void reset() {
      clear();
      emplace_back(State());
    }
    bool is_repetition() const;
    uint64_t key() const {
      return back().key(size() & 1);
    }
  };

  class Pos {

   public:
    Pos(const std::string& fen = fen::new_game) {
      set(fen);
    }
    void set(const std::string& fen);

    void flip();

    bool color(int sq) const {
      return piece(sq) & 0x1;
    }
    bool cr(int f) const {
      return cr() & f;
    }
    bool side() const {
      return ply() & 0x1;
    }
    bool check() const {
      return batck_to(king_sq(side()), !side());
    }
    bool in_check() const { // note: side-flip
      return batck_to(king_sq(!side()), side());
    }

    int ep() const {
      return state().ep();
    }
    int r50() const {
      return state().r50();
    }
    int cr() const {
      return state().cr();
    }
    int ply() const {
      return undo_stack.size();
    }
    int pc(int sq) const {
      return piece(sq) >> 1;
    }
    int piece(int sq) const {
      return sqrs_[sq];
    }
    int king_sq(const bool side) const {
      return king(side).bti();
    }

    u64 occ() const {
      return cb_[W_] | cb_[B_];
    }
    u64 opp() const {
      return cb_[!side()];
    }
    u64 own() const {
      return cb_[side()];
    }

    u64 kings() const {
      return bb_[KING];
    }
    u64 rooks() const {
      return bb_[ROOK] | bb_[QUEEN];
    }
    u64 bsops() const {
      return bb_[BISHOP] | bb_[QUEEN];
    }
    u64 kngts() const {
      return bb_[KNIGHT];
    }
    u64 pawns() const {
      return bb_[PAWN];
    }
    u64 pop(bool side) const {
      return cb_[side];
    }
    u64 king(bool side) const {
      return cb_[side] & bb_[KING];
    }
    u64 qeen(bool side) const {
      return cb_[side] & bb_[QUEEN];
    }
    u64 rook(bool side) const {
      return cb_[side] & bb_[ROOK];
    }
    u64 bsop(bool side) const {
      return cb_[side] & bb_[BISHOP];
    }
    u64 kngt(bool side) const {
      return cb_[side] & bb_[KNIGHT];
    }
    u64 pawn(bool side) const {
      return cb_[side] & bb_[PAWN];
    }
    u64 kingspace(bool side) const {
      return K_MASK[king_sq(side)];
    }
    uint64_t key() const {
      return undo_stack.key();
    }

    /// raw-generator ///
    u64 wpan_moves(int sq) const {
      auto all = occ() ^ bit(sq);
      return WP_MASK[sq] & ~(all | all << 8);
    }
    u64 bpan_moves(int sq) const {
      auto all = occ() ^ bit(sq);
      return BP_MASK[sq] & ~(all | all >> 8);
    }
    u64 wpan_capts(int sq) const {
      if (ep())
        return WP_ATCK[sq] & (pop(B_) | bit(ep()));
      return WP_ATCK[sq] & pop(B_);
    }
    u64 bpan_capts(int sq) const {
      if (ep())
        return BP_ATCK[sq] & (pop(W_) | bit(ep()));
      return BP_ATCK[sq] & pop(W_);
    }
    u64 kngt_moves(int sq) const {
      return N_MASK[sq] & ~own();
    }
    u64 bsop_moves(int sq) const {
      return db::magic_bsop(sq, magic_b(sq)) & ~own();
    }
    u64 rook_moves(int sq) const {
      return db::magic_rook(sq, magic_r(sq)) & ~own();
    }
    u64 qeen_moves(int sq) const {
      return rook_moves(sq) | bsop_moves(sq);
    }
    u64 wkng_moves(int sq) const {
      static constexpr u64 QR = 0x000000000000000E;
      static constexpr u64 KR = 0x0000000000000060;
      static constexpr u64 TQ = 0x0000000000000004;
      static constexpr u64 TK = 0x0000000000000040;

      u64 to = 0;
      // add OOO if legal
      if (!(QR & occ()) && cr(W_OOO) && !atck_to(sq, B_)
          && !atck_to(sq - 1, B_))
        to |= TQ;
      // add OO if legal
      if (!(KR & occ()) && cr(W_OO) && !atck_to(sq, B_) && !atck_to(sq + 1, B_))
        to |= TK;

      return (K_MASK[sq] & ~(own())) | to;
    }
    u64 bkng_moves(int sq) const {
      static constexpr u64 QR = 0x0E00000000000000;
      static constexpr u64 KR = 0x6000000000000000;
      static constexpr u64 TQ = 0x0400000000000000;
      static constexpr u64 TK = 0x4000000000000000;

      u64 to = 0;
      // add OOO if legal
      if (!(QR & occ()) && cr(B_OOO) && !atck_to(sq, W_)
          && !atck_to(sq - 1, W_))
        to |= TQ;
      // add OO if legal
      if (!(KR & occ()) && cr(B_OO) && !atck_to(sq, W_) && !atck_to(sq + 1, W_))
        to |= TK;

      return (K_MASK[sq] & ~(own())) | to;
    }
    u64 moves(int sq) const {
      uint64_t b = bit(sq);

      if (pawn(side()) & b)
        return side() ? wpan_moves(sq) | wpan_capts(sq)
                      : bpan_moves(sq) | bpan_capts(sq);

      if (qeen(side()) & b)
        return bsop_moves(sq) | rook_moves(sq);

      if (rook(side()) & b)
        return rook_moves(sq);

      if (bsop(side()) & b)
        return bsop_moves(sq);

      if (king_sq(side()) == sq)
        return side() ? wkng_moves(sq) : bkng_moves(sq);

      if (own() & b)
        return kngt_moves(sq);

      return 0;
    }
    template <bool S>
    u64 pawn_atck(int sq) const {
      return S ? WP_ATCK[sq] : BP_ATCK[sq];
    }
    u64 bsop_atck(int sq) const {
      u64 sig = (occ() ^ king(!color(sq))) & db::sig_bsop(sq);
      return db::magic_bsop(sq, sig * B_MAGIC[sq] >> 55);
    }
    u64 rook_atck(int sq) const {
      u64 sig = (occ() ^ king(!color(sq))) & db::sig_rook(sq);
      return db::magic_rook(sq, sig * R_MAGIC[sq] >> 52);
    }
    u64 qeen_atck(int sq) const {
      return rook_atck(sq) | bsop_atck(sq);
    }
    u64 atck_to(int sq, int side) const {
      u64 attacker = 0;

      attacker |= (pawn(side)) & db::att_by_pawn(sq, side);
      attacker |= (kngt(side)) & N_MASK[sq];
      attacker |= (bsop(side) | qeen(side)) & db::magic_bsop(sq, magic_b(sq));
      attacker |= (rook(side) | qeen(side)) & db::magic_rook(sq, magic_r(sq));
      attacker |= (king(side)) & K_MASK[sq];

      return attacker;
    }
    u64 all_atck_to(int sq) const {
      u64 attacker = 0;

      attacker |= pawns() & (db::att_by_pawn(sq, W_) | db::att_by_pawn(sq, B_));
      attacker |= kngts() & N_MASK[sq];
      attacker |= bsops() & db::magic_bsop(sq, magic_b(sq));
      attacker |= rooks() & db::magic_rook(sq, magic_r(sq));
      attacker |= kings() & K_MASK[sq];

      return attacker;
    }
    int iatck_to(int sq) const {

      auto atcks = all_atck_to(sq);

      auto mino_cnt = [&](bool s) { return u64(atcks & (kngt(s) | bsop(s))).count(); };
      auto pawn_cnt = [&](bool s) { return u64(atcks & pawn(s)).count(); };
      auto rook_cnt = [&](bool s) { return u64(atcks & rook(s)).count(); };
      auto qeen_cnt = [&](bool s) { return u64(atcks & qeen(s)).count(); };
      auto king_cnt = [&](bool s) { return u64(atcks & king(s)).count(); };

      int r = 135 * (pawn_cnt(W_) - pawn_cnt(B_))
            +  45 * (mino_cnt(W_) - mino_cnt(B_))
            +  27 * (rook_cnt(W_) - rook_cnt(B_))
            +  15 * (qeen_cnt(W_) - qeen_cnt(B_))
            +   1 * (king_cnt(W_) - king_cnt(B_));

      return r  ? (r > 0 ? 1 : -1)
                : 0;
    }
    bool batck_to(int sq, int side) const {
      if ((pawn(side)) & db::att_by_pawn(sq, side))
        return true;
      if ((kngt(side)) & N_MASK[sq])
        return true;
      if ((bsop(side) | qeen(side)) & db::magic_bsop(sq, magic_b(sq)))
        return true;
      if ((rook(side) | qeen(side)) & db::magic_rook(sq, magic_r(sq)))
        return true;
      if ((king(side)) & K_MASK[sq])
        return true;

      return false;
    }

    /// do/undo move ///

    bool do_move(Move m) {
      const int fr = m.fr();
      const int to = m.to();
      const int ep = state().ep();
      const int cp = piece(to);
      const int p  = piece(fr);

      assert(p >= BP && p <= WK);
      assert(fr >= A1 && fr <= H8);
      assert(to >= A1 && to <= H8);

      undo_stack.rem();

      move_stack_.push_back(m);
      cp_stack_.push_back(cp);

      if (cp) {
        clear_sq(to);
      }

      move(p, fr, to);

      state().clear_ep();

      if (p & 1) {

        if (p == WK) {
          if (m.is(388)) {
            assert(m.is_00());
            move(WR, H1, F1);
          } else if (m.is(132)) {
            assert(m.is_00());
            move(WR, A1, D1);
          }
        } else if (p == WP) {
          if (to - fr == 16)
            state().set_ep(fr + 8);
          else if (to == ep)
            clear_sq(ep - 8);
          else if (to > 55) {
            assert(m.pp() >= KNIGHT && m.pp() <= QUEEN);
            clear_sq(to);
            set_sq((m.pp() << 1) + 1, to);
          }
          state().clear_r50();
        } else if (cp == EMPTY)
          state().next_r50();
        else
          state().clear_r50();
      } else {
        if (p == BK) {
          if (m.is(4028)) {
            assert(m.is_00());
            move(BR, H8, F8);
          } else if (m.is(3772)) {
            assert(m.is_00());
            move(BR, A8, D8);
          }
        } else if (p == BP) {
          if (fr - to == 16)
            state().set_ep(to + 8);
          else if (ep && to == ep)
            clear_sq(ep + 8);
          else if (to < 8) {
            assert(m.pp() >= KNIGHT && m.pp() <= QUEEN);
            clear_sq(to);
            set_sq(m.pp() << 1, to);
          }
          state().clear_r50();
        } else if (cp == EMPTY)
          state().next_r50();
        else
          state().clear_r50();
      }

      if (in_check()) {
        undo_move();
        return false;
      }

      state().set_cr(fr, to);
      return true;
    }
    void undo_move() {
      if (move_stack_.empty())
        return;

      Move m = move_stack_.back();
      move_stack_.pop_back();
      int cp = cp_stack_.back();
      cp_stack_.pop_back();
      undo_stack.res();

      int to = m.to();
      int fr = m.fr();
      int p  = sqrs_[to];
      int ep = state().ep();

      rmove(p, to, fr);

      if (cp) {
        rset_sq(cp, to);
      }

      if (side()) {
        if (p == WK) {
          if (m.is(388)) {
            assert(m.is_00());
            rmove(WR, F1, H1);
          } else if (m.is(132)) {
            assert(m.is_00());
            rmove(WR, D1, A1);
          }
        } else if (p == WP && to == ep) {
          rset_sq(BP, ep - 8);
        } else if (m.pp()) {
          rclear_sq(fr);
          rset_sq(WP, fr);
        }
      } else {
        if (p == BK) {
          if (m.is(4028)) {
            assert(m.is_00());
            rmove(BR, F8, H8);
          } else if (m.is(3772)) {
            assert(m.is_00());
            rmove(BR, D8, A8);
          }
        } else if (p == BP && to == ep) {
          rset_sq(WP, ep + 8);
        }

        else if (m.pp()) {
          rclear_sq(fr);
          rset_sq(BP, fr);
        }
      }
    }
    void do_null() {
      undo_stack.rem();
      state().clear_ep();
    }
    void undo_null() {
      undo_stack.res();
    }

    /// eval ///

    int lva(int sq) {

      u64 a = 0;

      if ((a |= (pawn(side())) & db::att_by_pawn(sq, side())))
        return a.lsbi();
      if ((a |= (kngt(side())) & N_MASK[sq]))
        return a.lsbi();

      auto b = (occ() & db::sig_bsop(sq)) * B_MAGIC[sq] >> 55;

      if ((a |= (bsop(side()) & db::magic_bsop(sq, b))))
        return a.lsbi();

      auto r = (occ() & db::sig_rook(sq)) * R_MAGIC[sq] >> 52;

      if ((a |= (rook(side()) & db::magic_rook(sq, r))))
        return a.lsbi();

      auto mbr = db::magic_rook(sq, r) | db::magic_bsop(sq, b);

      if ((a |= (qeen(side()) & mbr)))
        return a.lsbi();
      if ((a |= (king(side()) & K_MASK[sq])))
        return a.lsbi();

      return 64;
    }
    int see(Move m) {

      int fr = m.fr();
      int to = m.to();
      int r  = VAL[piece(to)];

      if (!do_move(m))
        return 0;

      if ((fr = lva(to)) != 64) {
        assert(fr >= 0 && fr < 64);
        m = (pc(fr) == PAWN && (bit(to) & PROM)) ? Move(fr, to, QUEEN)
                                                 : Move(fr, to);
        r -= std::max(see(m), 0);
      }
      undo_move();
      return r;
    }

    template <bool S>
    int ev_pawns() const;
    template <bool S>
    int ev_kngts(u64 kingspace);
    template <bool S>
    int ev_bsops(u64 kingspace);
    template <bool S>
    int ev_rooks(u64 kingspace);
    template <bool S>
    int ev_qeens(u64 kingspace);
    int eval();

    bool is_draw() const {
      if (r50() > 99 && !check())
        return true;
      return is_repetition();
    }
    bool is_repetition() const {
      return undo_stack.is_repetition();
    }
    bool null_ok() const {
      return pst_.null_ok(side());
    }
    bool is_pawn_push(Move m) const {
      return (pc(m.fr()) == PAWN) && (m.to() & PUSH);
    }
    bool recap(Move m) const {
      return m.to() == move_stack_.back().to() && cp_stack_.back();
    }
    bool is_tactical(Move m) const {
      return m.pp() || pc(m.to()) || (pc(m.fr()) == PAWN && m.to() == ep());
    }
    Move last_move() const {
      return move_stack_.back();
    }

    /// iGenerator ///
    void esc(MoveList& l) const;
    void gen(MoveList& l) const;
    void all(MoveList& l) const;

   private:
    // state stack
    Stack undo_stack;
    // move stack
    std::vector<Move> move_stack_;
    // capture stack
    std::vector<uint8_t> cp_stack_;
    // piecees @ squares
    std::array<uint8_t, 64> sqrs_;
    // bb for uncolored piece types
    std::array<u64, 7> bb_;
    // bb for color
    std::array<u64, 2> cb_;
    // piece square table
    pst::Table pst_;

    const State& state() const {
      return undo_stack.back();
    }
    State& state() {
      return undo_stack.back();
    }

    // do_move: update state-copy
    void set_sq(int p, int sq) {
      cb_[p & 1] |= bit(sq);
      bb_[p >> 1] |= bit(sq);
      sqrs_[sq] = p;
      pst_.push(p, sq);
      state().set(p, sq);
    }
    void clear_sq(int sq) {
      int p = sqrs_[sq];

      if (p) {
        cb_[p & 1] ^= bit(sq);
        bb_[p >> 1] ^= bit(sq);
        sqrs_[sq] = EMPTY;
        pst_.pop(p, sq);
        state().clear(p, sq);
      }
    }
    void move(int p, int fr, int to) {
      assert(p >= BP && p <= WK);
      assert(fr >= 0 && fr < 64);
      assert(to >= 0 && to < 64);

      bb_[p >> 1] ^= bit(fr) | bit(to);
      cb_[p & 1] ^= bit(fr) | bit(to);

      sqrs_[fr] = EMPTY;
      sqrs_[to] = p;
      pst_.move(p, fr, to);
      state().move(p, fr, to);
    }

    // undo_move: don't touch the state
    void rset_sq(int p, int sq) {
      cb_[p & 1] |= bit(sq);
      bb_[p >> 1] |= bit(sq);
      sqrs_[sq] = p;
      pst_.push(p, sq);
    }
    void rclear_sq(int sq) {
      int p = sqrs_[sq];

      if (p) {
        cb_[p & 1] ^= bit(sq);
        bb_[p >> 1] ^= bit(sq);
        sqrs_[sq] = EMPTY;
        pst_.pop(p, sq);
      }
    }
    void rmove(int p, int fr, int to) {
      assert(p >= BP && p <= WK);
      assert(fr >= 0 && fr < 64);
      assert(to >= 0 && to < 64);

      bb_[p >> 1] ^= bit(fr) | bit(to);
      cb_[p & 1] ^= bit(fr) | bit(to);

      sqrs_[fr] = EMPTY;
      sqrs_[to] = p;
      pst_.move(p, fr, to);
    }

    u64 magic_r(int sq) const {
      return (occ() & db::sig_rook(sq)) * R_MAGIC[sq] >> 52;
    }
    u64 magic_b(int sq) const {
      return (occ() & db::sig_bsop(sq)) * B_MAGIC[sq] >> 55;
    }
    u64 magic_r(int sq, u64 exclude) const {
      return (occ() & ~exclude & db::sig_rook(sq)) * R_MAGIC[sq] >> 52;
    }
    u64 magic_b(int sq, u64 exclude) const {
      return (occ() & ~exclude & db::sig_bsop(sq)) * B_MAGIC[sq] >> 55;
    }
  };

  std::ostream& operator<<(std::ostream& os, Pos& b);
} // namespace core

#endif