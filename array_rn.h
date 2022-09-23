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

#ifndef ARRAY_RN_H
#define ARRAY_RN_H

#include <array>

// multi-dim array
// array_rn<int, b, c, d> a   :=   int a[b][c][d]

template <typename T, std::size_t N, std::size_t... n>
struct array_rn : public std::array<array_rn<T, n...>, N> {

  void fill(const T& v) {
    T* p = reinterpret_cast<T*>(this);
    std::fill(p, p + sizeof(*this) / sizeof(T), v);
  }
};

template <typename T, std::size_t N>
struct array_rn<T, N> : public std::array<T, N> {};

#endif
