// Eggs.Invoke
//
// Copyright Agustin K-ballo Berge, Fusion Fenix 2020
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef TEST_HPP
#define TEST_HPP

#include <cstdlib>
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
template <typename = void>
struct test_report_info
{
    static unsigned checks;
    static unsigned failures;
};

template <typename _>
unsigned test_report_info<_>::checks = 0;
template <typename _>
unsigned test_report_info<_>::failures = 0;

inline void test_check(bool c, test_context context)
{
    ++test_report_info<>::checks;
    if (!c)
    {
        ++test_report_info<>::failures;

        std::cerr << context << " is false\n";
        for (auto const& scope : test_scope::stack)
            std::cerr << "    " << scope << '\n';
        std::cerr << '\n';
    }
}

inline int test_report()
{
    unsigned const checks = test_report_info<>::checks;
    unsigned const failures = test_report_info<>::failures;
    if (failures)
    {
        std::cout << failures << " out of " << checks << " checks failed.\n";
        return EXIT_FAILURE;
    } else {
        std::cout << "All " << checks << " checks succeeded.\n";
        return EXIT_SUCCESS;
    }
}

///////////////////////////////////////////////////////////////////////////////
#define CHECK(...)                                                             \
    (::test_check((__VA_ARGS__), {__FILE__, __LINE__, #__VA_ARGS__}))

#define CHECK_SCOPE(...)                                                       \
    (::test_scope{{__FILE__, __LINE__, #__VA_ARGS__}}, (__VA_ARGS__))

#endif /*TEST_HPP*/
