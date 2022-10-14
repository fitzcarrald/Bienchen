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

#include "uci.h"
#include "core.h"
#include "search.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <thread>

namespace uci {

  core::Pos position(fen::new_game);
  Search search_(position);

  bool log(const Move m) {

    std::fstream file(".log.txt", std::ios::app);
    if (file.is_open()) {
      file << m << "\n";
      file.close();
    }
    return true;
  }
  void search() {

    Move m = search_.run();
    position.do_move(m);
    std::cout << "bestmove " << m << std::endl;
  }
  void pos(std::istringstream& is) {

    Move m;
    std::string str("");
    std::string fen("");

    is >> str;

    if (str == "startpos") {
      fen = fen::new_game;
      is >> str;
    } else if (str == "fen") {
      while (is >> str && str != "moves") {
        fen += str + " ";
      }
    } else
      return;

    position.set(fen);

    while (is >> str && (m = str) != Move::none()) {
      if (!position.do_move(m))
        log(m);
    }
  }
  void go(std::istringstream& is) {

    int movetime  = 0;
    int depth     = 0;
    int movestogo = 0;
    int wtime     = 0;
    int btime     = 0;
    int winc      = 0;
    int binc      = 0;
    bool inf      = false;

    std::string str;

    while (is >> str)
      if (str == "infinite")
        inf = true;
      else if (str == "wtime")
        is >> wtime;
      else if (str == "btime")
        is >> btime;
      else if (str == "winc")
        is >> winc;
      else if (str == "binc")
        is >> binc;
      else if (str == "movestogo")
        is >> movestogo;
      else if (str == "depth")
        is >> depth;
      else if (str == "movetime")
        is >> movetime;

    int wdiv = winc ? 30 : 40;
    int bdiv = binc ? 30 : 40;

    if (inf)
      search_.set_time(0);
    else {
      if (position.side()) {
        search_.set_time(wtime / wdiv);
        search_.set_inc(winc);
      } else {
        search_.set_time(btime / bdiv);
        search_.set_inc(binc);
      }
    }
    if (movetime) {
      search_.set_time(movetime);
    }
    if (depth)
      search_.set_depth(depth);
    else
      search_.set_depth(128);

    search_.set_stop(false);
  }
  void ng() {
    position.set(fen::new_game);
    search_.clear_ht();
  }
  void uci() {
    std::cout << "id name Bienchen" << std::endl
              << "id author Manuel Schenske" << std::endl
              << "uciok" << std::endl;
  }
  void rdy() {
    std::cout << "readyok" << std::endl;
  }
  void unknown(const std::string& cmd) {
    std::cout << "Unknown command: " << cmd << std::endl;
  }
  void run() {

    position.set(fen::new_game);
    std::string str("");
    std::string cmd("");
    std::vector<std::thread> v;

    do {
      if (!getline(std::cin, cmd))
        cmd = "quit";

      std::istringstream is(cmd);

      str.clear();
      is >> std::skipws >> str;

      if (str == "stop") {
        if (!search_.stopped())
          search_.set_stop(true);
      } else if (str == "quit") {
        if (!search_.stopped())
          search_.set_stop(true);
        break;
      } else if (str == "uci")
        uci();
      else if (str == "ucinewgame")
        ng();
      else if (str == "isready")
        rdy();
      else if (str == "go") {
        go(is);
        v.emplace_back(std::thread(search));
      } else if (str == "position")
        pos(is);
      else if (str == "perft") {
        v.emplace_back(std::thread(perft::run));
      } else if (str == "print")
        std::cout << position << std::endl;
      else
        unknown(cmd);
    } while (str != "quit");

    while (!v.empty()) {
      v.back().join();
      v.pop_back();
    }
  }
} // namespace uci