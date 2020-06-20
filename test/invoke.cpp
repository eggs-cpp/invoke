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

// [func.invoke], invoke

// template <class F, class... Args>
//   invoke_result_t<F, Args...> invoke(F&& f, Args&&... args)
//     noexcept(is_nothrow_invocable_v<F, Args...>);

// Account for P0012: "Make exception specifications be part of the type system".
static constexpr bool p0012 = !std::is_same<void(), void() noexcept>::value;

struct C
{
    int obj;
    C(int val)
      : obj(val)
    {
    }

    int fun(int base) noexcept(p0012)
    {
        return base + 0;
    }
    int cfun(int base) const noexcept(p0012)
    {
        return base + 1;
    }
    int lfun(int base) & noexcept(p0012)
    {
        return base + 2;
    }
    int rfun(int base) && noexcept(p0012)
    {
        return base + 3;
    }
    int clfun(int base) const& noexcept(p0012)
    {
        return base + 4;
    }
    int crfun(int base) const&& noexcept(p0012)
    {
        return base + 5;
    }
};

struct D : C
{
    D(int val) : C(val)
    {}
};

template <typename T, bool IsNothrow = true>
struct smart_ptr
{
    T* ptr;
    smart_ptr(T* ptr) : ptr(ptr) {}
    T& operator*() const noexcept(IsNothrow) { return *ptr; }
};
template <typename T>
using smart_ptr_throws = smart_ptr<T, false>;

template <typename T, bool IsNothrow = true>
struct conv_to
{
    T val;
    conv_to(T val) : val(val) {}
    operator T() const noexcept(IsNothrow) { return val; }
};
template <typename T>
using conv_to_throws = conv_to<T, false>;

std::true_type const nothrows{};
std::false_type const throws{};
std::integral_constant<bool, p0012> const p0012_nothrows{};

template <typename T>
T const* addressof(T const& ref)
{
    return &ref;
}

template <typename R, bool IsNothrow, typename F, typename A1>
void test_invoke_obj(
    R&& r, std::integral_constant<bool, IsNothrow>, F&& f, A1&& a1)
{
    CHECK(::addressof(eggs::invoke(std::forward<F>(f), std::forward<A1>(a1))) == ::addressof(r));
    CHECK(::addressof(eggs::invoke_r<R&&>(std::forward<F>(f), std::forward<A1>(a1))) == ::addressof(r));
    CHECK(std::is_same<decltype(eggs::invoke(std::forward<F>(f), std::forward<A1>(a1))), R&&>::value);
    CHECK(std::is_same<decltype(eggs::invoke_r<R&&>(std::forward<F>(f), std::forward<A1>(a1))), R&&>::value);
    CHECK(std::is_same<decltype(eggs::invoke_r<void>(std::forward<F>(f), std::forward<A1>(a1))), void>::value);
    CHECK(noexcept(eggs::invoke(std::forward<F>(f), std::forward<A1>(a1))) == IsNothrow);
    CHECK(noexcept(eggs::invoke_r<R&&>(std::forward<F>(f), std::forward<A1>(a1))) == IsNothrow);
    CHECK(noexcept(eggs::invoke_r<void>(std::forward<F>(f), std::forward<A1>(a1))) == IsNothrow);
}

template <typename R, bool IsNothrow, typename F, typename... Args>
void test_invoke_fun(
    R&& r, std::integral_constant<bool, IsNothrow>, F&& f, Args&&... args)
{
    CHECK(eggs::invoke(std::forward<F>(f), std::forward<Args>(args)...) == r);
    CHECK(eggs::invoke_r<R>(std::forward<F>(f), std::forward<Args>(args)...) == r);
    CHECK(std::is_same<decltype(eggs::invoke(std::forward<F>(f), std::forward<Args>(args)...)), R>::value);
    CHECK(std::is_same<decltype(eggs::invoke_r<R>(std::forward<F>(f), std::forward<Args>(args)...)), R>::value);
    CHECK(std::is_same<decltype(eggs::invoke_r<void>(std::forward<F>(f), std::forward<Args>(args)...)), void>::value);
    CHECK(noexcept(eggs::invoke(std::forward<F>(f), std::forward<Args>(args)...)) == IsNothrow);
    CHECK(noexcept(eggs::invoke_r<R>(std::forward<F>(f), std::forward<Args>(args)...)) == IsNothrow);
    CHECK(noexcept(eggs::invoke_r<void>(std::forward<F>(f), std::forward<Args>(args)...)) == IsNothrow);
}

void test_mem_obj_ptr()
{
    auto f = &C::obj;

    /* reference */ {
        C x = {42};
        C& r = x;
        C const& cr = x;

        CHECK_SCOPE(test_invoke_obj(r.obj, nothrows, f, r));
        CHECK_SCOPE(test_invoke_obj(cr.obj, nothrows, f, cr));
        CHECK_SCOPE(test_invoke_obj(std::move(r.obj), nothrows, f, std::move(r)));
        CHECK_SCOPE(test_invoke_obj(std::move(cr.obj), nothrows, f, std::move(cr)));

        D d = {42};
        D& rd = d;
        D const& crd = d;

        CHECK_SCOPE(test_invoke_obj(rd.obj, nothrows, f, rd));
        CHECK_SCOPE(test_invoke_obj(crd.obj, nothrows, f, crd));
        CHECK_SCOPE(test_invoke_obj(std::move(rd.obj), nothrows, f, std::move(rd)));
        CHECK_SCOPE(test_invoke_obj(std::move(crd.obj), nothrows, f, std::move(crd)));
    }

    /* reference wrapper */ {
        C x = {42};
        std::reference_wrapper<C> r = x;
        std::reference_wrapper<C const> cr = x;

        CHECK_SCOPE(test_invoke_obj(r.get().obj, nothrows, f, r));
        CHECK_SCOPE(test_invoke_obj(cr.get().obj, nothrows, f, cr));
    }

    /* pointer */ {
        C x = {42};
        C* p = &x;
        C const* cp = &x;

        CHECK_SCOPE(test_invoke_obj((*p).obj, nothrows, f, p));
        CHECK_SCOPE(test_invoke_obj((*cp).obj, nothrows, f, cp));
    }

    /* smart pointer */ {
        C x = {42};
        smart_ptr<C> p = &x;
        smart_ptr<C const> cp = &x;

        CHECK_SCOPE(test_invoke_obj((*p).obj, nothrows, f, p));
        CHECK_SCOPE(test_invoke_obj((*cp).obj, nothrows, f, cp));

        smart_ptr_throws<C> tp = &x;
        smart_ptr_throws<C const> tcp = &x;

        CHECK_SCOPE(test_invoke_obj((*tp).obj, throws, f, tp));
        CHECK_SCOPE(test_invoke_obj((*tcp).obj, throws, f, tcp));
    }
}

void test_mem_fun_ptr()
{
    auto f = &C::fun;
    auto cf = &C::cfun;
    auto lf = &C::lfun;
    auto rf = &C::rfun;
    auto clf = &C::clfun;
    auto crf = &C::crfun;

    /* reference */ {
        C x = {42};
        C& r = x;
        C const& cr = x;

        CHECK_SCOPE(test_invoke_fun(r.fun(40), p0012_nothrows, f, r, 40));
        CHECK_SCOPE(test_invoke_fun(r.cfun(40), p0012_nothrows, cf, r, 40));
        CHECK_SCOPE(test_invoke_fun(r.lfun(40), p0012_nothrows, lf, r, 40));
        CHECK_SCOPE(test_invoke_fun(r.clfun(40), p0012_nothrows, clf, r, 40));

        CHECK_SCOPE(test_invoke_fun(cr.cfun(40), p0012_nothrows, cf, cr, 40));
        CHECK_SCOPE(test_invoke_fun(cr.clfun(40), p0012_nothrows, clf, cr, 40));

        CHECK_SCOPE(test_invoke_fun(std::move(r).fun(40), p0012_nothrows, f, std::move(r), 40));
        CHECK_SCOPE(test_invoke_fun(std::move(r).cfun(40), p0012_nothrows, cf, std::move(r), 40));
#if __cplusplus > 201703L || defined(_MSC_VER) // C++20: P0704
        CHECK_SCOPE(test_invoke_fun(std::move(r).clfun(40), p0012_nothrows, clf, std::move(r), 40));
#endif
        CHECK_SCOPE(test_invoke_fun(std::move(r).rfun(40), p0012_nothrows, rf, std::move(r), 40));
        CHECK_SCOPE(test_invoke_fun(std::move(r).crfun(40), p0012_nothrows, crf, std::move(r), 40));

        CHECK_SCOPE(test_invoke_fun(std::move(cr).cfun(40), p0012_nothrows, cf, std::move(cr), 40));
#if __cplusplus > 201703L || defined(_MSC_VER)  // C++20: P0704
        CHECK_SCOPE(test_invoke_fun(std::move(cr).clfun(40), p0012_nothrows, clf, std::move(cr), 40));
#endif
        CHECK_SCOPE(test_invoke_fun(std::move(cr).crfun(40), p0012_nothrows, crf, std::move(cr), 40));
    }

    /* reference wrapper */ {
        C x = {42};
        std::reference_wrapper<C> r = x;
        std::reference_wrapper<C const> cr = x;

        CHECK_SCOPE(test_invoke_fun(r.get().fun(40), p0012_nothrows, f, r, 40));
        CHECK_SCOPE(test_invoke_fun(r.get().cfun(40), p0012_nothrows, cf, r, 40));
        CHECK_SCOPE(test_invoke_fun(r.get().lfun(40), p0012_nothrows, lf, r, 40));
        CHECK_SCOPE(test_invoke_fun(r.get().clfun(40), p0012_nothrows, clf, r, 40));

        CHECK_SCOPE(test_invoke_fun(cr.get().cfun(40), p0012_nothrows, cf, cr, 40));
        CHECK_SCOPE(test_invoke_fun(cr.get().clfun(40), p0012_nothrows, clf, cr, 40));
    }

    /* pointer */ {
        C x = {42};
        C* p = &x;
        C const* cp = &x;

        CHECK_SCOPE(test_invoke_fun((*p).fun(40), p0012_nothrows, f, p, 40));
        CHECK_SCOPE(test_invoke_fun((*p).cfun(40), p0012_nothrows, cf, p, 40));
        CHECK_SCOPE(test_invoke_fun((*p).lfun(40), p0012_nothrows, lf, p, 40));
        CHECK_SCOPE(test_invoke_fun((*p).clfun(40), p0012_nothrows, clf, p, 40));

        CHECK_SCOPE(test_invoke_fun((*cp).cfun(40), p0012_nothrows, cf, cp, 40));
        CHECK_SCOPE(test_invoke_fun((*cp).clfun(40), p0012_nothrows, clf, cp, 40));
    }

    /* smart pointer */ {
        C x = {42};
        smart_ptr<C> p = &x;
        smart_ptr<C const> cp = &x;

        CHECK_SCOPE(test_invoke_fun((*p).fun(40), p0012_nothrows, f, p, 40));
        CHECK_SCOPE(test_invoke_fun((*p).cfun(40), p0012_nothrows, cf, p, 40));
        CHECK_SCOPE(test_invoke_fun((*p).lfun(40), p0012_nothrows, lf, p, 40));
        CHECK_SCOPE(test_invoke_fun((*p).clfun(40), p0012_nothrows, clf, p, 40));

        CHECK_SCOPE(test_invoke_fun((*cp).cfun(40), p0012_nothrows, cf, cp, 40));
        CHECK_SCOPE(test_invoke_fun((*cp).clfun(40), p0012_nothrows, clf, cp, 40));

        smart_ptr_throws<C> tp = &x;
        smart_ptr_throws<C const> tcp = &x;

        CHECK_SCOPE(test_invoke_fun((*tp).fun(40), throws, f, tp, 40));
        CHECK_SCOPE(test_invoke_fun((*tp).cfun(40), throws, cf, tp, 40));
        CHECK_SCOPE(test_invoke_fun((*tp).lfun(40), throws, lf, tp, 40));
        CHECK_SCOPE(test_invoke_fun((*tp).clfun(40), throws, clf, tp, 40));

        CHECK_SCOPE(test_invoke_fun((*tcp).cfun(40), throws, cf, tcp, 40));
        CHECK_SCOPE(test_invoke_fun((*tcp).clfun(40), throws, clf, tcp, 40));
    }
}

void test_fun_obj()
{
    /* call-op */ {
        struct Fn
        {
            int operator()(int base) noexcept
            {
                return base + 6;
            }
        };
        auto f = Fn{};

        CHECK_SCOPE(test_invoke_fun(f(40), nothrows, f, 40));
        CHECK_SCOPE(test_invoke_fun(f(conv_to<int>(40)), nothrows, f, conv_to<int>(40)));
        CHECK_SCOPE(test_invoke_fun(f(conv_to_throws<int>(40)), throws, f, conv_to_throws<int>(40)));
    }

    /* fun-ptr */ {
        struct S
        {
            static int f(int base) noexcept(p0012)
            {
                return base + 7;
            }
        };
        auto f = &S::f;

        CHECK_SCOPE(test_invoke_fun(f(40), p0012_nothrows, f, 40));
    }
}

int main()
{
    test_mem_obj_ptr();
    test_mem_fun_ptr();
    test_fun_obj();

    return test_report();
}
