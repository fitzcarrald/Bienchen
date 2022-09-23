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

#ifndef UCI_H
#define UCI_H

#include "move.h"

namespace uci {

  bool log(const Move m);
  void search();
  void pos(std::istringstream& is);
  void go(std::istringstream& is);
  void ng();
  void uci();
  void rdy();
  void unknown(const std::string& cmd);
  void run();
} // namespace uci

#endif