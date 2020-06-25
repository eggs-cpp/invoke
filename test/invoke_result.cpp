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

template <typename T>
struct no_result_void
{
    using type = void;
};

template <typename T, typename Enable = void>
struct no_result
  : std::true_type
{};

template <typename T>
struct no_result<T, typename no_result_void<typename T::type>::type>
  : std::false_type
{};

struct C {};
struct D : C {};

template <typename T, bool IsNothrow = true>
struct smart_ptr
{
    T& operator*() const noexcept(IsNothrow);
};

template <typename T, bool IsNothrow = true>
struct conv_to
{
    operator T() const noexcept(IsNothrow);
};

int main()
{
    /* mem-fun-ptr */ {
        using Fn = int (C::*)(int);

        CHECK(std::is_same<eggs::invoke_result<Fn, C&, int>::type, int>::value);
        CHECK(no_result<eggs::invoke_result<Fn, C const&, int>>::value);
        CHECK(std::is_same<eggs::invoke_result<Fn, C&&, int>::type, int>::value);
        CHECK(no_result<eggs::invoke_result<Fn, C const&&, int>>::value);

        CHECK(std::is_same<eggs::invoke_result<Fn, D&, int>::type, int>::value);
        CHECK(no_result<eggs::invoke_result<Fn, D const&, int>>::value);
        CHECK(std::is_same<eggs::invoke_result<Fn, D&&, int>::type, int>::value);
        CHECK(no_result<eggs::invoke_result<Fn, D const&&, int>>::value);

        CHECK(std::is_same<eggs::invoke_result<Fn, std::reference_wrapper<C>, int>::type, int>::value);
        CHECK(no_result<eggs::invoke_result<Fn, std::reference_wrapper<C const>, int>>::value);

        CHECK(std::is_same<eggs::invoke_result<Fn, C*, int>::type, int>::value);
        CHECK(no_result<eggs::invoke_result<Fn, C const*, int>>::value);

        CHECK(std::is_same<eggs::invoke_result<Fn, smart_ptr<C>, int>::type, int>::value);
        CHECK(no_result<eggs::invoke_result<Fn, smart_ptr<C const>, int>>::value);

        CHECK(no_result<eggs::invoke_result<Fn>>::value);
        CHECK(no_result<eggs::invoke_result<Fn, int&>>::value);
        CHECK(no_result<eggs::invoke_result<Fn, C&, C>>::value);

        using Fnc = int (C::*)(int) const;

        CHECK(std::is_same<eggs::invoke_result<Fnc, C&, int>::type, int>::value);
        CHECK(std::is_same<eggs::invoke_result<Fnc, C const&, int>::type, int>::value);
        CHECK(std::is_same<eggs::invoke_result<Fnc, C&&, int>::type, int>::value);
        CHECK(std::is_same<eggs::invoke_result<Fnc, C const&&, int>::type, int>::value);

        using Fnl = int (C::*)(int)&;

        CHECK(std::is_same<eggs::invoke_result<Fnl, C&, int>::type, int>::value);
        CHECK(no_result<eggs::invoke_result<Fnl, C const&, int>>::value);
        CHECK(no_result<eggs::invoke_result<Fnl, C&&, int>>::value);
        CHECK(no_result<eggs::invoke_result<Fnl, C const&&, int>>::value);

        using Fnr = int (C::*)(int)&&;

        CHECK(no_result<eggs::invoke_result<Fnr, C&, int>>::value);
        CHECK(no_result<eggs::invoke_result<Fnr, C const&, int>>::value);
        CHECK(std::is_same<eggs::invoke_result<Fnr, C&&, int>::type, int>::value);
        CHECK(no_result<eggs::invoke_result<Fnr, C const&&, int>>::value);

        using Fncl = int (C::*)(int) const&;

        CHECK(std::is_same<eggs::invoke_result<Fncl, C&, int>::type, int>::value);
        CHECK(std::is_same<eggs::invoke_result<Fncl, C const&, int>::type, int>::value);
#if __cplusplus > 201703L || defined(_MSC_VER)  // C++20: P0704
        CHECK(std::is_same<eggs::invoke_result<Fncl, C&&, int>::type, int>::value);
        CHECK(std::is_same<eggs::invoke_result<Fncl, C const&&, int>::type, int>::value);
#else
        CHECK(no_result<eggs::invoke_result<Fncl, C&&, int>>::value);
        CHECK(no_result<eggs::invoke_result<Fncl, C const&&, int>>::value);
#endif

        using Fncr = int (C::*)(int) const&&;

        CHECK(no_result<eggs::invoke_result<Fncr, C&, int>>::value);
        CHECK(no_result<eggs::invoke_result<Fncr, C const&, int>>::value);
        CHECK(std::is_same<eggs::invoke_result<Fncr, C&&, int>::type, int>::value);
        CHECK(std::is_same<eggs::invoke_result<Fncr, C const&&, int>::type, int>::value);
    }

    /* mem-obj-ptr */ {
        using Fn = int C::*;

        CHECK(std::is_same<eggs::invoke_result<Fn, C&>::type, int&>::value);
        CHECK(std::is_same<eggs::invoke_result<Fn, C const&>::type, int const&>::value);
        CHECK(std::is_same<eggs::invoke_result<Fn, C&&>::type, int&&>::value);
        CHECK(std::is_same<eggs::invoke_result<Fn, C const&&>::type, int const&&>::value);

        CHECK(std::is_same<eggs::invoke_result<Fn, D&>::type, int&>::value);
        CHECK(std::is_same<eggs::invoke_result<Fn, D const&>::type, int const&>::value);
        CHECK(std::is_same<eggs::invoke_result<Fn, D&&>::type, int&&>::value);
        CHECK(std::is_same<eggs::invoke_result<Fn, D const&&>::type, int const&&>::value);

        CHECK(std::is_same<eggs::invoke_result<Fn, std::reference_wrapper<C>>::type, int&>::value);
        CHECK(std::is_same<eggs::invoke_result<Fn, std::reference_wrapper<C const>>::type, int const&>::value);

        CHECK(std::is_same<eggs::invoke_result<Fn, C*>::type, int&>::value);
        CHECK(std::is_same<eggs::invoke_result<Fn, C const*>::type,int const&>::value);

        CHECK(std::is_same<eggs::invoke_result<Fn, smart_ptr<C>>::type, int&>::value);
        CHECK(std::is_same<eggs::invoke_result<Fn, smart_ptr<C const>>::type, int const&>::value);

        CHECK(no_result<eggs::invoke_result<Fn>>::value);
        CHECK(no_result<eggs::invoke_result<Fn, int&>>::value);
        CHECK(no_result<eggs::invoke_result<Fn, C&, int>>::value);
    }

    /* fun-obj */ {
        struct Fn
        {
            int operator()(int) const noexcept
            {
                return 42;
            }
        };

        CHECK(std::is_same<eggs::invoke_result<Fn, int>::type, int>::value);
        CHECK(std::is_same<eggs::invoke_result<Fn, conv_to<int>>::type, int>::value);

        CHECK(no_result<eggs::invoke_result<Fn>>::value);
        CHECK(no_result<eggs::invoke_result<Fn, void*>>::value);
        CHECK(no_result<eggs::invoke_result<Fn, int, int>>::value);

        using Fn_ptr = int (*)(int);

        CHECK(std::is_same<eggs::invoke_result<Fn_ptr, int>::type, int>::value);
    }

    /* alias */ {
        CHECK(std::is_same<eggs::invoke_result_t<int (*)(int), int>,
            typename eggs::invoke_result<int (*)(int), int>::type>::value);
    }

    return test_report();
}
