// Eggs.Invoke
//
// Copyright Agustin K-ballo Berge, Fusion Fenix 2017-2020
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <eggs/invoke.hpp>

#include <type_traits>
#include <utility>

#include "test.hpp"

// Account for P0012: "Make exception specifications be part of the type system".
static constexpr bool p0012 = !std::is_same<void(), void() noexcept>::value;

struct C
{
    int fun(int) noexcept(p0012)
    {
        return 0;
    }
    int cfun(int) const noexcept(p0012)
    {
        return 1;
    }
    int lfun(int) & noexcept(p0012)
    {
        return 2;
    }
    int rfun(int) && noexcept(p0012)
    {
        return 3;
    }
    int clfun(int) const& noexcept(p0012)
    {
        return 4;
    }
    int crfun(int) const&& noexcept(p0012)
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
std::integral_constant<bool, p0012> const p0012_nothrows{};

template <typename R, typename F, typename... Args, bool IsNothrow,
    bool IsNothrowR = IsNothrow>
void test_invocable(std::integral_constant<bool, IsNothrow>,
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

    CHECK(std::is_base_of<std::integral_constant<bool, IsNothrow>,
        eggs::is_nothrow_invocable<F, Args...>>::value);
    CHECK(std::is_base_of<std::integral_constant<bool, IsNothrowR>,
        eggs::is_nothrow_invocable_r<R, F, Args...>>::value);
    CHECK(std::is_base_of<std::integral_constant<bool, IsNothrow>,
        eggs::is_nothrow_invocable_r<void, F, Args...>>::value);
    CHECK(std::is_base_of<std::integral_constant<bool, IsNothrow>,
        eggs::is_nothrow_invocable_r<void const, F, Args...>>::value);
}

template <typename F, typename... Args>
void test_not_invocable()
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

        CHECK_SCOPE(test_invocable<int, Fn, C&, int>(p0012_nothrows));
        CHECK_SCOPE(test_not_invocable<Fn, C const&, int>());
        CHECK_SCOPE(test_invocable<int, Fn, C&&, int>(p0012_nothrows));
        CHECK_SCOPE(test_not_invocable<Fn, C const&&, int>());

        CHECK_SCOPE(test_invocable<int, Fn, D&, int>(p0012_nothrows));
        CHECK_SCOPE(test_not_invocable<Fn, D const&, int>());
        CHECK_SCOPE(test_invocable<int, Fn, D&&, int>(p0012_nothrows));
        CHECK_SCOPE(test_not_invocable<Fn, D const&&, int>());

        CHECK_SCOPE(test_invocable<int, Fn, std::reference_wrapper<C>, int>(p0012_nothrows));
        CHECK_SCOPE(test_not_invocable<Fn, std::reference_wrapper<C const>, int>());

        CHECK_SCOPE(test_invocable<int, Fn, C*, int>(p0012_nothrows));
        CHECK_SCOPE(test_not_invocable<Fn, C const*, int>());

        CHECK_SCOPE(test_invocable<int, Fn, smart_ptr<C>, int>(p0012_nothrows));
        CHECK_SCOPE(test_not_invocable<Fn, smart_ptr<C const>, int>());

        CHECK_SCOPE(test_invocable<int, Fn, smart_ptr_throws<C>, int>(throws));
        CHECK_SCOPE(test_not_invocable<Fn, smart_ptr_throws<C const>, int>());

        CHECK_SCOPE(test_not_invocable<Fn>());
        CHECK_SCOPE(test_not_invocable<Fn, int&>());
        CHECK_SCOPE(test_not_invocable<Fn, C&, C>());

        using Fnc = decltype(&C::cfun);

        CHECK_SCOPE(test_invocable<int, Fnc, C&, int>(p0012_nothrows));
        CHECK_SCOPE(test_invocable<int, Fnc, C const&, int>(p0012_nothrows));
        CHECK_SCOPE(test_invocable<int, Fnc, C&&, int>(p0012_nothrows));
        CHECK_SCOPE(test_invocable<int, Fnc, C const&&, int>(p0012_nothrows));

        using Fnl = decltype(&C::lfun);

        CHECK_SCOPE(test_invocable<int, Fnl, C&, int>(p0012_nothrows));
        CHECK_SCOPE(test_not_invocable<Fnl, C const&, int>());
        CHECK_SCOPE(test_not_invocable<Fnl, C&&, int>());
        CHECK_SCOPE(test_not_invocable<Fnl, C const&&, int>());

        using Fnr = decltype(&C::rfun);

        CHECK_SCOPE(test_not_invocable<Fnr, C&, int>());
        CHECK_SCOPE(test_not_invocable<Fnr, C const&, int>());
        CHECK_SCOPE(test_invocable<int, Fnr, C&&, int>(p0012_nothrows));
        CHECK_SCOPE(test_not_invocable<Fnr, C const&&, int>());

        using Fncl = decltype(&C::clfun);

        CHECK_SCOPE(test_invocable<int, Fncl, C&, int>(p0012_nothrows));
        CHECK_SCOPE(test_invocable<int, Fncl, C const&, int>(p0012_nothrows));
        CHECK_SCOPE(test_invocable<int, Fncl, C&&, int>(p0012_nothrows));
        CHECK_SCOPE(test_invocable<int, Fncl, C const&&, int>(p0012_nothrows));

        using Fncr = decltype(&C::crfun);

        CHECK_SCOPE(test_not_invocable<Fncr, C&, int>());
        CHECK_SCOPE(test_not_invocable<Fncr, C const&, int>());
        CHECK_SCOPE(test_invocable<int, Fncr, C&&, int>(p0012_nothrows));
        CHECK_SCOPE(test_invocable<int, Fncr, C const&&, int>(p0012_nothrows));
    }

    /* mem-obj-ptr */ {
        using Fn = int C::*;

        CHECK_SCOPE(test_invocable<int&, Fn, C&>(nothrows));
        CHECK_SCOPE(test_invocable<int const&, Fn, C const&>(nothrows));
        CHECK_SCOPE(test_invocable<int&&, Fn, C&&>(nothrows));
        CHECK_SCOPE(test_invocable<int const&&, Fn, C const&&>(nothrows));

        CHECK_SCOPE(test_invocable<int&, Fn, D&>(nothrows));
        CHECK_SCOPE(test_invocable<int const&, Fn, D const&>(nothrows));
        CHECK_SCOPE(test_invocable<int&&, Fn, D&&>(nothrows));
        CHECK_SCOPE(test_invocable<int const&&, Fn, D const&&>(nothrows));

        CHECK_SCOPE(test_invocable<int&, Fn, std::reference_wrapper<C>>(nothrows));
        CHECK_SCOPE(test_invocable<int const&, Fn, std::reference_wrapper<C const>>(nothrows));

        CHECK_SCOPE(test_invocable<int&, Fn, C*>(nothrows));
        CHECK_SCOPE(test_invocable<int const&, Fn, C const*>(nothrows));

        CHECK_SCOPE(test_invocable<int&, Fn, smart_ptr<C>>(nothrows));
        CHECK_SCOPE(test_invocable<int const&, Fn, smart_ptr<C const>>(nothrows));

        CHECK_SCOPE(test_invocable<int&, Fn, smart_ptr_throws<C>>(throws));
        CHECK_SCOPE(test_invocable<int const&, Fn, smart_ptr_throws<C const>>(throws));

        CHECK_SCOPE(test_not_invocable<Fn>());
        CHECK_SCOPE(test_not_invocable<Fn, int&>());
        CHECK_SCOPE(test_not_invocable<Fn, C&, int>());
    }

    /* call-op */ {
        struct Fn
        {
            int operator()(int) noexcept(p0012);
        };

        CHECK_SCOPE(test_invocable<int, Fn&, int>(p0012_nothrows));
        CHECK_SCOPE(test_not_invocable<Fn const&, int>());
        CHECK_SCOPE(test_invocable<int, Fn&&, int>(p0012_nothrows));
        CHECK_SCOPE(test_not_invocable<Fn const&&, int>());

        CHECK_SCOPE(test_not_invocable<Fn>());
        CHECK_SCOPE(test_not_invocable<Fn, void*>());
        CHECK_SCOPE(test_not_invocable<Fn, int, int>());

        struct Fnc
        {
            int operator()(int) const noexcept(p0012);
        };

        CHECK_SCOPE(test_invocable<int, Fnc&, int>(p0012_nothrows));
        CHECK_SCOPE(test_invocable<int, Fnc const&, int>(p0012_nothrows));
        CHECK_SCOPE(test_invocable<int, Fnc&&, int>(p0012_nothrows));
        CHECK_SCOPE(test_invocable<int, Fnc const&&, int>(p0012_nothrows));

        struct Fnl
        {
            int operator()(int) & noexcept(p0012);
        };

        CHECK_SCOPE(test_invocable<int, Fnl&, int>(p0012_nothrows));
        CHECK_SCOPE(test_not_invocable<Fnl const&, int>());
        CHECK_SCOPE(test_not_invocable<Fnl&&, int>());
        CHECK_SCOPE(test_not_invocable<Fnl const&&, int>());

        struct Fnr
        {
            int operator()(int) && noexcept(p0012);
        };

        CHECK_SCOPE(test_not_invocable<Fnr&, int>());
        CHECK_SCOPE(test_not_invocable<Fnr const&, int>());
        CHECK_SCOPE(test_invocable<int, Fnr&&, int>(p0012_nothrows));
        CHECK_SCOPE(test_not_invocable<Fnr const&&, int>());

        struct Fncl
        {
            int operator()(int) const& noexcept(p0012);
        };

        CHECK_SCOPE(test_invocable<int, Fncl&, int>(p0012_nothrows));
        CHECK_SCOPE(test_invocable<int, Fncl const&, int>(p0012_nothrows));
        CHECK_SCOPE(test_invocable<int, Fncl&&, int>(p0012_nothrows));
        CHECK_SCOPE(test_invocable<int, Fncl const&&, int>(p0012_nothrows));

        struct Fncr
        {
            int operator()(int) const&& noexcept(p0012);
        };

        CHECK_SCOPE(test_not_invocable<Fncr&, int>());
        CHECK_SCOPE(test_not_invocable<Fncr const&, int>());
        CHECK_SCOPE(test_invocable<int, Fncr&&, int>(p0012_nothrows));
        CHECK_SCOPE(test_invocable<int, Fncr const&&, int>(p0012_nothrows));
    }

    /* fun-ptr */ {
        struct Fn
        {
            static int fun(int) noexcept(p0012);
        };
        using Fn_ptr = decltype(&Fn::fun);

        CHECK_SCOPE(test_invocable<int, Fn_ptr, int>(p0012_nothrows));
        CHECK_SCOPE(test_invocable<int, Fn_ptr, conv_to<int>>(p0012_nothrows));
        CHECK_SCOPE(test_invocable<int, Fn_ptr, conv_to_throws<int>>(throws));
        CHECK_SCOPE(test_invocable<conv_from<int>, Fn_ptr, int>(p0012_nothrows, p0012_nothrows));
        CHECK_SCOPE(test_invocable<conv_from_throws<int>, Fn_ptr, int>(p0012_nothrows, throws));

        CHECK_SCOPE(test_not_invocable<int, Fn_ptr>());
        CHECK_SCOPE(test_not_invocable<int, Fn_ptr, int, int>());
    }

    return test_report();
}
