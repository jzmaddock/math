// Copyright 2020 Matt Borland
//
// Use, modification and distribution are subject to the
// Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_MATH_SPECIAL_FUNCTIONS_PRIME_SIEVE_HPP
#define BOOST_MATH_SPECIAL_FUNCTIONS_PRIME_SIEVE_HPP

#include <boost/math/special_functions/prime.hpp>
#include <boost/math/special_functions/interval_sieve.hpp>
#include <boost/assert.hpp>
#include <vector>
#include <iterator>
#include <cmath>
#include <thread>
#include <memory>
#include <future>
#include <numeric>
#include <iostream>
#include <algorithm>
#include <execution>

namespace boost::math { namespace detail
{
// https://mathworld.wolfram.com/SieveofEratosthenes.html
// https://www.cs.utexas.edu/users/misra/scannedPdf.dir/linearSieve.pdf
template<class Integer, class Container>
void linear_sieve(Integer upper_bound, Container &resultant_primes)
{
    std::size_t least_divisors_size{static_cast<std::size_t>(upper_bound + 1)};
    std::unique_ptr<Integer[]> least_divisors{new Integer[least_divisors_size]{0}};

    for (std::size_t i{2}; i < upper_bound; ++i)
    {
        if (least_divisors[i] == 0)
        {
            least_divisors[i] = i;
            resultant_primes.emplace_back(i);
        }

        for (std::size_t j{}; j < resultant_primes.size() && i * resultant_primes[j] <= upper_bound && 
             resultant_primes[j] <= least_divisors[i] && j < least_divisors_size; ++j)
        {
            least_divisors[i * static_cast<std::size_t>(resultant_primes[j])] = resultant_primes[j];
        }
    }
}

template<class Integer, class PrimeContainer, class Container>
void mask_sieve(Integer lower_bound, Integer upper_bound, const PrimeContainer& primes, Container &resultant_primes)
{
    Integer limit {static_cast<Integer>(std::floor(std::sqrt(static_cast<double>(upper_bound)))) + 1};

    std::size_t primes_size {};
    auto it{primes.begin()};
    while(it != primes.end() && *it < limit)
    {
        ++primes_size;
        ++it;
    }

    const std::size_t n {static_cast<std::size_t>(upper_bound - lower_bound + 1)};
    std::unique_ptr<bool[]> is_prime {new bool[n]};
    memset(is_prime.get(), true, sizeof(*is_prime.get()) * (n));
     
    // Enable use of thread pool, not SIMD compatible
    std::for_each(std::execution::par, primes.begin(), it, [&is_prime, lower_bound, upper_bound](auto prime){
        for(Integer j {std::max(prime * prime, (lower_bound + prime - 1) / prime * prime)}; j < upper_bound; j += prime)
        {
            is_prime[j - lower_bound] = false;
        }
    });

    if(lower_bound == 1)
    {
        is_prime[0] = false;
    }

    for(Integer i{lower_bound}; i <= upper_bound; ++i)
    {
        if(is_prime[i - lower_bound])
        {
            resultant_primes.emplace_back(i);
        }
    }
}

template<class Integer, class Container>
void mask_sieve(Integer lower_bound, Integer upper_bound, Container &resultant_primes)
{
    Integer limit{static_cast<Integer>(std::floor(std::sqrt(static_cast<double>(upper_bound)))) + 1};
    std::vector<Integer> primes {};
    primes.reserve(limit / std::log(limit));

    boost::math::detail::linear_sieve(limit, primes);

    boost::math::detail::mask_sieve(lower_bound, upper_bound, primes, resultant_primes);
}

template<class Integer, class Container>
constexpr void prime_table(std::size_t min_index, Integer upper_bound, Container &resultant_primes)
{
    std::size_t current_index {min_index};
    Integer current_prime {2};

    while(current_prime < upper_bound)
    {
        resultant_primes.emplace_back(current_prime);
        ++current_index;
        current_prime = prime(current_index);
    }
}

template<class Integer, class Container>
constexpr void prime_table(Integer upper_bound, Container &resultant_primes)
{
    prime_table(0, upper_bound, resultant_primes);
}

template<class Integer, class PrimesContainer, class Container>
void segmented_sieve(Integer lower_bound, Integer upper_bound, const PrimesContainer &primes, Container &resultant_primes)
{
    const Integer L1_SIZE {32768};
    const Integer interval {L1_SIZE * 8};
    Integer current_lower_bound{lower_bound};
    Integer current_upper_bound{current_lower_bound + interval};

    if(current_upper_bound > upper_bound)
    {
        current_upper_bound = upper_bound;
    }

    std::size_t ranges {static_cast<std::size_t>((upper_bound - lower_bound) / interval)};

    std::vector<std::vector<Integer>> prime_vectors(ranges + 1);
    std::vector<std::future<void>> future_manager(ranges);

    Integer primes_in_range {static_cast<Integer>(current_upper_bound / std::log(static_cast<double>(current_upper_bound)) -
                             current_lower_bound / std::log(static_cast<double>(current_lower_bound)))};

    for(std::size_t i {}; i < ranges; ++i)
    {
        prime_vectors[i].reserve(primes_in_range);

        future_manager.emplace_back(std::async(std::launch::async, [current_lower_bound, current_upper_bound, &primes, &prime_vectors, i]{
            boost::math::detail::IntervalSieve sieve(current_lower_bound, current_upper_bound, primes, prime_vectors[i]);
        }));

        current_lower_bound = current_upper_bound;
        current_upper_bound += interval;
    }

    prime_vectors[ranges].reserve(primes_in_range);
    future_manager.emplace_back(std::async(std::launch::async, [current_lower_bound, upper_bound, &primes, &prime_vectors]{
        boost::math::detail::IntervalSieve sieve(current_lower_bound, upper_bound, primes, prime_vectors.back());
    }));

    for(auto &&future : future_manager)
    {
        if(future.valid())
        {
            future.get();
        }
    }

    for(auto &v : prime_vectors)
    {
        resultant_primes.insert(resultant_primes.end(), v.begin(), v.end());
    }
}

template<class Integer, class Container>
void segmented_sieve(Integer lower_bound, Integer upper_bound, Container &resultant_primes)
{
    Integer limit{static_cast<Integer>(std::floor(std::sqrt(static_cast<double>(upper_bound)))) + 1};
    std::vector<Integer> primes {};
    primes.reserve(limit / std::log(limit));

    // Prepare for max value so you do not have to calculate this again
    if(limit < 4096)
    {
        boost::math::detail::linear_sieve(limit, primes);
    }

    else
    {
        boost::math::detail::mask_sieve(static_cast<Integer>(2), limit, primes);
    }

    boost::math::detail::segmented_sieve(lower_bound, upper_bound, primes, resultant_primes);
}

template<class Integer, class Container>
void sequential_segmented_sieve(Integer lower_bound, Integer upper_bound, Container &resultant_primes)
{
    const Integer L1_SIZE {32768};
    const Integer interval {L1_SIZE * 8};
    Integer current_lower_bound{lower_bound};
    Integer current_upper_bound{current_lower_bound + interval};

    if(current_upper_bound > upper_bound)
    {
        current_upper_bound = upper_bound;
    }

    std::size_t ranges {static_cast<std::size_t>((upper_bound - lower_bound) / interval)};

    boost::math::detail::IntervalSieve sieve(current_lower_bound, current_upper_bound, resultant_primes, resultant_primes);
    if(ranges == 0)
    {
        return;
    }

    for(std::size_t i {}; i < ranges; ++i)
    {
        current_lower_bound = current_upper_bound;
        current_upper_bound += interval;
        if(current_upper_bound > upper_bound)
        {
            current_upper_bound = upper_bound;
        }
        sieve.NewRange(current_lower_bound, current_upper_bound, resultant_primes);
    }
}
} // End namespace detail

template<class Integer>
constexpr void prime_reserve(Integer upper_bound, std::vector<Integer> &prime_container)
{
    prime_container.reserve(upper_bound / std::log(static_cast<double>(upper_bound)));
}

template<class ExecutionPolicy, class Integer, class Container>
void prime_sieve(ExecutionPolicy&& policy, Integer upper_bound, Container &primes)
{
    static_assert(std::is_integral<Integer>::value, "No primes for floating point types");
    BOOST_ASSERT_MSG(upper_bound + 1 < std::numeric_limits<Integer>::max(), "Type Overflow");

    if(upper_bound == 2)
    {
        return;
    }

    if(upper_bound <= 4096)
    {
        boost::math::detail::linear_sieve(upper_bound, primes);
    }

    else if(typeid(policy) == typeid(std::execution::seq))
    {
        boost::math::detail::linear_sieve(static_cast<Integer>(4096), primes);
        boost::math::detail::sequential_segmented_sieve(static_cast<Integer>(4096), upper_bound, primes);
    }

    else
    {
        std::vector<Integer> small_primes {};
        small_primes.reserve(1028);

        std::thread t1([&small_primes] {
            boost::math::detail::linear_sieve(static_cast<Integer>(8192), small_primes);
        });
        std::thread t2([upper_bound, &primes] {
            boost::math::detail::segmented_sieve(static_cast<Integer>(8192), upper_bound, primes);
        });

        t1.join();
        t2.join();
        primes.insert(primes.begin(), small_primes.begin(), small_primes.end());
    }
}

template<class Integer, class Container>
void prime_sieve(Integer upper_bound, Container &primes)
{
    prime_sieve(std::execution::seq, upper_bound, primes);
}


template<class ExecutionPolicy, class Integer, class Container>
void prime_range(ExecutionPolicy&& policy, Integer lower_bound, Integer upper_bound, Container &primes)
{
    boost::math::prime_sieve(policy, upper_bound, primes);

    auto it{primes.begin()};
    while(*it < lower_bound && it != primes.end())
    {
        ++it;
    }

    primes.erase(primes.begin(), it);
}

template<class Integer, class Container>
inline void prime_range(Integer lower_bound, Integer upper_bound, Container &primes)
{
    prime_range(std::execution::seq, lower_bound, upper_bound, primes);
}
}

#endif //BOOST_MATH_SPECIAL_FUNCTIONS_PRIME_SIEVE_HPP
