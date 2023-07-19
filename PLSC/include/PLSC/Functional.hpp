#pragma once

#include "Typedefs.hpp"

#include <array>
#include <functional>
#include <utility>

namespace PLSC
{
    namespace Impl
    {
        template <class F, size_t... Indices>
        static constexpr auto IterateHelper(F &&f, std::index_sequence<Indices...>)
        {
            using T = decltype(std::forward<F>(f)(size_t(0)));
            return std::array<T, sizeof...(Indices)> {(std::forward<F>(f)(Indices))...};
        }
    } // namespace Impl

    /** @Iterate \<N\> -> (n -> x) -> [x] :: Generate a for-like function which outputs xs from indices
     * [0...i] in order. Returns the std::array\<x, N\> = [f(0), ..., f(N-1)]
     *
     * @param f  (i -> x) -> [x0, ..., xi]
     */
    template <size_t N, class F>
    static constexpr auto Iterate(F &&f)
    {
        return Impl::IterateHelper(f, std::make_index_sequence<N>());
    }

    template <class F>
    static constexpr auto Iterate(const size_t N, F &&f)
    {
        using T = decltype(std::forward<F>(f)(size_t(0)));
        std::vector<T> v;
        for (size_t i = 0; i < N; ++i) { v.push_back(std::forward<F>(f)(i)); }
        return v;
    }

    /**
     * @Enable Metaprogramming helper aliases.
     */
    namespace EnableIf
    {
        template <typename... Ts>
        using Arithmetic = typename std::enable_if<((std::is_arithmetic<Ts>::value) && ...), bool>::type;

        template <typename... Ts>
        using Integral = typename std::enable_if<((std::is_integral<Ts>::value) && ...), bool>::type;

        template <typename... Ts>
        using Floating = typename std::enable_if<((std::is_floating_point<Ts>::value) && ...), bool>::type;

        template <class C, class... Cs>
        using Derives = typename std::enable_if<((std::is_base_of<Cs, C>::value) && ...), bool>::type;

        template <class A, class B>
        using Convertible = typename std::enable_if<std::is_convertible<A, B>::value, bool>::type;

        template <class C, class... Cs>
        using Identical = typename std::enable_if<((std::is_same<C, Cs>::value) && ...), bool>::type;

        template <class F, class... Args>
        using Invocable = typename std::enable_if<std::is_invocable<F, Args...>::value, bool>::type;

        /// Any-lookup (... || ...) usable in C++11
        //        template <class X, class... Ys>
        //        struct Any
        //        {
        //        };
        //
        //        template <class X, class Y, class... Ys>
        //        struct Any<X, Y, Ys...>
        //        {
        //            static constexpr bool value = std::is_base_of<X, Y>::value || Any<X, Ys...>::value;
        //        };
        //
        //        template <class X, class Y>
        //        struct Any<X, Y>
        //        {
        //            static constexpr bool value = std::is_base_of<X, Y>::value;
        //        };
    } // namespace EnableIf

      /*!
       * <>A tuple intermediary interface for variadic types which can be wrapped into some monadic M, e.g.
       * a Union-like structure: union {x; y;} or shared_ptr of a virtual baseclass, etc. Allows variad arrays
       * of M x
       * @tparam M Monad type which can wrap all argument parameters Cs, allows you to take any combination of
       * nested tuples of types C in Cs and create an array of M c ... cs
       */
    template <class M>
    class Tuplex
    {
    public:
        /**
         * <> Flatwrap any combination of singles, x y z, tuple-like of singles, [x], or nested tuple-likes,
         * [x, [M y]], into an array of wrapped singles M x
         * @tparam Xs Types that can be wrapped by monad M: forall x in Xs -> M x
         * @param xs any of : x,[x,[x]]
         * @return [M x]
         */
        template <class... Xs>
        static constexpr auto Array(Xs... xs)
        {
            constexpr size_t N = (0 + ... + M_Count<Xs>::value);
            return M_ArrayHelper(Flatten(xs...), std::make_index_sequence<N>());
        }

        /*! Flatten any combination of tuple-like of singles or tuples of (tuples of) singles into a tuple of
         * singles.
         * @param args x,[y,[M z]] -> [M x, M y, M z]
         */
        template <class... Cs>
        static constexpr auto Flatten(Cs... args)
        {
            return std::tuple_cat((M_Flatten(args))...);
        }

        /// @Count Helper to find the flat tuple length before flattening
        template <class... Xs>
        static constexpr size_t Count()
        {
            return (0 + ... + M_Count<Xs>::value);
        }

        template <class Y, class... Xs>
        static constexpr size_t CountSpecific()
        {
            return (0 + ... + M_CountSpecific<Y, Xs>::value);
        }

    private:
        // Helper structs to count instances of the expanded result before expansion.
        template <class... Xs>
        struct M_Count
        {
        };

        template <class... Xs>
        struct M_Count<std::tuple<Xs...>>
        {
            static constexpr size_t value = (M_Count<Xs>::value + ...);
        };

        template <class X, size_t N>
        struct M_Count<std::array<X, N>>
        {
            // Count recurse on X here for nested types array<tuple<x...>>, array<array<x>>, etc
            static constexpr size_t value = M_Count<X>::value * N;
        };

        template <class X>
        struct M_Count<X>
        {
            static constexpr size_t value = 1;
        };

        template <class Y, class... Xs>
        struct M_CountSpecific
        {
        };

        template <class Y, class X>
        struct M_CountSpecific<Y, X>
        {
            static constexpr size_t value = std::is_same<Y, X>::value;
        };

        template <class Y, class... Xs>
        struct M_CountSpecific<Y, std::tuple<Xs...>>
        {
            static constexpr size_t value = (0 + ... + M_CountSpecific<Y, Xs>::value);
        };

        template <class Y, class X, size_t N>
        struct M_CountSpecific<Y, std::array<X, N>>
        {
            static constexpr size_t value = M_CountSpecific<Y, X>::value * N;
        };

    private:
        template <class Tuple, size_t... Indices>
        static constexpr auto M_ArrayHelper(Tuple tuple, std::index_sequence<Indices...>)
        {
            return std::array<M, sizeof...(Indices)> {(std::get<Indices>(tuple))...};
        }

        template <typename T, EnableIf::Convertible<T, M> = true>
        static constexpr auto M_Flatten(T x)
        {
            return std::make_tuple(M(x));
        }

        // These don't work.
        //        template <typename F, Enable::if_invocable<F, void>>
        //        static constexpr auto M_Flatten(F &&f)
        //        {
        //            return M_Flatten(std::forward<F>(f)());
        //        }
        //        template <typename T>
        //        static constexpr auto M_Flatten(std::function<T(void)> f)
        //        {
        //            return M_Flatten(std::invoke(f));
        //        }

        template <class T, size_t N>
        static constexpr auto M_Flatten(std::array<T, N> xs)
        {
            return M_FlattenHelper(xs, std::make_index_sequence<N>());
        }

        template <class... Cs>
        static constexpr auto M_Flatten(std::tuple<Cs...> xs)
        {
            return M_FlattenHelper(xs, std::make_index_sequence<sizeof...(Cs)>());
        }

        template <class C, size_t... Indices>
        static constexpr auto M_FlattenHelper(C tuple, std::index_sequence<Indices...>)
        {
            return std::tuple_cat((M_Flatten(std::get<Indices>(tuple)))...);
        }
    };

} // namespace PLSC

  // constexpr int _f_i(size_t i) { return i + i * i; }
// int           _ff_i(size_t i) { return i + i * i; }

// #include <cmath>
// void f__test()
//{
//     static constexpr auto arr  = PLSC::Iterate<5>([](const size_t i) { return i * i; });
//     static constexpr auto arr1 = PLSC::Iterate<20>([](auto i) { return std::array<int, 5> {1, 2, 3, 4, 5};
//     }); static constexpr auto arr2 = PLSC::Iterate<15>(_f_i); auto                  arr3 =
//     PLSC::Iterate<10>(_ff_i);
//
//     static constexpr PLSC::Number n0  = 2;
//     static constexpr PLSC::Number n1  = -3.1415;
//     static constexpr int          cl0 = PLSC::roundexpr::static_ceil(n0);
//     static constexpr int          cl1 = PLSC::roundexpr::static_ceil(n1);
//     constexpr auto                cs1 = static_cast<intmax_t>(n1);
//     constexpr auto                cs2 = static_cast<PLSC::Number>(cs1) == n1;
//     constexpr auto                cs3 = static_cast<intmax_t>(n1) + ((n1))
// }