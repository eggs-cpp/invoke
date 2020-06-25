**Eggs.Invoke**
==================

## Introduction ##

**Eggs.Invoke** is a **C++11/14/17/20** single-header implementation of
`INVOKE` and its related facilities.

## Reference ##

#### Definitions

Define `INVOKE(f, t1, t2, ..., tN)` as follows:

- `(t1.*f)(t2, ..., tN)` when `f` is a pointer to a member function of a
  class `T` and `std::is_base_of_v<T, std::remove_reference_t<decltype(t1)>>`
  is `true`;

- `(t1.get().*f)(t2, ..., tN)` when `f` is a pointer to a member function of a
  class `T` and `std::remove_cvref_t<decltype(t1)>` is a specialization of
  `std::reference_wrapper`;

- `((*t1).*f)(t2, ..., tN)` when `f` is a pointer to a member function of a
  class `T` and `t1` does not satisfy the previous two items;

- `t1.*f` when `N == 1` and `f` is a pointer to data member of a class `T` and
  `std::is_base_of_v<T, std::remove_reference_t<decltype(t1)>>` is `true`;

- `t1.get().*f` when `N == 1` and `f` is a pointer to data member of a class
  `T` and `std::remove_cvref_t<decltype(t1)>` is a specialization of
  `std::reference_wrapper`;

- `(*t1).*f` when `N == 1` and `f` is a pointer to data member of a class `T`
  and `t1` does not satisfy the previous two items;

- `f(t1, t2, ..., tN)` in all other cases.

Define `INVOKE<R>(f, t1, t2, ..., tN)` as `static_cast<void>(INVOKE(f, t1, t2,
..., tN))` if `R` is _cv_ `void`, otherwise `INVOKE(f, t1, t2, ..., tN)`
implicitly converted to `R`.

#### Function template `eggs::invoke`

```cpp
template <class F, class... Args>
constexpr eggs::invoke_result_t<F, Args...> invoke(F&& f, Args&&... args)
    noexcept(eggs::is_nothrow_invocable_v<F, Args...>);
```

- _Returns_: `INVOKE(std::forward<F>(f), std::forward<Args>(args)...)`.

- _Remarks_: This function shall not participate in overload resolution unless
  `eggs::is_invocable_v<F, Args...>` is `true`.

#### Function template `eggs::invoke_r`

```cpp
template <class R, class F, class... Args> // (extension)
constexpr R eggs::invoke_r(F&& f, Args&&... args)
    noexcept(eggs::is_nothrow_invocable_r_v<R, F, Args...>);
```

- _Returns_: `INVOKE<R>(std::forward<F>(f), std::forward<Args>(args)...)`.

- _Remarks_: This function shall not participate in overload resolution unless
  `eggs::is_invocable_r_v<R, F, Args...>` is `true`.

#### Transformation Trait `eggs::invoke_result`

```cpp
template <class Fn, class... ArgTypes> struct invoke_result;
```

- _Comments_: If the expression `INVOKE(std::declval<Fn>(),
  std::declval<ArgTypes>()...)` is well-formed when treated as an unevaluated
  operand, the member typedef `type` names the type `decltype(INVOKE(std::declval<Fn>(),
  std::declval<ArgTypes>()...))`; otherwise, there shall be no member `type`.
  Access checking is performed as if in a context unrelated to `Fn` and
  `ArgTypes`. Only the validity of the immediate context of the expression is
  considered.

- _Preconditions_: `Fn` and all types in the template parameter pack `ArgTypes`
  are complete types, _cv_ `void`, or arrays of unknown bound.

```cpp
template <class Fn, class... ArgTypes>
using invoke_result_t = typename invoke_result<Fn, ArgTypes...>::type;
```

#### Unary Type Traits `eggs::is_invocable`

```cpp
template <class Fn, class... ArgTypes> struct is_invocable;
```

- _Condition_: The expression `INVOKE(std::declval<Fn>(),
  std::declval<ArgTypes>()...)` is well-formed when treated as an unevaluated
  operand.

- _Comments_: `Fn` and all types in the template parameter pack `ArgTypes`
  shall be complete types, _cv_ `void`, or arrays of unknown bound.

```cpp
template <class Fn, class... ArgTypes> // (C++14)
inline constexpr bool is_invocable_v =
    eggs::is_invocable<Fn, ArgTypes...>::value;
```

#### Unary Type Traits `eggs::is_invocable_r`

```cpp
template <class R, class Fn, class... ArgTypes> struct is_invocable_r;
```

- _Condition_: The expression `INVOKE<R>(std::declval<Fn>(),
  std::declval<ArgTypes>()...)` is well-formed when treated as an unevaluated
  operand.

- _Comments_: `Fn`, `R`, and all types in the template parameter pack
  `ArgTypes` shall be complete types, _cv_ `void`, or arrays of unknown bound.

```cpp
template <class R, class Fn, class... ArgTypes> // (C++14)
inline constexpr bool is_invocable_r_v =
    eggs::is_invocable_r<R, Fn, ArgTypes...>::value;
```

#### Unary Type Traits `eggs::is_nothrow_invocable`

```cpp
template <class Fn, class... ArgTypes> struct is_nothrow_invocable;
```

- _Condition_: `eggs::is_invocable_v<Fn, ArgTypes...>` is `true` and the
  expression `INVOKE(std::declval<Fn>(), std::declval<ArgTypes>()...)` is
  known not to throw any exceptions.

- _Comments_: `Fn` and all types in the template parameter pack `ArgTypes`
  shall be complete types, _cv_ `void`, or arrays of unknown bound.

```cpp
template <class Fn, class... ArgTypes> // (C++14)
inline constexpr bool is_nothrow_invocable_v =
    eggs::is_nothrow_invocable<Fn, ArgTypes...>::value;
```

#### Unary Type Traits `eggs::is_nothrow_invocable_r`

```cpp
template <class R, class Fn, class... ArgTypes> struct is_nothrow_invocable_r;
```

- _Condition_: `eggs::is_invocable_r_v<R, Fn, ArgTypes...>` is `true` and the
  expression `INVOKE<R>(std::declval<Fn>(), std::declval<ArgTypes>()...)` is
  known not to throw any exceptions.

- _Comments_: `Fn`, `R`, and all types in the template parameter pack
  `ArgTypes` shall be complete types, _cv_ `void`, or arrays of unknown bound.

```cpp
template <class R, class Fn, class... ArgTypes> // (C++14)
inline constexpr bool is_nothrow_invocable_r_v =
    eggs::is_nothrow_invocable_r<R, Fn, ArgTypes...>::value;
```

## CI Status ##

- C++11: ![](https://github.com/eggs-cpp/invoke/workflows/Eggs.Invoke%20C++11/badge.svg?branch=master)
- C++14: ![](https://github.com/eggs-cpp/invoke/workflows/Eggs.Invoke%20C++14/badge.svg?branch=master)
- C++17: ![](https://github.com/eggs-cpp/invoke/workflows/Eggs.Invoke%20C++17/badge.svg?branch=master)
- C++20: ![](https://github.com/eggs-cpp/invoke/workflows/Eggs.Invoke%20C++20/badge.svg?branch=master)

---

> Copyright _Agustín Bergé_, _Fusion Fenix_ 2017-2020
> 
> Distributed under the Boost Software License, Version 1.0. (See accompanying
> file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
