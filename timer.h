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

#ifndef TIMER_H
#define TIMER_H

#include <chrono>

class Timer {

  using time_t = std::chrono::time_point<std::chrono::steady_clock>;
  using msec_t = std::chrono::duration<int, std::ratio<1, 1000>>;

 public:
  Timer() : start_(now()) {
  }
  void reset();
  int stop();

 private:
  time_t start_;
  time_t now();
};

#endif