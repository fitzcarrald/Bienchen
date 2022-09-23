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

#include "core.h"
#include "move.h"

#include <iostream>
#include <sstream>

namespace core {

  /// Stack /////////////////////////////

  bool Stack::is_repetition() const {

    if (size() < 5)
      return false;

    int begin = size() - 3;
    int end   = size() - 1 - back().r50();

    for (int i = begin; i > end && i >= 0; i -= 2) {
      if (back().key(true) == (*this)[i].key(true))
        return true;
    }
    return false;
  }

  /// Pos ///////////////////////////////

  void Pos::set(const std::string& fen) {

    undo_stack.reset();
    pst_.clear();

    sqrs_.fill(0);
    bb_.fill(0);
    cb_.fill(0);

    move_stack_.clear();
    move_stack_.reserve(1024);

    cp_stack_.clear();
    cp_stack_.reserve(1024);

    std::stringstream ss(fen);
    std::string str[] = { "", "", "", "" };

    int r50  = 0;
    int ply_ = 0;

    // read fen: position, side, castlerights, ep, r50, ply
    ss >> str[0] >> str[1] >> str[2] >> str[3] >> r50 >> ply_;

    state().set_r50(r50);

    // add 2 pseudo-half-moves for every Move
    // to sync the half-move clock and 50 moves state
    for (auto i = 1; i < ply_; i++) {
      undo_stack.rem();
      undo_stack.rem();
    }

    // position
    int sq = 0;
    for (auto& c : str[0]) {
      auto p = fen::piece(c);

      if (p < 2)
        continue;
      else if (p < 14)
        set_sq(p, fen::index(sq++));
      else if (p > 16)
        sq += p - 17;
    }

    // cr rights
    static int flag[] = { 0, 0,     0,     0,    0,    0, 0, 0, 0,
                          0, B_OOO, W_OOO, B_OO, W_OO, 0, 0, 0 };

    int cr = 0;
    for (auto c : str[2])
      cr |= flag[fen::piece(c)];

    state().set_cr(cr);
    state().set_ep(fen::stoi(str[3]));

    // side to move
    if (str[1] == "b")
      undo_stack.rem();
  }

  // followed Stockfish's pawn_eval ideas - https://stockfishchess.org/
  // my params are @most a good guess
  template <bool S>
  int Pos::ev_pawns() const {

    static constexpr int con[] = { 0, 0, 2, 3, 5, 10, 15, 25, 0 };
    bool bckw;
    int sc  = 0;
    int cph = 24 - pst_.phase();
    int hph = cph >> 1;

    int rank, rrank;
    u64 oppo, blox, stps, leve, xlev, dubl, hood, phlx, supp;

    for (auto sq : pawn(S)) {

      rank  = sq >> 3;
      rrank = S ? rank + 1 : 8 - rank;

      blox = pawn(!S) & bit(S ? sq + 8 : sq - 8);
      leve = pawn(!S) & pawn_atck<S>(sq);
      xlev = pawn(!S) & pawn_atck<S>(S ? sq + 8 : sq - 8);
      dubl = pawn(S) & db::forward_file(sq, S);
      oppo = pawn(!S) & db::forward_file(sq, S);
      stps = oppo | (pawn(!S) & db::passed(sq, S));
      hood = pawn(S) & db::isolated(sq);
      phlx = hood & RANK_MASK[rank];
      supp = hood & RANK_MASK[S ? rank - 1 : rank + 1];
      bckw = !(hood & db::forward_rank(sq, !S)) && (xlev | blox);

      if (!(hood)) {
        if (oppo && dubl && !bool(stps))
          sc -= 4 + cph;
        else
          sc -= 2 + (6 + hph) * !bool(oppo);
      } else if (phlx | supp) {
        sc += con[rrank] * (2 + bool(phlx) - bool(oppo));
        sc += 7 * supp.count();
      } else if (bckw)
        sc -= hph + (6 + hph) * !bool(oppo);
      if (!supp)
        sc -= (4 + cph) * bool(dubl) + bool(leve.count() > 1);
    }
    return sc;
  }

  int Pos::eval() {
    if (pst_.is_material_draw())
      return 0;

    u64 major_b = king(B_) | qeen(B_) | rook(B_);
    u64 major_w = king(W_) | qeen(W_) | rook(W_);

    int sc  = pst_.mix() + ev_pawns<W_>() - ev_pawns<B_>();
    int ph  = pst_.phase();
    int qtr = ph >> 2; // [0 .. 6]

    int atck_count[2] = { 1, 1 };
    int atck_wght[2]  = { 0, 0 };
    u64 hit_king;
    u64 hit[2] = { 0, 0 };

    // int kngt_wght = (pst_.cnt(WP) + pst_.cnt(BP) + ph) >> 1;
    // sc += kngt_wght * (pst_.cnt(WN) - pst_.cnt(BN));
    // sc += (12 - ph) * (bool(pst_.cnt(WB) > 1) - bool(pst_.cnt(BB) > 1));

    for (auto sq : kngt(W_)) {

      if (!(db::isolated(sq) & db::forward_rank(sq, W_) & pawn(B_)))
        sc += qtr;
      for (auto t : N_MASK[sq]) {
        sc += u64(N_MASK[t] & major_b).count() * qtr;
      }
      hit_king = N_MASK[sq] & kingspace(B_);
      if (hit_king != 0) {
        hit[W_] |= hit_king;
        atck_count[W_]++;
        atck_wght[W_] += 4;
      }
    }
    for (auto sq : kngt(B_)) {

      if (!(db::isolated(sq) & db::forward_rank(sq, B_) & pawn(W_)))
        sc -= qtr;
      for (auto t : N_MASK[sq]) {
        sc -= u64(N_MASK[t] & major_w).count() * qtr;
      }
      hit_king = N_MASK[sq] & kingspace(W_);
      if (hit_king != 0) {
        hit[B_] |= hit_king;
        atck_count[B_]++;
        atck_wght[B_] += 4;
      }
    }
    for (auto sq : bsop(W_)) {

      hit_king = bsop_atck(sq) & kingspace(B_);
      if (hit_king != 0) {
        hit[W_] |= hit_king;
        atck_count[W_]++;
        atck_wght[W_] += 2;
      }
      sc -= 3 * u64(db::forward_rank(sq, W_) & B_MASK[sq] & pawn(W_)).count();
    }
    for (auto sq : bsop(B_)) {

      hit_king = bsop_atck(sq) & kingspace(W_);
      if (hit_king != 0) {
        hit[B_] |= hit_king;
        atck_count[B_]++;
        atck_wght[B_] += 2;
      }
      sc += 3 * u64(db::forward_rank(sq, B_) & B_MASK[sq] & pawn(B_)).count();
    }
    for (auto sq : rook(W_)) {

      hit_king = rook_atck(sq) & kingspace(B_);
      if (hit_king != 0) {
        hit[W_] |= hit_king;
        atck_count[W_]++;
        atck_wght[W_] += 2;
      }
      sc -= u64(db::forward_file(sq, W_) & pawn(W_)).count() * qtr;
    }
    for (auto sq : rook(B_)) {
      hit_king = rook_atck(sq) & kingspace(W_);
      if (hit_king != 0) {
        hit[B_] |= hit_king;
        atck_count[B_]++;
        atck_wght[B_] += 2;
      }
      sc -= u64(db::forward_file(sq, B_) & pawn(B_)).count() * qtr;
    }
    for (auto sq : qeen(W_)) {
      hit_king = qeen_atck(sq) & kingspace(B_);
      if (hit_king != 0) {
        hit[W_] |= hit_king;
        atck_count[W_]++;
        atck_wght[W_] += 1;
      }
    }
    for (auto sq : qeen(B_)) {
      hit_king = qeen_atck(sq) & kingspace(W_);
      if (hit_king != 0) {
        hit[B_] |= hit_king;
        atck_count[B_]++;
        atck_wght[B_] += 1;
      }
    }

    sc += (((hit[W_].count() * atck_wght[W_]) / atck_count[W_]
            - (hit[B_].count() * atck_wght[B_]) / atck_count[B_])
           * qtr);

    return (side() ? sc : -sc) + 14;
  }

  /// iGenerator ///

  void Pos::esc(MoveList& l) const {

    auto s    = side();
    auto f    = king_sq(s);
    auto atck = atck_to(f, !s);

    for (auto t : u64(K_MASK[f] & ~own()))
      l.emplace_back(f, t);

    if (atck.count() > 1) // king move (cause double check)
      return;

    auto fr   = atck.bti();
    auto dest = db::between(fr, f) | atck;

    if (ep() && (pawn(!s) & dest) && (dest & (bit(s ? ep() - 8 : ep() + 8))))
      dest |= bit(ep());

    for (auto f : u64(own() - king(s)))
      for (auto t : u64(moves(f) & dest))
        if ((pawns() & bit(f)) && (PROM & bit(t)))
          for (auto pp : { QUEEN, ROOK, BISHOP, KNIGHT })
            l.emplace_back(f, t, pp);
        else
          l.emplace_back(f, t);
  }
  void Pos::gen(MoveList& l) const {

    bool s = side();

    for (auto f : pawn(s)) {
      for (auto t : s ? u64((wpan_moves(f) | wpan_capts(f)) & ~PROM)
                      : u64((bpan_moves(f) | bpan_capts(f)) & ~PROM))
        l.emplace_back(f, t);

      for (auto t : s ? u64((wpan_moves(f) | wpan_capts(f)) & PROM)
                      : u64((bpan_moves(f) | bpan_capts(f)) & PROM))
        for (auto pp : { QUEEN, ROOK, BISHOP, KNIGHT })
          l.emplace_back(f, t, pp);
    }
    for (auto f : kngt(s))
      for (auto t : u64(N_MASK[f] & ~own()))
        l.emplace_back(f, t);

    for (auto f : u64(bsop(s) | qeen(s)))
      for (auto t : bsop_moves(f))
        l.emplace_back(f, t);

    for (auto f : u64(rook(s) | qeen(s)))
      for (auto t : rook_moves(f))
        l.emplace_back(f, t);

    for (auto t : s ? wkng_moves(king_sq(s)) : bkng_moves(king_sq(s)))
      l.emplace_back(king_sq(s), t);
  }
  void Pos::all(MoveList& l) const {
    if (check())
      esc(l);
    else
      gen(l);
  }

  /// print Pos ///
  std::ostream& operator<<(std::ostream& os, Pos& b) {

    static const char symb[] = " #pPnNbBrRqQkK";
    static const char head[] =
      " State                               White    "
      "                        "
      "   Black                               Pawn   "
      "                        "
      "     Bishop                              Rook";
    static const char line[] =
      " "
      "+---+---+---+---+---+---+---+---+   "
      "+---+---+---+---+---+---+---+---+   "
      "+---+---+---+---+---+---+---+---+   "
      "+---+---+---+---+---+---+---+---+   "
      "+---+---+---+---+---+---+---+---+   "
      "+---+---+---+---+---+---+---+---+";

    os << std::endl << head << std::endl << line << std::endl;

    for (auto row = 0; row < 8; row++) {
      // for (auto dia = 0; dia < 6; dia++) {
      for (auto col = 0; col < 8; col++) {
        int i   = 8 * row + (col % 8);
        auto fi = fen::index(i);
        auto p  = b.piece(fi);

        if ((i + 1) & 7)
          os << " | " << symb[p];
        else
          os << " | " << symb[p] << " |";
      }
      os << "  ";
      for (auto col = 0; col < 8; col++) {
        int i   = 8 * row + (col % 8);
        auto fi = fen::index(i);
        auto x  = b.own() & bit(fi) ? 1 : 0;
        // auto p = b.piece(fi);

        if ((i + 1) & 7)
          os << " | " << symb[x];
        else
          os << " | " << symb[x] << " |";
      }
      os << "  ";
      for (auto col = 0; col < 8; col++) {
        int i   = 8 * row + (col % 8);
        auto fi = fen::index(i);
        auto x  = b.opp() & bit(fi) ? 1 : 0;
        // auto p = b.piece(fi);

        if ((i + 1) & 7)
          os << " | " << symb[x];
        else
          os << " | " << symb[x] << " |";
      }
      os << "  ";
      for (auto col = 0; col < 8; col++) {
        int i   = 8 * row + (col % 8);
        auto fi = fen::index(i);
        auto x  = b.pawns() & bit(fi) ? 1 : 0;
        // auto p = b.piece(fi);

        if ((i + 1) & 7)
          os << " | " << symb[x];
        else
          os << " | " << symb[x] << " |";
      }
      os << "  ";
      for (auto col = 0; col < 8; col++) {
        int i   = 8 * row + (col % 8);
        auto fi = fen::index(i);
        auto x  = b.bsops() & bit(fi) ? 1 : 0;
        // auto p = b.piece(fi);

        if ((i + 1) & 7)
          os << " | " << symb[x];
        else
          os << " | " << symb[x] << " |";
      }
      os << "  ";
      for (auto col = 0; col < 8; col++) {
        int i   = 8 * row + (col % 8);
        auto fi = fen::index(i);
        auto x  = b.rooks() & bit(fi) ? 1 : 0;
        // auto p = b.piece(fi);

        if ((i + 1) & 7)
          os << " | " << symb[x];
        else
          os << " | " << symb[x] << " |";
      }
      //}
      os << std::endl << line << std::endl;
    }

    os << std::endl;
    os << "PLY: " << int(b.ply()) << std::endl;
    os << "R50: " << int(b.r50()) << std::endl;
    os << "CR: " << int(b.cr()) << std::endl;
    os << "EP: " << int(b.ep()) << std::endl;
    return os;
  }
} // namespace core