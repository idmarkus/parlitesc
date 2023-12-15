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

    /// ###########################################
    /// # Tuple stuff                             #
    /// ###########################################

    /// @Cons X, [Xs] -> [X, Xs]
    template <class, class>
    struct Cons;

    template <class C, class... Cs>
    struct Cons<C, std::tuple<Cs...>>
    {
        using T = std::tuple<C, Cs...>;
    };

    /// @Pop [X, Xs...] -> X, [Xs]
    template <class>
    struct Pop;

    template <class X, class... Xs>
    struct Pop<std::tuple<X, Xs...>>
    {
        using Head = X;
        using Tail = std::tuple<Xs...>;
    };

    /// @Elem Is X in [Xs...]
    template <class X, class Ys>
    struct Elem;

    template <class X, class Y, class... Ys>
    struct Elem<X, std::tuple<Y, Ys...>>
    {
        static constexpr bool value = std::is_same_v<X, Y> || Elem<X, std::tuple<Ys...>>::value;
    };

    template <class X, class Y>
    struct Elem<X, std::tuple<Y>>
    {
        static constexpr bool value = std::is_same_v<X, Y>;
    };

    template <class X>
    struct Elem<X, std::tuple<>>
    {
        static constexpr bool value = false;
    };

    template <class... Cs>
    struct Unique;

    template <>
    struct Unique<>
    {
        using T = std::tuple<>;
    };

    template <class Y, class... Xs, class... Ys>
    struct Unique<std::tuple<Xs...>, std::tuple<Y, Ys...>>
    {
        using T = std::conditional<Elem<Y, std::tuple<Xs...>>::value,
                                   typename Unique<std::tuple<Xs...>, std::tuple<Ys...>>::T,
                                   typename Unique<std::tuple<Y, Xs...>, std::tuple<Ys...>>::T>::type;
    };

    template <class... Xs>
    struct Unique<std::tuple<Xs...>, std::tuple<>>
    {
        using T = std::tuple<Xs...>;
    };

    template <class... Cs>
    struct Unique<const std::tuple<Cs...>>
    {
        using T = Unique<std::tuple<>, std::tuple<Cs...>>::T;
    };
    template <class... Cs>
    struct Unique<std::tuple<Cs...>>
    {
        using T = Unique<std::tuple<>, std::tuple<Cs...>>::T;
    };

    template <class, class>
    struct Count;

    template <class X, class... Ys>
    struct Count<X, const std::tuple<Ys...>>
    {
        static constexpr size_t value = Count<X, std::tuple<Ys...>>::value;
    };

    template <class X, class Y, class... Ys>
    struct Count<X, std::tuple<Y, Ys...>>
    {
        static constexpr size_t value = (std::is_same_v<X, Y> ? 1 : 0) + Count<X, std::tuple<Ys...>>::value;
    };

    template <class X>
    struct Count<X, std::tuple<>>
    {
        static constexpr size_t value = 0;
    };

    template <class X, auto y>
    struct Zipr
    {
        using T                     = X;
        static constexpr auto value = y;
    };

    template <class X, class Tup = std::tuple<>>
    struct ZipCount
    {
        using T                       = X;
        static constexpr size_t value = Count<X, Tup>::value;
    };

    template <class X, class Tup>
    inline constexpr bool elem = Elem<X, Tup>::value;

    template <class X, class Tuple>
    static constexpr size_t count = Count<X, Tuple>::value;

    template <class X, class Tuple>
    struct ArrayBase
    {
        static constexpr size_t N = count<X, Tuple>;
    };

    template <class...>
    struct Arrayplex;

    template <class Tuple, class... Unique>
    struct Arrayplex<Tuple, std::tuple<Unique...>> : ArrayBase<Unique, Tuple>...
    {
        template <class X>
        inline static constexpr size_t N = ArrayBase<X, Tuple>::N;
    };

    template <class... Xs>
    struct Arrayplex<const std::tuple<Xs...>>
        : Arrayplex<std::tuple<Xs...>, typename Unique<std::tuple<Xs...>>::T>
    {};

    static constexpr void testunique()
    {
        constexpr auto tup = std::make_tuple(1, 2.0, 1.2f, false, false);
        using unq          = Unique<decltype(tup)>::T;

        constexpr size_t cint = Count<bool, decltype(tup)>::value;

        static_assert(std::is_same_v<unq, std::tuple<int, double, float, bool>>);
        constexpr unq t2;

        using wrap          = Arrayplex<decltype(tup)>;
        constexpr auto hasb = wrap::N<bool>;
    }

    /*!
     * <>A tuple intermediary interface for variadic types which can be wrapped into some monadic M, e.g.
     * a Union-like structure: union {x; y;} or shared_ptr of a virtual baseclass, etc. Allows variad arrays
     * of M x
     * @tparam M Monad type which can wrap all argument parameters Cs, allows you to take any combination of
     * nested tuples of types C in Cs and create an array of M c ... cs
     */
    class Tuplex
    {
    public:
        /*! Flatten any combination of tuple-like of singles or tuples of (tuples of) singles into a tuple of
         * singles.
         * @param args x,[y,[M z]] -> [M x, M y, M z]
         */
        template <class... Cs>
        static constexpr auto Flatten(Cs... args)
        {
            return std::tuple_cat((M_Flatten(args))...);
        }

    public:
        /**
         * <> Flatwrap any combination of singles, x y z, tuple-like of singles, [x], or nested tuple-likes,
         * [x, [M y]], into an array of wrapped singles M x
         * @tparam Xs Types that can be wrapped by monad M: forall x in Xs -> M x
         * @param xs any of : x,[x,[x]]
         * @return [M x]
         */
        template <class M, class... Xs>
        static constexpr auto Array(Xs... xs)
        {
            constexpr size_t N = (0 + ... + M_Count<Xs>::value);
            return M_ArrayHelper<M>(Flatten(xs...), std::make_index_sequence<N>());
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
        template <class M, class Tuple, size_t... Indices>
        static constexpr auto M_ArrayHelper(Tuple tuple, std::index_sequence<Indices...>)
        {
            return std::array<M, sizeof...(Indices)> {(M(std::get<Indices>(tuple)))...};
        }

        template <typename T>
        static constexpr auto M_Flatten(T x)
        {
            return std::make_tuple(x);
        }

        template <std::invocable F>
        static constexpr auto M_Flatten(F f)
        {
            return M_Flatten(f());
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

    template <typename Arg>
    using Fun = void (*)(Arg);

    template <typename Arg>
    static constexpr void Nop(Arg _)
    {}

    template <typename T, Fun<T>... fs>
    struct ComposeVoid
    {
        static constexpr void F(T x) { ((fs(x)), ...); }
    };

    //    namespace Impl
    //    {
    //        template <typename T, Fun<T>... Fs>
    //        struct Composer
    //        {
    //        };
    //
    //        template <typename T, Fun<T> Fn, Fun<T>... Fs>
    //        struct Composer<T, Fn, Fs...>
    //        {
    //            static constexpr T F(T x) { return Fn(Composer<T, Fs...>::F(x)); }
    //        };
    //
    //        template <typename T, Fun<T> Fn>
    //        struct Composer<T, Fn>
    //        {
    //            static constexpr T F(T x) { return Fn(x); }
    //        };
    //    } // namespace Impl
    //
    //    template <typename T, Fun<T>... Fs>
    //    struct Composer
    //    {
    //        static constexpr auto F = Impl::Composer<T, Fs...>::F;
    //
    //        constexpr T operator()(T x) const { return F(x); }
    //    };

    //    template <typename T, Fun<T> Fn>
    //    struct Functor
    //    {
    //        static constexpr Fun<T> F = Fn;
    //
    //        template <Fun<T> Fn2>
    //        constexpr auto operator<<(Functor<T, Fn2> vf) const
    //        {
    //            return Functor<T, Impl::Composer<T, F, vf.F>::F>();
    //        }
    //
    //        constexpr T operator<<(const T &p) const { return F(p); }
    //
    //        constexpr T operator()(T p) const { return F(p); }
    //        constexpr operator Fun<T>() const { return F; }
    //
    //        //        constexpr Func<T, > operator<<(const Func<T, F> f) const { return F }
    //    };

    //    static constexpr int Pval(int p) { return p + 2; }
    //    static void          test()
    //    {
    //        constexpr int v = 0;
    //        constexpr int b = Pval(v);
    //
    //        constexpr Functor<int, Pval>     f;
    //        constexpr Composer<int, Pval, f> comp;
    //        constexpr int                    cv = comp(v);
    //
    //        constexpr int c = f << f << v;
    //
    //        constexpr auto cm  = f << f << f;
    //        constexpr auto cmv = cm(v);
    //
    //        static constexpr std::array<const Fun<int>, 5> farr
    //            = {f, (f << f), (f << f << f), (f << f << f << f), (f << f << f << f << f)};
    //
    //        constexpr auto fav = farr[4](v);
    //    }

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