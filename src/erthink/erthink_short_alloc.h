/*
 *  Copyright (c) 2019-2020 Leonid Yuriev <leo@yuriev.ru>.
 *  https://github.com/erthink/erthink
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

// Portion Copyright (c) 2015 Howard Hinnant
// https://howardhinnant.github.io/stack_alloc.html
// The MIT License (MIT)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#if !defined(__cplusplus) || __cplusplus < 201103L
#error "This source code requires C++11 at least."
#endif

#include <cassert>
#include <cstddef>
#include <new>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include "erthink_defs.h"

#ifndef NDEBUG
#include <cstring> // for memset()
#endif

namespace erthink {

class allocation_arena_exhausted : public std::bad_alloc {
public:
  allocation_arena_exhausted() = default;
  virtual const char *what() const throw() {
    return "short_alloc has exhausted allocation arena";
  }
  virtual ~allocation_arena_exhausted() throw() {}
};

template <bool ALLOW_OUTLIVE, std::size_t N_BYTES,
          std::size_t ALIGN = alignof(std::max_align_t)>
class allocation_arena {
public:
  static cxx11_constexpr_var auto allow_outlive = ALLOW_OUTLIVE;
  static cxx11_constexpr_var auto size = N_BYTES;
  static cxx11_constexpr_var auto alignment = ALIGN;

private:
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4307) /* integral constant overflow */
#pragma warning(disable : 4324) /* structure was padded due to alignment */
#endif
#ifndef NDEBUG
  static cxx11_constexpr_var std::size_t signature_A =
      std::size_t(2125913479ul) * N_BYTES +
      std::size_t(ALLOW_OUTLIVE ? 4190596621ul : 3120246733ul) +
      ALIGN * std::size_t(3366427381ul);
  static cxx11_constexpr_var std::size_t signature_B =
      std::size_t(3909341731ul) * N_BYTES +
      std::size_t(ALLOW_OUTLIVE ? 937226681ul : 2469154943ul) +
      ALIGN * std::size_t(3752260177ul);
  static cxx11_constexpr_var std::size_t signature_C =
      std::size_t(3756481181ul) * N_BYTES +
      std::size_t(ALLOW_OUTLIVE ? 2595029951ul : 1665577289ul) +
      ALIGN * std::size_t(2105020861ul);
#endif /* NDEBUG */

#ifndef NDEBUG
  std::size_t checkpoint_A_;
#endif
  alignas(alignment) char buf_[size];
#ifndef NDEBUG
  std::size_t checkpoint_B_;
#endif
  char *ptr_;
#ifndef NDEBUG
  std::size_t checkpoint_C_;
#endif
#ifdef _MSC_VER
#pragma warning(pop)
#endif

  cxx11_constexpr static std::size_t align_up(std::size_t n) cxx11_noexcept {
    return (n + (alignment - 1)) & ~(alignment - 1);
  }

  cxx11_constexpr bool
  pointer_in_buffer(const char *ptr,
                    bool accepd_end_of_buffer = false) const cxx11_noexcept {
    return buf_ <= ptr &&
           (accepd_end_of_buffer ? ptr <= buf_ + size : ptr < buf_ + size);
  }

public:
  using exhausted_exception = allocation_arena_exhausted;
  ~allocation_arena() {
    ptr_ = nullptr;
#ifndef NDEBUG
    checkpoint_A_ = 0;
    checkpoint_B_ = 0;
    checkpoint_C_ = 0;
    std::memset(buf_, 0xBB, sizeof(buf_));
#endif
  }
  NDEBUG_CONSTEXPR allocation_arena() cxx11_noexcept : ptr_(buf_) {
    static_assert(size > 1, "Oops, ALLOW_OUTLIVE is messed with N_BYTES?");
#if !ERTHINK_PROVIDE_ALIGNED_NEW
    static_assert(!allow_outlive || alignment <= alignof(std::max_align_t),
                  "you've chosen an alignment that is larger than "
                  "alignof(std::max_align_t), and cannot be guaranteed by "
                  "normal operator new");
#endif
    static_assert(size % alignment == 0,
                  "size N needs to be a multiple of alignment Align");
#ifndef NDEBUG
    checkpoint_A_ = signature_A;
    checkpoint_B_ = signature_B;
    checkpoint_C_ = signature_C;
    std::memset(buf_, 0x55, sizeof(buf_));
#endif
  }
  void NDEBUG_CONSTEXPR debug_check() const cxx11_noexcept {
    assert(checkpoint_A_ == signature_A);
    assert(checkpoint_B_ == signature_B);
    assert(checkpoint_C_ == signature_C);
    assert(ptr_ >= buf_ && ptr_ <= buf_ + sizeof(buf_));
  }

  allocation_arena(const allocation_arena &) = delete;
  allocation_arena &operator=(const allocation_arena &) = delete;

  NDEBUG_CONSTEXPR bool
  pointer_in_bounds(const void *ptr) const cxx11_noexcept {
    debug_check();
    return pointer_in_buffer(static_cast<const char *>(ptr));
  }
  cxx11_constexpr bool chunk_in_bounds(const void *ptr,
                                       std::size_t bytes) const cxx11_noexcept {
    return pointer_in_buffer(static_cast<const char *>(ptr), true) &&
           (bytes == 0 ||
            pointer_in_buffer(static_cast<const char *>(ptr) + bytes, true));
  }

  template <std::size_t ReqAlign> char *allocate(std::size_t n) {
    static_assert(ReqAlign <= alignment,
                  "alignment is too large for this arena");
    debug_check();

    if (likely(pointer_in_buffer(ptr_))) {
      auto const aligned_n = align_up(n);
      if (likely(static_cast<decltype(aligned_n)>(buf_ + size - ptr_) >=
                 aligned_n)) {
        char *r = ptr_;
#ifndef NDEBUG
        std::memset(r, 0xCC, aligned_n);
#endif
        ptr_ += aligned_n;
        debug_check();
        return r;
      }
    }

    if (allow_outlive) {
#if ERTHINK_PROVIDE_ALIGNED_NEW
      return static_cast<char *>(
          ::operator new(n, std::align_val_t(alignment)));
#else
      return static_cast<char *>(::operator new(n));
#endif
    }
    throw exhausted_exception();
  }

  void deallocate(char *p, std::size_t n) {
    debug_check();
    if (likely(pointer_in_buffer(p))) {
      auto const aligned_n = align_up(n);
#ifndef NDEBUG
      std::memset(p, 0xDD, aligned_n);
#endif
      if (p + aligned_n == ptr_)
        ptr_ = p;
      debug_check();
      return;
    }

    if (allow_outlive) {
#if ERTHINK_PROVIDE_ALIGNED_NEW
#if defined(__cpp_sized_deallocation)
      ::operator delete(p, n, std::align_val_t(alignment));
#else
      ::operator delete(p, std::align_val_t(alignment));
#endif
#elif defined(__cpp_sized_deallocation)
      ::operator delete(p, n);
#else
      ::operator delete(p);
#endif
      return;
    }
    throw std::logic_error("short_alloc was disabled to exhausted arena");
  }

  NDEBUG_CONSTEXPR std::size_t used() const cxx11_noexcept {
    debug_check();
    return static_cast<std::size_t>(ptr_ - buf_);
  }

  void reset() cxx11_noexcept {
    debug_check();
#ifndef NDEBUG
    std::memset(buf_, 0x55, sizeof(buf_));
#endif
    ptr_ = buf_;
  }
};

template <class T, typename ARENA> class short_alloc {
public:
  using value_type = T;
  using arena_type = ARENA;
  static cxx11_constexpr_var auto alignment = arena_type::alignment;
  static cxx11_constexpr_var auto size = arena_type::size;
  using is_always_equal = std::false_type;

  using propagate_on_container_copy_assignment = std::false_type;
  using propagate_on_container_move_assignment = std::false_type;
  using propagate_on_container_swap = std::false_type;

private:
  arena_type &arena_;

public:
  using arena_exhausted_exception = typename ARENA::exhausted_exception;
  cxx11_constexpr short_alloc(/*allocation arena must be provided */) = delete;
  cxx11_constexpr short_alloc(const short_alloc &) cxx11_noexcept = default;
  short_alloc &operator=(const short_alloc &) = delete;
  cxx11_constexpr short_alloc &
  select_on_container_copy_construction() const cxx11_noexcept {
    return *this;
  }

  cxx11_constexpr short_alloc(arena_type &area) cxx11_noexcept : arena_(area) {}
  template <class U>
  cxx11_constexpr
  short_alloc(const short_alloc<U, arena_type> &src) cxx11_noexcept
      : short_alloc(src.arena_) {}

  template <class U> struct rebind {
    using other = short_alloc<U, arena_type>;
  };

  T *allocate(std::size_t n) {
    return reinterpret_cast<T *>(
        arena_.template allocate<alignof(T)>(n * sizeof(T)));
  }
  void deallocate(T *p, std::size_t n) cxx11_noexcept {
    arena_.deallocate(reinterpret_cast<char *>(p), n * sizeof(T));
  }

  template <typename... Args> inline void construct(T *p, Args &&... args) {
    new (p) T(std::forward<Args>(args)...);
  }

  inline void destroy(T *p) { p->~T(); }

  template <class X, typename X_ARENA, class Y, typename Y_ARENA>
  friend inline bool
  operator==(const short_alloc<X, X_ARENA> &x,
             const short_alloc<Y, Y_ARENA> &y) cxx11_noexcept;

  template <class X, typename X_ARENA> friend class short_alloc;
};

template <class X, typename X_ARENA, class Y, typename Y_ARENA>
inline bool operator==(const short_alloc<X, X_ARENA> &x,
                       const short_alloc<Y, Y_ARENA> &y) cxx11_noexcept {
  return X_ARENA::size == Y_ARENA::size &&
         X_ARENA::alignment == Y_ARENA::alignment && &x.arena_ == &y.arena_;
}

template <class X, typename X_ARENA, class Y, typename Y_ARENA>
inline bool operator!=(const short_alloc<X, Y_ARENA> &x,
                       const short_alloc<Y, Y_ARENA> &y) cxx11_noexcept {
  return !(x == y);
}

} // namespace erthink
