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

#ifndef SEARCH_H
#define SEARCH_H

#include "core.h"
#include "gen.h"
#include "timer.h"

#include <atomic>

class Search {

 public:
  Search(core::Pos& p) : pos_(p) {
  }

  void init();
  Move run();

  bool stopped() {
    return stop_;
  }
  void set_time(int msecs) {
    time_ = msecs;
  }
  void set_inc(int msecs) {
    time_inc_ = msecs;
  }
  void set_depth(int d) {
    max_depth_ = d;
  }
  void set_stop(bool s = true) {
    stop_.store(s);
  }
  void clear_ht() {
    table_.clear();
  }

 private:
  bool is_recap(Move m) {
    return move_line_.back().to() == m.to() && cp_line_.back();
  }
  Move last_move() {
    return move_line_.back();
  }
  int draw_value() {
    return 2 * (node_counter_ & 1) - 1;
  }
  void info(int score, int depth, PV& pv);
  void time_check();
  int quiesce(int depth, int alpha, int beta);
  int alphaBeta(int depth, int alpha, int beta, PV& pv, bool opt);

  core::Pos& pos_;

  std::array<MoveList, MAX_DEPTH> move_lists_;
  std::vector<Move> move_line_;
  std::vector<uint8_t> cp_line_;

  gen::Heuristic hr_;
  ht::Table table_;
  Timer timer_;
  std::atomic<bool> stop_;

  uint64_t node_counter_ = 1;
  int time_              = 1;
  int time_inc_          = 0;
  int cur_depth_         = 0;
  int sel_depth_         = 0;
  int max_depth_         = MAX_DEPTH;
};

namespace perft {

  uint64_t perft(int depth, core::Pos& p);
  void run();
} // namespace perft

#endif