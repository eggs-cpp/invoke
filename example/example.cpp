// Eggs.Invoke
//
// Copyright Agustin K-ballo Berge, Fusion Fenix 2020
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <eggs/invoke.hpp>

#include <iostream>
#include <string>
#include <utility>
#include <vector>

/// identity function
struct identity
{
    template <typename T>
    constexpr T&& operator()(T&& t) const noexcept
    {
        return std::forward<T>(t);
    }
};

/// algorithm that prints to `std::cout` the projection of those elements in
/// the range [iter, sentinel) that satisfy a given predicate
template <typename Iter, typename Sentinel, typename Pred, typename Proj = identity>
void print_if(Iter iter, Sentinel sentinel, Pred const& pred, Proj const& proj = {})
{
    /// Check that `INVOKE(pred, *iter)` is well-formed,
    static_assert(eggs::is_invocable<Pred const&, decltype(*iter)>::value,
        "Predicate shall accept the iterator's value type as its only argument");

    /// and that it returns something convertible to `bool`;
    static_assert(std::is_convertible<
        eggs::invoke_result_t<Pred const&, decltype(*iter)>, bool>::value,
        "Predicate return type shall be convertible to bool");

    /// ...or check both requirements at once:
    /// Check that `INVOKE<bool>(pred, *iter)` is well-formed.
    static_assert(eggs::is_invocable_r<bool, Pred const&, decltype(*iter)>::value,
        "Predicate shall accept the iterator's value type as its only argument, "
        "and its return type shall be convertible to bool");

    /// Also check that `INVOKE(proj, *iter)` is well-formed,
    static_assert(eggs::is_invocable<Proj const&, decltype(*iter)>::value,
        "Projection shall accept the iterator's value type as its only argument");

#if __cpp_generic_lambdas
    /// and that it returns something "printable" to `std::cout`.
    auto printable = [](auto const& v) -> decltype((void)(std::cout << v)) {};
    static_assert(eggs::is_invocable<decltype(printable),
        eggs::invoke_result_t<Proj const&, decltype(*iter)>>::value,
        "Projection return type shall be printable to std::cout");
#endif

    for (; iter != sentinel; ++iter)
    {
        if (eggs::invoke(pred, *iter)) // or eggs::invoke_r<bool>(pred, *iter)
            std::cout << eggs::invoke(proj, *iter) << '\n';
    }
}

/// user definition
struct User
{
    std::string name;

    bool is_superuser;

    constexpr bool is_regular_user() const noexcept { return !is_superuser; }

    friend std::ostream& operator<<(std::ostream& out, User const& user)
    {
        return out << '('
            << "name: " << user.name
            << ", is_superuser: " << user.is_superuser
            << ')';
    }
};

/// table of users
std::vector<User> const users = {
    { "Alice", true },
    { "Bob", false },
    { "Charlie", false }
};

int main()
{
    /// print all users
    ///
    /// `eggs::invoke(<lambda>, user)` is equivalent to `<lambda>(user)`.
    std::cout << "Users:\n";
    print_if(users.begin(), users.end(), [](User const&) { return true; });
    std::cout << '\n';

    /// print the names of superusers
    ///
    /// `eggs::invoke(&User::is_superuser, user)` is equivalent to
    /// `user.is_superuser` or `user->is_superuser`.
    std::cout << "Superusers:\n";
    print_if(users.begin(), users.end(), &User::is_superuser, &User::name);
    std::cout << '\n';

    /// print the names of regular users
    ///
    /// `eggs::invoke(&User::is_regular_user, user)` is equivalent to
    /// `user.is_regular_user()` or `user->is_regular_user()`.
    std::cout << "Regular users:\n";
    print_if(users.begin(), users.end(), &User::is_regular_user, &User::name);
    std::cout << '\n';

    return 0;
}
