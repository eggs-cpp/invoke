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
    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    struct is_reference_wrapper
      : std::false_type
    {};

    template <typename T>
    struct is_reference_wrapper<std::reference_wrapper<T>>
      : std::true_type
    {};

    ///////////////////////////////////////////////////////////////////////////
    // when `pm` is a pointer to member of a class `C` and
    // `is_base_of_v<C, remove_reference_t<T>>` is `true`;
    template <typename C, typename T, typename Enable = typename std::enable_if<
        std::is_base_of<C, typename std::remove_reference<T>::type>::value>::type>
    static constexpr T&& mem_ptr_target(T& v) noexcept
    {
        return std::forward<T>(v);
    }

    // when `pm` is a pointer to member of a class `C` and
    // `remove_cvref_t<T>` is a specialization of `reference_wrapper`;
    template <typename C, typename T, typename Enable = typename std::enable_if<
        detail::is_reference_wrapper<typename std::remove_cv<
            typename std::remove_reference<T>::type>::type>::value>::type>
    static constexpr auto mem_ptr_target(T& v) noexcept
     -> decltype(v.get())
    {
        return v.get();
    }

    // when `pm` is a pointer to member of a class `C` and `T` does not
    // satisfy the previous two items;
    template <typename C, typename T>
    static constexpr auto mem_ptr_target(T& v)
        noexcept(noexcept(*std::forward<T>(v)))
     -> decltype(*std::forward<T>(v))
    {
        return *std::forward<T>(v);
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T, bool IsFunction>
    struct invoke_mem_ptr;

    template <typename T, typename C>
    struct invoke_mem_ptr<T C::*, /*IsFunction=*/false>
    {
        T C::*pm;

    public:
        constexpr invoke_mem_ptr(T C::*pm) noexcept
          : pm(pm)
        {}

        template <typename T1>
        constexpr auto operator()(T1&& t1) const
            noexcept(noexcept(
                detail::mem_ptr_target<C, T1>(t1).*pm))
         -> decltype(
                detail::mem_ptr_target<C, T1>(t1).*pm)
        {
            return detail::mem_ptr_target<C, T1>(t1).*pm;
        }
    };

    template <typename T, typename C>
    struct invoke_mem_ptr<T C::*, /*IsFunction=*/true>
    {
        T C::*pm;

    public:
        constexpr invoke_mem_ptr(T C::*pm) noexcept
          : pm(pm)
        {}

        template <typename T1, typename... Tn>
        constexpr auto operator()(T1&& t1, Tn&&... tn) const
            noexcept(noexcept(
                (detail::mem_ptr_target<C, T1>(t1).*pm)(std::forward<Tn>(tn)...)))
         -> decltype(
                (detail::mem_ptr_target<C, T1>(t1).*pm)(std::forward<Tn>(tn)...))
        {
            return (detail::mem_ptr_target<C, T1>(t1).*pm)(std::forward<Tn>(tn)...);
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    template <typename F, typename FD = typename std::remove_cv<
        typename std::remove_reference<F>::type>::type>
    struct dispatch_invoke
    {
        using type = F&&;
    };

    template <typename F, typename T, typename C>
    struct dispatch_invoke<F, T C::*>
    {
        using type = invoke_mem_ptr<T C::*, std::is_function<T>::value>;
    };

    template <typename F>
    using invoke = typename dispatch_invoke<F>::type;

#define EGGS_INVOKE(F, ...)                                                    \
    (::eggs::detail::invoke<decltype((F))>(F)(__VA_ARGS__))

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
                EGGS_INVOKE(std::forward<F>(f), std::forward<Args>(args)...))))
         -> decltype(conversion(
                EGGS_INVOKE(std::forward<F>(f), std::forward<Args>(args)...)))
        {
            return EGGS_INVOKE(std::forward<F>(f), std::forward<Args>(args)...);
        }
    };

    // `static_cast<void>(INVOKE(f, t1, t2, ..., tN))` if `R` is _cv_ `void`.
    template <typename R>
    struct invoke_r<R, void>
    {
        template <typename F, typename... Args>
        static constexpr auto call(F&& f, Args&&... args)
            noexcept(noexcept(
                EGGS_INVOKE(std::forward<F>(f), std::forward<Args>(args)...)))
         -> std::void_t<decltype(
                EGGS_INVOKE(std::forward<F>(f), std::forward<Args>(args)...))>
        {
            return static_cast<void>(
                EGGS_INVOKE(std::forward<F>(f), std::forward<Args>(args)...));
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
        struct invoke_result_impl<F(Ts...), std::void_t<decltype(
            EGGS_INVOKE(std::declval<F>(), std::declval<Ts>()...))>>
        {
            using type = decltype(
                EGGS_INVOKE(std::declval<F>(), std::declval<Ts>()...));
        };
    }    // namespace detail

    template <typename Fn, typename... ArgTypes>
    struct invoke_result
      : detail::invoke_result_impl<Fn&&(ArgTypes&&...)>
    {};

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
        struct is_invocable_impl<F(Ts...), std::void_t<decltype(
            EGGS_INVOKE(std::declval<F>(), std::declval<Ts>()...))>>
          : std::true_type
        {};
    }

    template <typename Fn, typename... ArgTypes>
    struct is_invocable
      : detail::is_invocable_impl<Fn&&(ArgTypes&&...)>::type
    {};

    template <typename Fn, typename... ArgTypes>
    inline constexpr bool is_invocable_v =
        is_invocable<Fn, ArgTypes...>::value;

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        template <typename T, typename R, typename Enable = void>
        struct is_invocable_r_impl
          : std::false_type
        {};

        template <typename F, typename... Ts, typename R>
        struct is_invocable_r_impl<F(Ts...), R, std::void_t<decltype(
            EGGS_INVOKE_R(R, std::declval<F>(), std::declval<Ts>()...))>>
          : std::true_type
        {};
    }

    template <typename R, typename Fn, typename... ArgTypes>
    struct is_invocable_r
      : detail::is_invocable_r_impl<Fn&&(ArgTypes&&...), R>::type
    {};

    template <typename R, typename Fn, typename... ArgTypes>
    inline constexpr bool is_invocable_r_v =
        is_invocable_r<R, Fn, ArgTypes...>::value;

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        template <typename T, typename Enable = void>
        struct is_nothrow_invocable_impl
          : std::false_type
        {};

        template <typename F, typename... Ts>
        struct is_nothrow_invocable_impl<F(Ts...), std::void_t<decltype(
            EGGS_INVOKE(std::declval<F>(), std::declval<Ts>()...))>>
          : std::bool_constant<noexcept(
                EGGS_INVOKE(std::declval<F>(), std::declval<Ts>()...))>
        {};
    }

    template <typename Fn, typename... ArgTypes>
    struct is_nothrow_invocable
      : detail::is_nothrow_invocable_impl<Fn&&(ArgTypes&&...)>::type
    {};

    template <typename Fn, typename... ArgTypes>
    inline constexpr bool is_nothrow_invocable_v =
        is_nothrow_invocable<Fn, ArgTypes...>::value;

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        template <typename T, typename R, typename Enable = void>
        struct is_nothrow_invocable_r_impl
          : std::false_type
        {};

        template <typename F, typename... Ts, typename R>
        struct is_nothrow_invocable_r_impl<F(Ts...), R, std::void_t<decltype(
            EGGS_INVOKE_R(R, std::declval<F>(), std::declval<Ts>()...))>>
          : std::bool_constant<noexcept(
                EGGS_INVOKE_R(R, std::declval<F>(), std::declval<Ts>()...))>
        {};
    }

    template <typename R, typename Fn, typename... ArgTypes>
    struct is_nothrow_invocable_r
      : detail::is_nothrow_invocable_r_impl<Fn&&(ArgTypes&&...), R>::type
    {};

    template <typename R, typename Fn, typename... ArgTypes>
    inline constexpr bool is_nothrow_invocable_r_v =
        is_nothrow_invocable_r<R, Fn, ArgTypes...>::value;

    ///////////////////////////////////////////////////////////////////////////
    template <typename Fn, typename... ArgTypes>
    constexpr invoke_result_t<Fn, ArgTypes...>
    invoke(Fn&& f, ArgTypes&&... args)
        noexcept(is_nothrow_invocable_v<Fn, ArgTypes...>)
    {
        return EGGS_INVOKE(
            std::forward<Fn>(f), std::forward<ArgTypes>(args)...);
    }

    template <typename R, typename Fn, typename... ArgTypes,
        typename Enable = invoke_result_t<Fn, ArgTypes...>>
    constexpr R
    invoke_r(Fn&& f, ArgTypes&&... args)
        noexcept(is_nothrow_invocable_v<Fn, ArgTypes...>)
    {
        return EGGS_INVOKE_R(
            R, std::forward<Fn>(f), std::forward<ArgTypes>(args)...);
    }

}    // namespace eggs

#endif /*EGGS_INVOKE_HPP*/
