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

#if __cpp_constexpr >= 201304
#define constexpr14 constexpr
#define CHECK_CONSTEXPR14 CHECK_CONSTEXPR
#else
#define constexpr14
#define CHECK_CONSTEXPR14
#endif

#if __cplusplus >= 202002L && __cpp_lib_constexpr_functional >= 201907L
#define constexpr20 constexpr
#define CHECK_CONSTEXPR20 CHECK_CONSTEXPR
#else
#define constexpr20
#define CHECK_CONSTEXPR20
#endif

// Account for P0012: "Make exception specifications be part of the type system".
static constexpr bool p0012 = !std::is_same<void(), void() noexcept>::value;

struct C
{
    int obj;

    constexpr C(int val)
      : obj(val)
    {}

    constexpr14 int fun(int base) noexcept(p0012)
    {
        return base + 0;
    }
    constexpr int cfun(int base) const noexcept(p0012)
    {
        return base + 1;
    }
    constexpr14 int lfun(int base) & noexcept(p0012)
    {
        return base + 2;
    }
    constexpr14 int rfun(int base) && noexcept(p0012)
    {
        return base + 3;
    }
    constexpr int clfun(int base) const& noexcept(p0012)
    {
        return base + 4;
    }
    constexpr int crfun(int base) const&& noexcept(p0012)
    {
        return base + 5;
    }
};

struct D : C
{
    constexpr D(int val) : C(val)
    {}
};

template <typename T, bool IsNothrow = true>
struct smart_ptr
{
    T* ptr;
    constexpr smart_ptr(T* ptr) : ptr(ptr) {}
    constexpr T& operator*() const noexcept(IsNothrow) { return *ptr; }
};
template <typename T>
using smart_ptr_throws = smart_ptr<T, false>;

template <typename T, bool IsNothrow = true>
struct conv_to
{
    T val;
    constexpr operator T() const noexcept(IsNothrow) { return val; }
    constexpr bool operator==(conv_to const& other) const noexcept { return val == other.val; }
};
template <typename T>
using conv_to_throws = conv_to<T, false>;

template <typename T, bool IsNothrow = true>
struct conv_from
{
    T val;
    constexpr conv_from(T val) noexcept(IsNothrow) : val(val) {}
    constexpr bool operator==(conv_from const& other) const noexcept { return val == other.val; }
};
template <typename T>
using conv_from_throws = conv_from<T, false>;

void test_mem_obj_ptr()
{
    constexpr auto obj = &C::obj;

    /* reference */ {
        static C x = {42};
        constexpr C& r = x;
        constexpr C const& cr = x;

        EGGS_INVOKE(obj, r);
        EGGS_INVOKE(obj, cr);
        EGGS_INVOKE(obj, std::move(r));
        EGGS_INVOKE(obj, std::move(cr));

        static D d = {42};
        constexpr D& rd = d;
        constexpr D const& crd = d;

        EGGS_INVOKE(obj, rd);
        EGGS_INVOKE(obj, crd);
        EGGS_INVOKE(obj, std::move(rd));
        EGGS_INVOKE(obj, std::move(crd));
    }

    /* reference wrapper */ {
        static C x = {42};
        constexpr20 std::reference_wrapper<C> r = x;
        constexpr20 std::reference_wrapper<C const> cr = x;

        EGGS_INVOKE(obj, r);
        EGGS_INVOKE(obj, cr);
    }

    /* pointer */ {
        static C x = {42};
        constexpr C* p = &x;
        constexpr C const* cp = &x;

        EGGS_INVOKE(obj, p);
        EGGS_INVOKE(obj, cp);
    }

    /* smart pointer */ {
        static C x = {42};
        constexpr smart_ptr<C> p = &x;
        constexpr smart_ptr<C const> cp = &x;

        EGGS_INVOKE(obj, p);
        EGGS_INVOKE(obj, cp);

        constexpr smart_ptr_throws<C> tp = &x;
        constexpr smart_ptr_throws<C const> tcp = &x;

        EGGS_INVOKE(obj, tp);
        EGGS_INVOKE(obj, tcp);
    }
}

void test_mem_fun_ptr()
{
    constexpr auto fun = &C::fun;
    constexpr auto cfun = &C::cfun;
    constexpr auto lfun = &C::lfun;
    constexpr auto rfun = &C::rfun;
    constexpr auto clfun = &C::clfun;
    constexpr auto crfun = &C::crfun;

    /* reference */ {
        static C x = {42};
        constexpr C& r = x;
        constexpr C const& cr = x;

        EGGS_INVOKE(fun, r, 40);
        EGGS_INVOKE(cfun, r, 40);
        EGGS_INVOKE(lfun, r, 40);
        EGGS_INVOKE(clfun, r, 40);

        EGGS_INVOKE(cfun, cr, 40);
        EGGS_INVOKE(clfun, cr, 40);

        EGGS_INVOKE(fun, std::move(r), 40);
        EGGS_INVOKE(cfun, std::move(r), 40);
#if __cplusplus > 201703L || defined(_MSC_VER) // C++20: P0704
        EGGS_INVOKE(clfun, std::move(r), 40);
#else
#endif
        EGGS_INVOKE(rfun, std::move(r), 40);
        EGGS_INVOKE(crfun, std::move(r), 40);

        EGGS_INVOKE(cfun, std::move(cr), 40);
#if __cplusplus > 201703L || defined(_MSC_VER)  // C++20: P0704
        EGGS_INVOKE(clfun, std::move(cr), 40);
#else
#endif
        EGGS_INVOKE(crfun, std::move(cr), 40);
    }

    /* reference wrapper */ {
        static C x = {42};
        constexpr20 std::reference_wrapper<C> r = x;
        constexpr20 std::reference_wrapper<C const> cr = x;

        EGGS_INVOKE(fun, r, 40);
        EGGS_INVOKE(cfun, r, 40);
        EGGS_INVOKE(lfun, r, 40);
        EGGS_INVOKE(clfun, r, 40);

        EGGS_INVOKE(cfun, cr, 40);
        EGGS_INVOKE(clfun, cr, 40);
    }

    /* pointer */ {
        static C x = {42};
        constexpr C* p = &x;
        constexpr C const* cp = &x;

        EGGS_INVOKE(fun, p, 40);
        EGGS_INVOKE(cfun, p, 40);
        EGGS_INVOKE(lfun, p, 40);
        EGGS_INVOKE(clfun, p, 40);

        EGGS_INVOKE(cfun, cp, 40);
        EGGS_INVOKE(clfun, cp, 40);
    }

    /* smart pointer */ {
        static C x = {42};
        constexpr smart_ptr<C> p = &x;
        constexpr smart_ptr<C const> cp = &x;

        EGGS_INVOKE(fun, p, 40);
        EGGS_INVOKE(cfun, p, 40);
        EGGS_INVOKE(lfun, p, 40);
        EGGS_INVOKE(clfun, p, 40);

        EGGS_INVOKE(cfun, cp, 40);
        EGGS_INVOKE(clfun, cp, 40);

        constexpr smart_ptr_throws<C> tp = &x;
        constexpr smart_ptr_throws<C const> tcp = &x;

        EGGS_INVOKE(fun, tp, 40);
        EGGS_INVOKE(cfun, tp, 40);
        EGGS_INVOKE(lfun, tp, 40);
        EGGS_INVOKE(clfun, tp, 40);

        EGGS_INVOKE(cfun, tcp, 40);
        EGGS_INVOKE(clfun, tcp, 40);
    }
}

void test_fun_obj()
{
    /* call-op */ {
        struct Fn
        {
            constexpr14 int operator()(int base) noexcept
            {
                return base + 0;
            }
        };
        auto f = Fn{};
        //constexpr auto cf = Fn{};

        EGGS_INVOKE(f, 40);
        EGGS_INVOKE(std::move(f), 40);

        struct Fnc
        {
            constexpr int operator()(int base) const noexcept
            {
                return base + 1;
            }
        };
        auto fc = Fnc{};
        constexpr auto cfc = Fnc{};

        EGGS_INVOKE(fc, 40);
        EGGS_INVOKE(cfc, 40);
        EGGS_INVOKE(std::move(fc), 40);
        EGGS_INVOKE(std::move(cfc), 40);

        struct Fnl
        {
            constexpr14 int operator()(int base) & noexcept
            {
                return base + 2;
            }
        };
        auto fl = Fnl{};
        //constexpr auto cfl = Fnl{};

        EGGS_INVOKE(fl, 40);

        struct Fnr
        {
            constexpr14 int operator()(int base) && noexcept
            {
                return base + 3;
            }
        };
        auto fr = Fnr{};
        //constexpr auto cfr = Fnr{};

        EGGS_INVOKE(std::move(fr), 40);

        struct Fncl
        {
            constexpr int operator()(int base) const& noexcept
            {
                return base + 4;
            }
        };
        auto fcl = Fncl{};
        constexpr auto cfcl = Fncl{};

        EGGS_INVOKE(fcl, 40);
        EGGS_INVOKE(cfcl, 40);
        EGGS_INVOKE(std::move(fcl), 40);
        EGGS_INVOKE(std::move(cfcl), 40);

        struct Fncr
        {
            constexpr int operator()(int base) const&& noexcept
            {
                return base + 5;
            }
        };
        auto fcr = Fncr{};
        constexpr auto cfcr = Fncr{};

        EGGS_INVOKE(std::move(fcr), 40);
        EGGS_INVOKE(std::move(cfcr), 40);
    }

    /* fun-ptr */ {
        struct S
        {
            static constexpr int f(int base) noexcept(p0012)
            {
                return base + 6;
            }
        };
        constexpr auto f = &S::f;

        EGGS_INVOKE(f, 40);
        EGGS_INVOKE_R(int, f, 40);

        EGGS_INVOKE(f, conv_to<int>{40});
        EGGS_INVOKE(f, conv_to_throws<int>{40});
        EGGS_INVOKE_R(conv_from<int>, f, 40);
        EGGS_INVOKE_R(conv_from_throws<int>, f, 40);
    }

    /* fun-ref */ {
        struct S
        {
            static constexpr int f0() noexcept(p0012)
            {
                return 6;
            }
            static constexpr int f1(int base) noexcept(p0012)
            {
                return base + 6;
            }
        };

        EGGS_INVOKE(S::f0);
        EGGS_INVOKE_R(int, S::f0);
        EGGS_INVOKE(S::f1, 40);
        EGGS_INVOKE_R(int, S::f1, 40);
    }
}

int main()
{
    test_mem_obj_ptr();
    test_mem_fun_ptr();
    test_fun_obj();

    return test_report();
}
