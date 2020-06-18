// Eggs.Invoke
//
// Copyright Agustin K-ballo Berge, Fusion Fenix 2017-2020
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <eggs/invoke.hpp>

#include <cassert>
#include <type_traits>
#include <utility>

#if NDEBUG
#define CHECK(...) (void)(__VA_ARGS__)
#else
#define CHECK(...) assert((__VA_ARGS__))
#endif

struct C
{
    int fun(int) noexcept
    {
        return 0;
    }
    int cfun(int) const noexcept
    {
        return 1;
    }
    int lfun(int) & noexcept
    {
        return 2;
    }
    int rfun(int) && noexcept
    {
        return 3;
    }
    int clfun(int) const& noexcept
    {
        return 4;
    }
    int crfun(int) const&& noexcept
    {
        return 5;
    }
};

struct D : C {};

template <typename T, bool IsNothrow = true>
struct smart_ptr
{
    T* ptr;
    T& operator*() const noexcept(IsNothrow);
};
template <typename T>
using smart_ptr_throws = smart_ptr<T, false>;

template <typename T, bool IsNothrow = true>
struct conv_to
{
    operator T() const noexcept(IsNothrow);
};
template <typename T>
using conv_to_throws = conv_to<T, false>;

template <typename T, bool IsNothrow = true>
struct conv_from
{
    conv_from(T) noexcept(IsNothrow) {}
};
template <typename T>
using conv_from_throws = conv_from<T, false>;

std::true_type const nothrows{};
std::false_type const throws{};

template <typename R, typename F, typename... Args, bool IsNothrow,
    bool IsNothrowR = IsNothrow>
void check_invocable(std::integral_constant<bool, IsNothrow>,
    std::integral_constant<bool, IsNothrowR> = {})
{
    CHECK(std::is_base_of<std::true_type,
        eggs::is_invocable<F, Args...>>::value);
    CHECK(std::is_base_of<std::true_type,
        eggs::is_invocable_r<R, F, Args...>>::value);
    CHECK(std::is_base_of<std::true_type,
        eggs::is_invocable_r<void, F, Args...>>::value);
    CHECK(std::is_base_of<std::true_type,
        eggs::is_invocable_r<void const, F, Args...>>::value);

    CHECK(std::is_base_of<std::bool_constant<IsNothrow>,
        eggs::is_nothrow_invocable<F, Args...>>::value);
    CHECK(std::is_base_of<std::bool_constant<IsNothrowR>,
        eggs::is_nothrow_invocable_r<R, F, Args...>>::value);
    CHECK(std::is_base_of<std::bool_constant<IsNothrow>,
        eggs::is_nothrow_invocable_r<void, F, Args...>>::value);
    CHECK(std::is_base_of<std::bool_constant<IsNothrow>,
        eggs::is_nothrow_invocable_r<void const, F, Args...>>::value);
}

template <typename F, typename... Args>
void check_not_invocable()
{
    CHECK(std::is_base_of<std::false_type,
        eggs::is_invocable<F, Args...>>::value);
    CHECK(std::is_base_of<std::false_type,
        eggs::is_invocable_r<void, F, Args...>>::value);
    CHECK(std::is_base_of<std::false_type,
        eggs::is_invocable_r<void const, F, Args...>>::value);

    CHECK(std::is_base_of<std::false_type,
        eggs::is_nothrow_invocable<F, Args...>>::value);
    CHECK(std::is_base_of<std::false_type,
        eggs::is_nothrow_invocable_r<void, F, Args...>>::value);
    CHECK(std::is_base_of<std::false_type,
        eggs::is_nothrow_invocable_r<void const, F, Args...>>::value);
}

int main()
{
    /* mem-fun-ptr */ {
        using Fn = decltype(&C::fun);

        check_invocable<int, Fn, C&, int>(nothrows);
        check_not_invocable<Fn, C const&, int>();
        check_invocable<int, Fn, C&&, int>(nothrows);
        check_not_invocable<Fn, C const&&, int>();

        check_invocable<int, Fn, D&, int>(nothrows);
        check_not_invocable<Fn, D const&, int>();
        check_invocable<int, Fn, D&&, int>(nothrows);
        check_not_invocable<Fn, D const&&, int>();

        check_invocable<int, Fn, std::reference_wrapper<C>, int>(nothrows);
        check_not_invocable<Fn, std::reference_wrapper<C const>, int>();

        check_invocable<int, Fn, C*, int>(nothrows);
        check_not_invocable<Fn, C const*, int>();

        check_invocable<int, Fn, smart_ptr<C>, int>(nothrows);
        check_not_invocable<Fn, smart_ptr<C const>, int>();

        check_invocable<int, Fn, smart_ptr_throws<C>, int>(throws);
        check_not_invocable<Fn, smart_ptr_throws<C const>, int>();

        check_not_invocable<Fn>();
        check_not_invocable<Fn, int&>();
        check_not_invocable<Fn, C&, C>();

        using Fnc = decltype(&C::cfun);

        check_invocable<int, Fnc, C&, int>(nothrows);
        check_invocable<int, Fnc, C const&, int>(nothrows);
        check_invocable<int, Fnc, C&&, int>(nothrows);
        check_invocable<int, Fnc, C const&&, int>(nothrows);

        using Fnl = decltype(&C::lfun);

        check_invocable<int, Fnl, C&, int>(nothrows);
        check_not_invocable<Fnl, C const&, int>();
        check_not_invocable<Fnl, C&&, int>();
        check_not_invocable<Fnl, C const&&, int>();

        using Fnr = decltype(&C::rfun);

        check_not_invocable<Fnr, C&, int>();
        check_not_invocable<Fnr, C const&, int>();
        check_invocable<int, Fnr, C&&, int>(nothrows);
        check_not_invocable<Fnr, C const&&, int>();

        using Fncl = decltype(&C::clfun);

        check_invocable<int, Fncl, C&, int>(nothrows);
        check_invocable<int, Fncl, C const&, int>(nothrows);
#if __cplusplus > 201703L || defined(_MSC_VER)  // C++20: P0704
        check_invocable<int, Fncl, C&&, int>(nothrows);
        check_invocable<int, Fncl, C const&&, int>(nothrows);
#else
        check_not_invocable<Fncl, C&&, int>();
        check_not_invocable<Fncl, C const&&, int>();
#endif

        using Fncr = decltype(&C::crfun);

        check_not_invocable<Fncr, C&, int>();
        check_not_invocable<Fncr, C const&, int>();
        check_invocable<int, Fncr, C&&, int>(nothrows);
        check_invocable<int, Fncr, C const&&, int>(nothrows);
    }

    /* mem-obj-ptr */ {
        using Fn = int C::*;

        check_invocable<int&, Fn, C&>(nothrows);
        check_invocable<int const&, Fn, C const&>(nothrows);
        check_invocable<int&&, Fn, C&&>(nothrows);
        check_invocable<int const&&, Fn, C const&&>(nothrows);

        check_invocable<int&, Fn, D&>(nothrows);
        check_invocable<int const&, Fn, D const&>(nothrows);
        check_invocable<int&&, Fn, D&&>(nothrows);
        check_invocable<int const&&, Fn, D const&&>(nothrows);

        check_invocable<int&, Fn, std::reference_wrapper<C>>(nothrows);
        check_invocable<int const&, Fn, std::reference_wrapper<C const>>(nothrows);

        check_invocable<int&, Fn, C*>(nothrows);
        check_invocable<int const&, Fn, C const*>(nothrows);

        check_invocable<int&, Fn, smart_ptr<C>>(nothrows);
        check_invocable<int const&, Fn, smart_ptr<C const>>(nothrows);

        check_invocable<int&, Fn, smart_ptr_throws<C>>(throws);
        check_invocable<int const&, Fn, smart_ptr_throws<C const>>(throws);

        check_not_invocable<Fn>();
        check_not_invocable<Fn, int&>();
        check_not_invocable<Fn, C&, int>();
    }

    /* fun-obj */ {
        struct Fn
        {
            int operator()(int) noexcept
            {
                return 42;
            }
        };

        check_invocable<int, Fn, int>(nothrows);
        check_invocable<int, Fn, conv_to<int>>(nothrows);
        check_invocable<int, Fn, conv_to_throws<int>>(throws);
        check_invocable<conv_from<int>, Fn, int>(nothrows);
        check_invocable<conv_from_throws<int>, Fn, int>(nothrows, throws);
        check_invocable<conv_from_throws<int>, Fn, conv_to_throws<int>>(throws, throws);

        check_not_invocable<Fn>();
        check_not_invocable<Fn, void*>();
        check_not_invocable<Fn, int, int>();

        struct S
        {
            static int f(int) noexcept
            {
                return 0;
            }
        };
        using Fn_ptr = decltype(&S::f);

        check_invocable<int, Fn_ptr, int>(nothrows);
    }
}
