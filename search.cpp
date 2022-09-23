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

#include "search.h"

void Search::time_check() {

  if (node_counter_++ & 0xFFFF)
    return;

  if (time_) {
    if (timer_.stop() > time_)
      stop_.store(true);
  }
}
void Search::info(int score, int depth, PV& pv) {
  // clang-format off
  int t = timer_.stop();
  if (std::abs(score) >= MATT_IN_MAX) {

    score = (score > 0 ? MATT - score + 1 : -MATT + score) / 2;

    std::cout << "info depth "  << depth
              << " seldepth "   << sel_depth_
              << " nodes "      << node_counter_
              << " score mate " << score
              << " nps "        << (node_counter_ * 1000) / (t + 1)
              << " time "       << t;
  } else {
    std::cout << "info depth "  << depth
              << " seldepth "   << sel_depth_
              << " nodes "      << node_counter_
              << " score cp "   << score
              << " nps "        << (node_counter_ * 1000) / (t + 1)
              << " time "       << t;
  }
  std::cout << " pv " << pv << std::endl;
  // clang-format on
}
int Search::quiesce(int depth, int alpha, int beta) {

  time_check();
  if (pos_.is_draw())
    return 0;
  if (hr_.sply() >= MAX_DEPTH)
    return pos_.eval();

  bool is_check = pos_.check();
  int ply       = hr_.sply();
  int score     = is_check ? -MATT + ply : pos_.eval();

  if (score >= beta)
    return beta;
  if (score > alpha)
    alpha = score;

  move_lists_[ply].clear();
  gen::G gen(hr_);
  gen.tactical(move_lists_[ply], pos_, !depth);
  move_lists_[ply].sort();

  score    = SCORE_NONE;
  u64 done = 0;

  for (Move m : move_lists_[ply]) {

    if (!is_check && !m.pp()
        && VALUE[pos_.piece(m.fr())] > VALUE[pos_.piece(m.to())]
        && pos_.see(m) < 0)
      continue;
    if (!is_check && !gen.checked(m) && (bit(m.to()) & done))
      continue;
    if (!pos_.do_move(m))
      continue;
    if (!is_check && !gen.checked(m))
      done |= bit(m.to());

    hr_.push();
    sel_depth_ = std::max(ply, sel_depth_);

    score = -quiesce(depth - 1, -beta, -alpha);

    hr_.pop();
    pos_.undo_move();

    if (stop_)
      return 0;

    if (score > alpha) {
      alpha = score;
      if (alpha >= beta)
        break;
    }
  }
  if (is_check && score == SCORE_NONE)
    alpha = -MATT + ply;
  return alpha;
}
int Search::alphaBeta(int depth, int alpha, int beta, PV& pv, bool opt) {

  int ply      = hr_.sply();
  bool is_pv   = beta - alpha != 1;
  bool is_root = is_pv && ply == 0;

  if (pos_.r50() >= 3 && alpha < 0 && !is_root && pos_.is_repetition()) {
    alpha = draw_value();
    if (alpha >= beta)
      return alpha;
  }

  if (depth <= 0)
    return quiesce(0, alpha, beta);

  time_check();

  uint64_t key   = pos_.key();
  bool is_check  = pos_.check();
  int eval       = SCORE_NONE;
  int score      = SCORE_NONE;
  int best_score = is_root ? alpha : -MATT;

  ht::Entry* entry = nullptr;

  assert(-INF <= alpha && alpha < beta && beta <= INF);
  assert(is_pv || (alpha == beta - 1));
  assert(0 < depth && depth < MAX_DEPTH);

  if (!is_root) {
    if (pos_.is_draw() || ply >= MAX_DEPTH - 1)
      return (ply >= MAX_DEPTH - 1 && !is_check) ? pos_.eval() : draw_value();

    alpha = std::max(-MATT + ply, alpha);
    beta  = std::min(MATT - ply - 1, beta);

    if (alpha >= beta) {
      assert(alpha > -INF && alpha < INF);
      return alpha;
    }
  } else
    goto Move_Loop;

  entry = table_.e(key, depth);

  if (!is_pv && entry) {
    score = ht::out(entry->score_, ply, pos_.r50());

    if (entry->depth_ >= depth && score != SCORE_NONE && score >= beta
          ? (entry->flag_ & ht::Entry::LB)
          : (entry->flag_ & ht::Entry::UB)) {
      if (pos_.r50() < 90)
        return score;
    }
  }
  hr_.set_tt_move(table_.move(key));

  pv.clear();

  if (is_check) {
    eval = -MATT + ply;
    goto Move_Loop;
  } else if (last_move() != Move::null())
    eval = pos_.eval();
  else
    eval = pos_.eval() + 50;

  if (!is_pv && std::abs(beta) < MATT_IN_MAX - 225) {

    // beta pruning
    if (depth <= 3) {
      int sc = eval - 75 * depth;
      if (sc >= beta) {
        return sc;
      }
    }
    // null
    if (opt && eval >= beta && pos_.null_ok()) {
      pos_.do_null();
      hr_.push();

      int r = (11 + depth) / 3 + std::min(int(eval - beta) / 150, 3);

      if (depth <= r)
        score = -quiesce(0, -beta, -alpha);
      else {
        PV npv;
        score = -alphaBeta(depth - r, -beta, -beta + 1, npv, false);
      }

      pos_.undo_null();
      hr_.pop();

      if (score >= beta) {
        if (depth < 13) {
          assert(score > -INF && score < INF);
          return score;
        }
        PV npv;
        int vs = alphaBeta(depth - r, beta - 1, beta, npv, false);
        if (vs >= beta) {
          assert(score > -INF && score < INF);
          return score;
        }
      }
    }
  }
  // former IID
  if (is_pv && depth >= 4 && std::abs(beta) < MATT_IN_MAX && hr_.tt_move() == 0)
    --depth;

Move_Loop:

  move_lists_[ply].clear();
  gen::G gen(hr_);
  gen.all(move_lists_[ply], pos_);
  move_lists_[ply].sort();

  int old_alpha  = alpha;
  Move best_move = Move::none();
  int move_cnt   = 0;

  score = SCORE_NONE;

  hr_.clear_gc();

  for (Move m : move_lists_[ply]) {
    PV npv;

    bool is_tactical = pos_.is_tactical(m);
    // int see          = is_tactical ? pos_.see(m) : -1;
    // bool recap       = !is_root && is_recap(m) && see >= 0;
    bool recap     = !is_root && is_recap(m);
    bool pawn_push = pos_.is_pawn_push(m);

    if (!pos_.do_move(m))
      continue;

    move_line_.push_back(m);
    cp_line_.push_back(pos_.piece(m.to()));

    move_cnt++;
    hr_.push();

    //
    bool see = is_tactical ? -quiesce(0, -alpha - 1, -alpha) > alpha : false;
    recap    = recap && see;

    int ext = 0;
    int red = 0;

    bool leave_pv =
      move_cnt > 1 + 2 * is_root + 2 * bool(is_pv && std::abs(best_score) < 2);

    if (!is_root && cur_depth_ >= 6) {
      if ((is_pv && (is_check || recap || pawn_push))
          || (depth <= 4 && (is_check || recap)))
        ext = 1;

      else if (depth >= 3 && leave_pv && !is_tactical && !is_check && !m.is_00()
               && !pawn_push && !recap && m.sc() < -depth
               && std::abs(eval) < MATT_IN_MAX) {

        red = depth / 3;
      }
    }

    if ((is_pv && leave_pv) || red) {
      score = -alphaBeta(depth + ext - red - 1, -alpha - 1, -alpha, npv, opt);
      if (score > alpha)
        score = -alphaBeta(depth + ext - 1, -beta, -alpha, npv, opt);
    } else {
      score = -alphaBeta(depth + ext - 1, -beta, -alpha, npv, opt);
    }

    hr_.pop();
    pos_.undo_move();

    move_line_.pop_back();
    cp_line_.pop_back();

    if (!is_tactical) {
      hr_.process_serchinfo(
        m.frto(), pos_.piece(m.fr()), depth, score, alpha, beta);
    }

    if (stop_)
      return 0;

    if (score > best_score) {
      best_score = score;
      if (score > alpha) {
        alpha     = score;
        best_move = m;

        if (is_pv) {
          pv.replace(m, npv);
          if (is_root) {
            info(best_score, depth, pv);
          }
        }

        if (score >= beta) {
          for (auto x : move_lists_[ply]) {
            if (x != m && !pos_.is_tactical(x))
              hr_.add_history(
                x.frto(), pos_.piece(x.fr()), -((depth * depth) >> 3));
          }
          break;
        }
      }
    }
  }

  if (move_cnt == 0)
    best_score = is_check ? -MATT + ply : PATT;

  if (!is_root) {
    if (best_score >= beta)
      table_.rem(key, best_move, depth, ht::in(best_score, ply), ht::Entry::LB);
    else if (is_pv && best_score > old_alpha)
      table_.rem(key, best_move, depth, ht::in(best_score, ply), ht::Entry::XB);
    else
      table_.rem(
        key, Move::none(), depth, ht::in(best_score, ply), ht::Entry::UB);
  }
  return best_score;
}
void Search::init() {

  timer_.reset();
  hr_.clear();
  cp_line_.clear();
  move_line_.clear();
  // 2do table_.reset();
  stop_.store(false);
  node_counter_ = 1;
}
Move Search::run() {

  init();

  int base_time = pos_.ply() > 12 ? time_ : time_ / 2;
  int score     = -MATT;
  int alpha     = -MATT;
  int beta      = MATT;
  int delta     = 17;

  Move bm = Move::none();
  PV pv;

  for (cur_depth_ = 1; cur_depth_ < MAX_DEPTH; cur_depth_++) {
    if (cur_depth_ >= 6) {
      alpha = std::max(score - delta, -MATT);
      beta  = std::min(score + delta, MATT);
    }

    int depth = cur_depth_;

    // modded Stockfish's aspi window loop - https://stockfishchess.org/
    while (!stop_) {
      if (alpha < -2500)
        alpha = -MATT;
      if (beta > 2500)
        beta = MATT;

      sel_depth_ = 0;
      pv.clear();

      score = alphaBeta(depth, alpha, beta, pv, true);

      if (pv[0] != bm) {
        bm = pv[0];
        if (base_time > 30000 && pos_.ply() > 12)
          time_ += std::abs(score) < 1000
                   ? (base_time + time_inc_) / 100 * cur_depth_
                   : 0;
        hr_.set_tt_move(bm.frto()); // 2do
      }

      if (stop_) {
        if (bm != Move::none())
          return bm;
        else if (pv[0] != Move::none())
          return pv[0];
      }

      if (score <= alpha) {
        beta  = (alpha + beta) / 2;
        alpha = std::max(score - 2 * delta, -MATT);
      } else if (score >= beta) {
        beta = std::min(score + 2 * delta, MATT);
        depth--;
      } else {
        break;
      }
      delta += (delta >> 2) + 5;
    }

    if (stop_ && (bm != Move::none() || pv[0] != Move::none()))
      break;
  }
  return bm;
}

/// generator & performance test
namespace perft {

  gen::Heuristic not_used;

  const std::string PERFT[7] = {

    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
    "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -",
    "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -",
    "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
    "r2q1rk1/pP1p2pp/Q4n2/bbp1p3/Np6/1B3NBn/pPPP1PPP/R3K2R b KQ - 0 1",
    "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8",
    "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10"
  };
  constexpr size_t PERFT_RESULT[7][6] = {

    { 1, 20, 400, 8902, 197281, 4865609 },
    { 1, 48, 2039, 97862, 4085603, 193690690 },
    { 1, 14, 191, 2812, 43238, 674624 },
    { 1, 6, 264, 9467, 422333, 15833292 },
    { 1, 6, 264, 9467, 422333, 15833292 },
    { 1, 44, 1486, 62379, 2103487, 89941194 },
    { 1, 46, 2079, 89890, 3894594, 164075551 }
  };

  uint64_t perft(int depth, core::Pos& p) {

    uint64_t nodes = 0;
    if (depth == 0)
      return 1;

    MoveList ml;
    ml.clear();

    // p.all(ml);         // generate with pos internal generator
    gen::G gen(not_used); // & with
    gen.all(ml, p);       // gen::G()

    for (Move m : ml) {
      if (!p.do_move(m))
        continue;
      nodes += perft(depth - 1, p);
      p.undo_move();
    }
    return nodes;
  }

  void run() {

    bool all_right    = true;
    uint64_t moves    = 0;
    uint64_t time     = 0;
    uint64_t all_time = 0;

    for (int p = 0; p < 7; p++) {
      std::cout << std::endl;
      Timer timer;
      timer.reset();
      core::Pos position(PERFT[p]);

      std::cout << PERFT[p] << " : ..." << std::endl;

      for (int i = 0; i < 6; ++i) {
        int x   = perft(i, position);
        int d   = PERFT_RESULT[p][i] - x;
        bool ok = d == 0;

        if (!ok)
          all_right = false;
        std::cout << i << ": ";
        if (ok)
          std::cout << "ok" << std::endl;
        else
          std::cout << "error " << d << std::endl;

        moves += PERFT_RESULT[p][i];
      }

      time = timer.stop();
      all_time += time;
      std::cout << time << std::endl;
      std::cout << "kN / s: " << PERFT_RESULT[p][5] / (time + 1) << std::endl;
    }

    std::cout << "Test result: ";
    if (all_right)
      std::cout << "--- OK ---" << std::endl;
    else
      std::cout << "error" << std::endl;

    std::cout << "kN / s: " << moves / (all_time + 1) << std::endl;
    std::cout << "total nodes " << moves << " in "
              << (float(all_time) / 1000.0f) << " s" << std::endl;
  }
} // namespace perft