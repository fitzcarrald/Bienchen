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

#include "gen.h"

namespace gen {

  void G::find_proms(const core::Pos& pos) {
    bool s = pos.side();
    u64 p  = pos.pawn(s);

    p &= s ? RANK_MASK[6] : RANK_MASK[1];

    for (auto sq : p)
      proms_[sq] = moves_[sq];
  }
  void G::find_checks(const core::Pos& pos) {
    bool sd = pos.side();
    int sq  = pos.king_sq(!sd);

    // pot
    auto bsop_ckecks =
      db::magic_bsop(sq, (pos.occ() & db::sig_bsop(sq)) * B_MAGIC[sq] >> 55)
      & ~pos.own();
    auto rook_ckecks =
      db::magic_rook(sq, (pos.occ() & db::sig_rook(sq)) * R_MAGIC[sq] >> 52)
      & ~pos.own();
    auto kngt_ckecks = N_MASK[sq] & ~pos.own();

    if (sd) {
      if (pos.cr(W_OOO) && (rook_ckecks & bit(D1)))
        checks_[E1] |= bit(C1);
      if (pos.cr(W_OO) && (rook_ckecks & bit(F1)))
        checks_[E1] |= bit(G1);
    } else {
      if (pos.cr(B_OOO) && (rook_ckecks & bit(D8)))
        checks_[E8] |= bit(C8);
      if (pos.cr(B_OO) && (rook_ckecks & bit(F8)))
        checks_[E8] |= bit(G8);
    }

    for (auto fr : pos.kngt(sd))
      checks_[fr] = moves_[fr] & kngt_ckecks;
    for (auto fr : pos.bsop(sd))
      checks_[fr] = moves_[fr] & bsop_ckecks;
    for (auto fr : pos.rook(sd))
      checks_[fr] = moves_[fr] & rook_ckecks;
    for (auto fr : pos.qeen(sd))
      checks_[fr] = moves_[fr] & (rook_ckecks | bsop_ckecks);
    for (auto fr : pos.pawn(sd))
      checks_[fr] = (sd && fr < 48) || (!sd && fr > 15)
                    ? moves_[fr] & db::att_by_pawn(sq, sd)
                    : moves_[fr] & (rook_ckecks | bsop_ckecks | kngt_ckecks);

    // abzug
    for (auto fr : u64(B_MASK[sq] & (pos.bsop(sd) | pos.qeen(sd)))) {
      u64 my = db::between(fr, sq) & pos.own();
      u64 op = db::between(fr, sq) & pos.opp();

      if (my && !op && my.count() == 1) {
        checks_[my.bti()] |= moves_[my.bti()] & ~db::between(fr, sq);
        // abzug_[my.bti()] |= moves_[my.bti()] & ~db::between(fr, sq);
      }
    }
    for (auto fr : u64(R_MASK[sq] & (pos.rook(sd) | pos.qeen(sd)))) {
      u64 my = db::between(fr, sq) & pos.own();
      u64 op = db::between(fr, sq) & pos.opp();

      if (my && !op && my.count() == 1) {
        checks_[my.bti()] |= moves_[my.bti()] & ~db::between(fr, sq);
        // abzug_[my.bti()] |= moves_[my.bti()] & ~db::between(fr, sq);
      }
    }
  }

  void G::add_move(Move m, MoveList& list, core::Pos& pos) {
    assert(pos.piece(m.fr()));
    assert(m.fr() >= 0 && m.fr() <= 63);
    assert(m.to() >= 0 && m.to() <= 63);

    assert(!pos.is_tactical(m));
    assert(m.pp() == 0);

    if (m.frto() == h_.tt_move())
      m.set(32000);
    else if (m.frto() == h_.killer(0))
      m.set(30000 + 3);
    else if (m.frto() == h_.killer(1))
      m.set(30000 + 2);
    else
      m.set(h_.history(m.frto(), pos.piece(m.fr())));

    list.push_back(m);
  }
  void G::add_tac(Move m, MoveList& list, core::Pos& pos) {
    assert(pos.piece(m.fr()));
    assert(pos.is_tactical(m) || checked(m));
    assert(m.fr() >= A1 && m.fr() <= H8);
    assert(m.to() >= A1 && m.to() <= H8);

    int pc = pos.pc(m.fr());
    int cp = pc == PAWN && m.to() == pos.ep() ? PAWN : pos.pc(m.to());
    int pp = m.pp();

    if (m.frto() == h_.tt_move())
      m.set(32000);
    else if (cp || pp)
      m.set(score(pc, cp, pp));
    else if (m.sc() < 30000 + 1 && checked(m))
      m.set(30000 + 1); // check

    list.push_back(m);
  }

  void G::gen(int fr, u64 dest, MoveList& list, core::Pos& pos) {
    int p = pos.piece(fr);

    for (auto to : u64(pos.moves(fr) & dest)) {
      int cp = pos.piece(to);

      bool ep = ((p >> 1) == PAWN && to == pos.ep()); // ? true : false;

      if ((p >> 1) == PAWN && (bit(to) & PROM))
        for (auto pp : { QUEEN, ROOK, BISHOP, KNIGHT })
          add_tac(Move(fr, to, pp), list, pos);
      else if (cp || ep)
        add_tac(Move(fr, to), list, pos);
      else
        add_move(Move(fr, to), list, pos);
    }
  }
  void G::gen(int fr, MoveList& list, core::Pos& pos) {
    for (auto to : u64(K_MASK[fr] & ~pos.own())) {
      if (pos.piece(to))
        add_tac(Move(fr, to), list, pos);
      else
        add_move(Move(fr, to), list, pos);
    }
  }
  void G::gen_esc(MoveList& list, core::Pos& pos) {
    assert(pos.check());

    auto side = pos.side();
    auto k    = pos.king(side);
    auto sq   = pos.king_sq(side);
    auto atck = pos.atck_to(sq, !side);

    gen(sq, list, pos);

    if (atck.count() > 1) // king move (cause double check)
      return;

    auto ep   = pos.ep();
    auto fr   = atck.bti();
    auto dest = db::between(fr, sq) | atck;

    if (ep) {
      if ((pos.pawn(!side) & dest) && (dest & (bit(side ? ep - 8 : ep + 8))))
        dest |= bit(ep);
    }

    for (auto fr : u64(pos.own() & ~k))
      gen(fr, dest, list, pos);
  }
  void G::gen_tactical(MoveList& list, core::Pos& pos) {
    u64 op;

    for (auto sq : pos.own())
      moves_[sq] = pos.moves(sq);

    find_checks(pos);
    find_proms(pos);

    for (auto fr : pos.own()) {
      int pc = pos.pc(fr);

      op = pc == PAWN ? u64(pos.opp() | bit(pos.ep())) : pos.opp();

      for (auto to : u64((moves_[fr] & (checks_[fr] | op)) | proms_[fr])) {
        if (pc == PAWN && (bit(to) & PROM))
          for (auto pp : { QUEEN, ROOK, BISHOP, KNIGHT })
            add_tac(Move(fr, to, pp), list, pos);
        else
          add_tac(Move(fr, to), list, pos);
      }
    }
  }
  void G::gen_cp_prom(MoveList& list, core::Pos& pos) {
    u64 op;

    for (auto sq : pos.own())
      moves_[sq] = pos.moves(sq);

    find_checks(pos);
    find_proms(pos);

    for (auto fr : pos.own()) {
      int pc = pos.pc(fr);

      op = pc == PAWN ? u64(pos.opp() | bit(pos.ep())) : pos.opp();

      for (auto to : u64((moves_[fr] & (checks_[fr] | op)) | proms_[fr])) {
        int cp  = pos.pc(to);
        bool ep = (pc == PAWN && to == pos.ep());
        if (!(cp || ep) && !(bit(to) & proms_[fr]))
          continue;

        if (pc == PAWN && (bit(to) & PROM))
          for (auto pp : { QUEEN, ROOK, BISHOP, KNIGHT })
            add_tac(Move(fr, to, pp), list, pos);
        else
          add_tac(Move(fr, to), list, pos);
      }
    }
  }
  void G::gen_non_tactical(MoveList& list, core::Pos& pos) {
    u64 op;

    for (auto fr : pos.own()) {
      op = pos.pc(fr) == PAWN ? u64(pos.opp() | bit(pos.ep())) : pos.opp();

      for (auto to : u64((moves_[fr] & ~(checks_[fr] | op)) & ~proms_[fr])) {
        assert(!pos.piece(to));
        add_move(Move(fr, to), list, pos);
      }
    }
  }
} // namespace gen