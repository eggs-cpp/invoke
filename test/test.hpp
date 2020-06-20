// Eggs.Invoke
//
// Copyright Agustin K-ballo Berge, Fusion Fenix 2020
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef TEST_HPP
#define TEST_HPP

#include <deque>
#include <iostream>

///////////////////////////////////////////////////////////////////////////////
struct test_context
{
    char const* file;
    unsigned line;
    char const* expr;
};

inline std::ostream& operator<<(std::ostream& out, test_context const& context)
{
    return out << context.file << "(" << context.line << "): " << context.expr;
}

///////////////////////////////////////////////////////////////////////////////
template <typename = void>
struct test_scope_stack
{
    static std::deque<test_context> stack;
};

template <typename _>
std::deque<test_context> test_scope_stack<_>::stack;

struct test_scope : test_scope_stack<>
{
    explicit test_scope(test_context const& context)
    {
        stack.push_front(context);
    }

    ~test_scope()
    {
        stack.pop_front();
    }
};

///////////////////////////////////////////////////////////////////////////////
inline void test_check(bool c, test_context context)
{
    if (!c)
    {
        std::cerr << context << " is false\n";
        for (auto const& scope : test_scope::stack)
            std::cerr << "    " << scope << '\n';
        std::cerr << '\n';
    }
}

///////////////////////////////////////////////////////////////////////////////
#define CHECK(...)                                                             \
    (::test_check((__VA_ARGS__), {__FILE__, __LINE__, #__VA_ARGS__}))

#define CHECK_SCOPE(...)                                                       \
    (::test_scope{{__FILE__, __LINE__, #__VA_ARGS__}}, (__VA_ARGS__))

#endif /*TEST_HPP*/
