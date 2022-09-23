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

#ifndef MOVE_H
#define MOVE_H

#include <algorithm> // stable_sort
#include <iostream>
#include <vector>

class Move {

 public:
  constexpr Move() = default; // {}
  constexpr Move(int ft) : frto_(ft), score_(0) {
  }
  constexpr Move(int fr, int to) : frto_(fr | to << 6), score_(0) {
  }
  constexpr Move(int fr, int to, int pp)
    : frto_(fr | (to << 6) | (pp << 12)), score_(0) {
  }

  constexpr Move(const Move& o) : frto_(o.frto_), score_(o.score_) {
  }
  Move(std::string& s);
  constexpr Move& operator=(const Move& o) {
    frto_  = o.frto_;
    score_ = o.score_;
    return *this;
  }
  Move& operator=(std::string& s) {
    *this = Move(s);
    return *this;
  }
  constexpr void set(const int sc) {
    score_ = sc;
  }
  constexpr bool operator>(const Move& o) const {
    return score_ > o.score_;
  }
  constexpr bool operator<(const Move& o) const {
    return score_ < o.score_;
  }
  constexpr bool operator==(const Move& o) const {
    return frto_ == o.frto_;
  }
  constexpr bool operator!=(const Move& o) const {
    return frto_ != o.frto_;
  }
  constexpr int fr() const {
    return frto_ & 0x3F;
  }
  constexpr int to() const {
    return (frto_ >> 6) & 0x3F;
  }
  constexpr int pp() const {
    return (frto_ >> 12) & 0x7;
  }
  constexpr int sc() const {
    return score_;
  }
  constexpr int frto() const {
    return frto_;
  }
  constexpr int index() const {
    return frto_ & 0xFFF;
  }
  constexpr bool is(const int f) const {
    return (frto_ & 0xFFF) == f;
  }
  constexpr bool is_null() const {
    return frto_ == 0xFFF;
  }
  constexpr bool is_00() const {
    return ((frto_ & 0xFFF) == 388) || ((frto_ & 0xFFF) == 132)
        || ((frto_ & 0xFFF) == 4028) || ((frto_ & 0xFFF) == 3772);
  }
  static constexpr Move none() {
    return Move(0);
  }
  static constexpr Move null() {
    return Move(0xFFF);
  }

 private:
  uint16_t frto_ = 0;
  int16_t score_ = 0;
};

class MoveList : public std::vector<Move> {

 public:
  MoveList() {
    reserve(128);
  };
  void sort() {
    std::stable_sort(begin(), end(), [](Move i, Move j) { return i > j; });
  }
};

class PV : public std::vector<Move> {

 public:
  void replace(const Move m0, PV& pv) {
    clear();
    push_back(m0);
    for (auto mi : pv)
      push_back(mi);
  }
};

std::ostream& operator<<(std::ostream& os, const Move& m);
std::ostream& operator<<(std::ostream& os, PV& list);

#endif