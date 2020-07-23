//! \file eggs/invoke.hpp
// Eggs.Invoke
//
// Copyright Agustin K-ballo Berge, Fusion Fenix 2017-2020
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef EGGS_INVOKE_HPP
#define EGGS_INVOKE_HPP

#include <functional>
#include <type_traits>
#include <utility>

namespace eggs { namespace detail
{
#define EGGS_FWD(...) static_cast<decltype(__VA_ARGS__)&&>(__VA_ARGS__)

    ///////////////////////////////////////////////////////////////////////////
    template <typename C, typename T, bool Ref, bool RefWrapper,
        bool IsFunction = std::is_function<T>::value>
    struct invoke_mem_ptr;

    // when `pm` is a pointer to member of a class `C` and
    // `is_base_of_v<C, remove_reference_t<T>>` is `true`;
    template <typename C, typename T>
    struct invoke_mem_ptr<C, T, /*Ref=*/true, /*RefWrapper=*/false, /*IsFunction=*/false>
    {
        T C::*pm;

    public:
        constexpr invoke_mem_ptr(T C::*pm) noexcept
          : pm(pm)
        {}

        template <typename T1>
        constexpr auto operator()(T1&& t1) const
            noexcept(noexcept(EGGS_FWD(t1).*pm))
         -> decltype(EGGS_FWD(t1).*pm)
        {
            return EGGS_FWD(t1).*pm;
        }
    };

    template <typename C, typename T>
    struct invoke_mem_ptr<C, T, /*Ref=*/true, /*RefWrapper=*/false, /*IsFunction=*/true>
    {
        T C::*pm;

    public:
        constexpr invoke_mem_ptr(T C::*pm) noexcept
          : pm(pm)
        {}

        template <typename T1, typename... Tn>
        constexpr auto operator()(T1&& t1, Tn&&... tn) const
            noexcept(noexcept((EGGS_FWD(t1).*pm)(EGGS_FWD(tn)...)))
         -> decltype((EGGS_FWD(t1).*pm)(EGGS_FWD(tn)...))
        {
            return (EGGS_FWD(t1).*pm)(EGGS_FWD(tn)...);
        }
    };

    // when `pm` is a pointer to member of a class `C` and
    // `remove_cvref_t<T>` is a specialization of `reference_wrapper`;
    template <typename C, typename T>
    struct invoke_mem_ptr<C, T, /*Ref=*/false, /*RefWrapper=*/true, /*IsFunction=*/false>
    {
        T C::*pm;

    public:
        constexpr invoke_mem_ptr(T C::*pm) noexcept
          : pm(pm)
        {}

        template <typename T1>
        constexpr auto operator()(T1&& t1) const
            noexcept(noexcept(t1.get().*pm))
         -> decltype(t1.get().*pm)
        {
            return t1.get().*pm;
        }
    };

    template <typename C, typename T>
    struct invoke_mem_ptr<C, T, /*Ref=*/false, /*RefWrapper=*/true, /*IsFunction=*/true>
    {
        T C::*pm;

    public:
        constexpr invoke_mem_ptr(T C::*pm) noexcept
          : pm(pm)
        {}

        template <typename T1, typename... Tn>
        constexpr auto operator()(T1&& t1, Tn&&... tn) const
            noexcept(noexcept((t1.get().*pm)(EGGS_FWD(tn)...)))
         -> decltype((t1.get().*pm)(EGGS_FWD(tn)...))
        {
            return (t1.get().*pm)(EGGS_FWD(tn)...);
        }
    };

    // when `pm` is a pointer to member of a class `C` and `T` does not
    // satisfy the previous two items;
    template <typename C, typename T>
    struct invoke_mem_ptr<C, T, /*Ref=*/false, /*RefWrapper=*/false, /*IsFunction=*/false>
    {
        T C::*pm;

    public:
        constexpr invoke_mem_ptr(T C::*pm) noexcept
          : pm(pm)
        {}

        template <typename T1>
        constexpr auto operator()(T1&& t1) const
            noexcept(noexcept((*EGGS_FWD(t1)).*pm))
         -> decltype((*EGGS_FWD(t1)).*pm)
        {
            return (*EGGS_FWD(t1)).*pm;
        }
    };

    template <typename C, typename T>
    struct invoke_mem_ptr<C, T, /*Ref=*/false, /*RefWrapper=*/false, /*IsFunction=*/true>
    {
        T C::*pm;

    public:
        constexpr invoke_mem_ptr(T C::*pm) noexcept
          : pm(pm)
        {}

        template <typename T1, typename... Tn>
        constexpr auto operator()(T1&& t1, Tn&&... tn) const
            noexcept(noexcept(((*EGGS_FWD(t1)).*pm)(EGGS_FWD(tn)...)))
         -> decltype(((*EGGS_FWD(t1)).*pm)(EGGS_FWD(tn)...))
        {
            return ((*EGGS_FWD(t1)).*pm)(EGGS_FWD(tn)...);
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    template <typename F>
    auto invoke(F&&, ...)
     -> F&&;

    template <typename T, typename C, typename T1>
    auto invoke(T C::*, T1 const&, ...)
     -> invoke_mem_ptr<C, T,
            /*Ref=*/std::is_base_of<C, T1>::value,
            /*RefWrapper=*/false>;

    template <typename T, typename C, typename X>
    auto invoke(T C::*, std::reference_wrapper<X>, ...)
     -> invoke_mem_ptr<C, T,
            /*Ref=*/false,
            /*RefWrapper=*/true>;

#define EGGS_INVOKE(F, ...)                                                    \
    (static_cast<decltype(::eggs::detail::invoke(F, __VA_ARGS__))>(F)(__VA_ARGS__))

    // `INVOKE(f, t1, t2, ..., tN)` implicitly converted to `R`.
    template <typename R, typename RD = typename std::remove_cv<R>::type>
    struct invoke_r
    {
    private:
        static R conversion(R) noexcept;

    public:
        template <typename F, typename... Args>
        static constexpr auto call(F&& f, Args&&... args)
            noexcept(noexcept(conversion(
                EGGS_INVOKE(EGGS_FWD(f), EGGS_FWD(args)...))))
         -> decltype(conversion(
                EGGS_INVOKE(EGGS_FWD(f), EGGS_FWD(args)...)))
        {
            return EGGS_INVOKE(EGGS_FWD(f), EGGS_FWD(args)...);
        }
    };

    // `static_cast<void>(INVOKE(f, t1, t2, ..., tN))` if `R` is _cv_ `void`.
    template <typename R>
    struct invoke_r<R, void>
    {
        template <typename F, typename... Args>
        static constexpr auto call(F&& f, Args&&... args)
            noexcept(noexcept(
                EGGS_INVOKE(EGGS_FWD(f), EGGS_FWD(args)...)))
         -> decltype(static_cast<void>(
                EGGS_INVOKE(EGGS_FWD(f), EGGS_FWD(args)...)))
        {
            return static_cast<void>(
                EGGS_INVOKE(EGGS_FWD(f), EGGS_FWD(args)...));
        }
    };

#define EGGS_INVOKE_R(R, F, ...)                                               \
    (::eggs::detail::invoke_r<R>::call(F, __VA_ARGS__))

}}    // namespace eggs::detail

namespace eggs
{
    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        template <typename T, typename Enable = void>
        struct invoke_result_impl
        {};

        template <typename F, typename... Ts>
        struct invoke_result_impl<F(Ts...), decltype((void)
            EGGS_INVOKE(std::declval<F>(), std::declval<Ts>()...))>
        {
            using type = decltype(
                EGGS_INVOKE(std::declval<F>(), std::declval<Ts>()...));
        };
    }    // namespace detail

    //! template <class Fn, class... ArgTypes> struct invoke_result;
    //!
    //! - _Comments_: If the expression `INVOKE(std::declval<Fn>(),
    //!   std::declval<ArgTypes>()...)` is well-formed when treated as an
    //!   unevaluated operand, the member typedef `type` names the type
    //!   `decltype(INVOKE(std::declval<Fn>(), std::declval<ArgTypes>()...))`;
    //!   otherwise, there shall be no member `type`. Access checking is
    //!   performed as if in a context unrelated to `Fn` and `ArgTypes`. Only
    //!   the validity of the immediate context of the expression is considered.
    //!
    //! - _Preconditions_: `Fn` and all types in the template parameter pack
    //!   `ArgTypes` are complete types, _cv_ `void`, or arrays of unknown
    //!   bound.
    template <typename Fn, typename... ArgTypes>
    struct invoke_result
      : detail::invoke_result_impl<Fn&&(ArgTypes&&...)>
    {};

    //! template <class Fn, class... ArgTypes>
    //! using invoke_result_t = typename invoke_result<Fn, ArgTypes...>::type;
    template <typename Fn, typename... ArgTypes>
    using invoke_result_t =
        typename invoke_result<Fn, ArgTypes...>::type;

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        template <typename T, typename Enable = void>
        struct is_invocable_impl
          : std::false_type
        {};

        template <typename F, typename... Ts>
        struct is_invocable_impl<F(Ts...), decltype((void)
            EGGS_INVOKE(std::declval<F>(), std::declval<Ts>()...))>
          : std::true_type
        {};
    }

    //! template <class Fn, class... ArgTypes> struct is_invocable;
    //!
    //! - _Condition_: The expression `INVOKE(std::declval<Fn>(),
    //!   std::declval<ArgTypes>()...)` is well-formed when treated as an
    //!   unevaluated operand.
    //!
    //! - _Comments_: `Fn` and all types in the template parameter pack
    //!   `ArgTypes` shall be complete types, _cv_ `void`, or arrays of
    //!   unknown bound.
    template <typename Fn, typename... ArgTypes>
    struct is_invocable
      : detail::is_invocable_impl<Fn&&(ArgTypes&&...)>::type
    {};

#if __cpp_variable_templates
    //! template <class Fn, class... ArgTypes> // (C++14)
    //! inline constexpr bool is_invocable_v =
    //!     eggs::is_invocable<Fn, ArgTypes...>::value;
    template <typename Fn, typename... ArgTypes>
#if __cpp_inline_variables
    inline
#endif
    constexpr bool is_invocable_v =
        is_invocable<Fn, ArgTypes...>::value;
#endif

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        template <typename T, typename R, typename Enable = void>
        struct is_invocable_r_impl
          : std::false_type
        {};

        template <typename F, typename... Ts, typename R>
        struct is_invocable_r_impl<F(Ts...), R, decltype((void)
            EGGS_INVOKE_R(R, std::declval<F>(), std::declval<Ts>()...))>
          : std::true_type
        {};
    }

    //! template <class R, class Fn, class... ArgTypes> struct is_invocable_r;
    //!
    //! - _Condition_: The expression `INVOKE<R>(std::declval<Fn>(),
    //!   std::declval<ArgTypes>()...)` is well-formed when treated as an
    //!   unevaluated operand.
    //!
    //! - _Comments_: `Fn`, `R`, and all types in the template parameter pack
    //!   `ArgTypes` shall be complete types, _cv_ `void`, or arrays of
    //!   unknown bound.
    template <typename R, typename Fn, typename... ArgTypes>
    struct is_invocable_r
      : detail::is_invocable_r_impl<Fn&&(ArgTypes&&...), R>::type
    {};

#if __cpp_variable_templates
    //! template <class R, class Fn, class... ArgTypes> // (C++14)
    //! inline constexpr bool is_invocable_r_v =
    //!     eggs::is_invocable_r<R, Fn, ArgTypes...>::value;
    template <typename R, typename Fn, typename... ArgTypes>
#if __cpp_inline_variables
    inline
#endif
    constexpr bool is_invocable_r_v =
        is_invocable_r<R, Fn, ArgTypes...>::value;
#endif

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        template <typename T, typename Enable = void>
        struct is_nothrow_invocable_impl
          : std::false_type
        {};

        template <typename F, typename... Ts>
        struct is_nothrow_invocable_impl<F(Ts...), decltype((void)
            EGGS_INVOKE(std::declval<F>(), std::declval<Ts>()...))>
          : std::integral_constant<bool, noexcept(
                EGGS_INVOKE(std::declval<F>(), std::declval<Ts>()...))>
        {};
    }

    //! template <class Fn, class... ArgTypes> struct is_nothrow_invocable;
    //!
    //! - _Condition_: `eggs::is_invocable_v<Fn, ArgTypes...>` is `true` and
    //!   the expression `INVOKE(std::declval<Fn>(), std::declval<ArgTypes>()...)`
    //!   is known not to throw any exceptions.
    //!
    //! - _Comments_: `Fn` and all types in the template parameter pack
    //!   `ArgTypes` shall be complete types, _cv_ `void`, or arrays of
    //!   unknown bound.
    template <typename Fn, typename... ArgTypes>
    struct is_nothrow_invocable
      : detail::is_nothrow_invocable_impl<Fn&&(ArgTypes&&...)>::type
    {};

#if __cpp_variable_templates
    //! template <class Fn, class... ArgTypes> // (C++14)
    //! inline constexpr bool is_nothrow_invocable_v =
    //!     eggs::is_nothrow_invocable<Fn, ArgTypes...>::value;
    template <typename Fn, typename... ArgTypes>
#if __cpp_inline_variables
    inline
#endif
    constexpr bool is_nothrow_invocable_v =
        is_nothrow_invocable<Fn, ArgTypes...>::value;
#endif

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        template <typename T, typename R, typename Enable = void>
        struct is_nothrow_invocable_r_impl
          : std::false_type
        {};

        template <typename F, typename... Ts, typename R>
        struct is_nothrow_invocable_r_impl<F(Ts...), R, decltype((void)
            EGGS_INVOKE_R(R, std::declval<F>(), std::declval<Ts>()...))>
          : std::integral_constant<bool, noexcept(
                EGGS_INVOKE_R(R, std::declval<F>(), std::declval<Ts>()...))>
        {};
    }

    //! template <class R, class Fn, class... ArgTypes> struct is_nothrow_invocable_r;
    //!
    //! - _Condition_: `eggs::is_invocable_r_v<R, Fn, ArgTypes...>` is `true`
    //!   and the expression `INVOKE(std::declval<Fn>(), std::declval<ArgTypes>()...)`
    //!   is known not to throw any exceptions.
    //!
    //! - _Comments_: `Fn`, `R`, and all types in the template parameter pack
    //!   `ArgTypes` shall be complete types, _cv_ `void`, or arrays of
    //!   unknown bound.
    template <typename R, typename Fn, typename... ArgTypes>
    struct is_nothrow_invocable_r
      : detail::is_nothrow_invocable_r_impl<Fn&&(ArgTypes&&...), R>::type
    {};

#if __cpp_variable_templates
    //! template <class R, class Fn, class... ArgTypes> // (C++14)
    //! inline constexpr bool is_nothrow_invocable_r_v =
    //!     eggs::is_nothrow_invocable_r<R, Fn, ArgTypes...>::value;
    template <typename R, typename Fn, typename... ArgTypes>
#if __cpp_inline_variables
    inline
#endif
    constexpr bool is_nothrow_invocable_r_v =
        is_nothrow_invocable_r<R, Fn, ArgTypes...>::value;
#endif

    ///////////////////////////////////////////////////////////////////////////
    //! template <class F, class... Args>
    //! constexpr eggs::invoke_result_t<F, Args...> invoke(F&& f, Args&&... args)
    //!     noexcept(eggs::is_nothrow_invocable_v<F, Args...>);
    //!
    //! - _Returns_: `INVOKE(std::forward<F>(f), std::forward<Args>(args)...)`.
    //!
    //! - _Remarks_: This function shall not participate in overload resolution
    //!   unless `eggs::is_invocable_v<F, Args...>` is `true`.
    template <typename Fn, typename... ArgTypes>
    constexpr auto
    invoke(Fn&& f, ArgTypes&&... args)
        noexcept(noexcept(EGGS_INVOKE(EGGS_FWD(f), EGGS_FWD(args)...)))
     -> decltype(EGGS_INVOKE(EGGS_FWD(f), EGGS_FWD(args)...))
    {
        return EGGS_INVOKE(EGGS_FWD(f), EGGS_FWD(args)...);
    }

    ///////////////////////////////////////////////////////////////////////////
    //! template <class R, class F, class... Args> // (extension)
    //! constexpr R eggs::invoke_r(F&& f, Args&&... args)
    //!     noexcept(eggs::is_nothrow_invocable_r_v<R, F, Args...>);
    //!
    //! - _Returns_: `INVOKE<R>(std::forward<F>(f), std::forward<Args>(args)...)`.
    //!
    //! - _Remarks_: This function shall not participate in overload resolution
    //!   unless `eggs::is_invocable_r_v<R, F, Args...>` is `true`.
    template <typename R, typename Fn, typename... ArgTypes>
    constexpr auto
    invoke_r(Fn&& f, ArgTypes&&... args)
        noexcept(noexcept(EGGS_INVOKE_R(R, EGGS_FWD(f), EGGS_FWD(args)...)))
     -> decltype(EGGS_INVOKE_R(R, EGGS_FWD(f), EGGS_FWD(args)...))
    {
        return EGGS_INVOKE_R(R, EGGS_FWD(f), EGGS_FWD(args)...);
    }

#undef EGGS_FWD
}    // namespace eggs

#endif /*EGGS_INVOKE_HPP*/
