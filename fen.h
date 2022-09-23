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

#ifndef FEN_H
#define FEN_H

#include <string>

namespace fen {

  // clang-format off
  static const std::string fen_chars = " /pPnNbBrRqQkK w-0123456789";
  static const std::string file      = "abcdefghABCDEFGH";
  static const std::string rank      = "12345678";
  static const std::string side      = "bwBW";
  static const std::string new_game  = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

  // fen-coordinate to int
  static constexpr int stoi(const std::string& s) {
    if (s.size() != 2) return 0x0;
    return (rank.find(s[1]) << 3) + (file.find(s[0]) & 7);
  }
  // fen index to internal (upside-down flip)
  static constexpr int index(const int i) {
    return i ^ 0b111000;
  }
  // fen-piece to internal
  static constexpr int piece(const char c) {
    return fen_chars.find(c);
  }
  // fen-color to internal
  static constexpr bool stoc(const std::string& s) {
    return side.find(s) & 1;
  }
  // clang-format on
} // namespace fen

#endif