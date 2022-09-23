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

#include "move.h"
#include "main.h"
#include <iostream>

static const std::string wspace = " ";
static const std::string prom_piece[]{ "", "", "n", "b", "r", "q" };
static const std::string FILES = "abcdefgh";
static const std::string RANKS = "12345678";
static const std::string PIECE = " pnbrqk";
static const std::string coordinate[N_SQ]{

  "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1", "a2", "b2", "c2", "d2", "e2",
  "f2", "g2", "h2", "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3", "a4", "b4",
  "c4", "d4", "e4", "f4", "g4", "h4", "a5", "b5", "c5", "d5", "e5", "f5", "g5",
  "h5", "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6", "a7", "b7", "c7", "d7",
  "e7", "f7", "g7", "h7", "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8"
};

/// Move ////////////////

Move::Move(std::string& s) : frto_(0), score_(0) {

  int pp = 0;

  if (s.length() == 5) {
    s[4] = char(tolower(s[4]));
    pp   = PIECE.find(s.substr(4, 1));
  }

  int ff = FILES.find(s.substr(0, 1));
  int fr = RANKS.find(s.substr(1, 1));
  int f  = 8 * fr + ff;
  int tf = FILES.find(s.substr(2, 1));
  int tr = RANKS.find(s.substr(3, 1));
  int t  = 8 * tr + tf;

  frto_ = f | (t << 6) | (pp << 12);
}

std::ostream& operator<<(std::ostream& os, const Move& m) {
  return os << coordinate[m.fr()] + coordinate[m.to()] + prom_piece[m.pp()]
                 + wspace;
}
std::ostream& operator<<(std::ostream& os, PV& list) {
  for (Move m : list)
    os << m;
  return os;
}