//  Copyright John Maddock 2012

//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <pch_light.hpp>
#include <boost/math/special_functions/jacobi.hpp>
#include <boost/multiprecision/cpp_bin_float.hpp>

#define SC_(x) static_cast<typename table_type<T>::type>(BOOST_STRINGIZE(x))
#define TESTING_MP
#include <test_jacobi.hpp>

//
// DESCRIPTION:
// ~~~~~~~~~~~~
//
// This file tests the Jacobi Elliptic Functions.
// There are two sets of tests, spot
// tests which compare our results with selected values computed
// using the online special function calculator at
// functions.wolfram.com, while the bulk of the accuracy tests
// use values generated with NTL::RR at 1000-bit precision
// and our generic versions of these functions.
//
// Note that when this file is first run on a new platform many of
// these tests will fail: the default accuracy is 1 epsilon which
// is too tight for most platforms.  In this situation you will
// need to cast a human eye over the error rates reported and make
// a judgement as to whether they are acceptable.  Either way please
// report the results to the Boost mailing list.  Acceptable rates of
// error are marked up below as a series of regular expressions that
// identify the compiler/stdlib/platform/data-type/test-data/test-function
// along with the maximum expected peek and RMS mean errors for that
// test.
//

void expected_results()
{
   //
   // Define the max and mean errors expected for
   // various compilers and platforms.
   //
   //
   // Catch all cases come last:
   //
   add_expected_result(
      ".*",                          // compiler
      ".*",                          // stdlib
      ".*",                          // platform
      "cpp_bin_float_quad",          // test type(s)
      ".*near 1.*",               // test data group
      ".*", 7000, 500);            // test function
   add_expected_result(
      ".*",                          // compiler
      ".*",                          // stdlib
      ".*",                          // platform
      "cpp_bin_float_quad",          // test type(s)
      ".*Large Phi.*",               // test data group
      ".*", 50000, 5000);            // test function
   add_expected_result(
      ".*",                          // compiler
      ".*",                          // stdlib
      ".*",                          // platform
      "cpp_bin_float_quad",          // test type(s)
      ".*Mathworld.*",               // test data group
      ".*", 100, 40);                 // test function
   add_expected_result(
      ".*",                          // compiler
      ".*",                          // stdlib
      ".*",                          // platform
      "cpp_bin_float_quad",          // test type(s)
      ".*",                          // test data group
      ".*", 50, 20);                 // test function
   //
   // Finish off by printing out the compiler/stdlib/platform names,
   // we do this to make it easier to mark up expected error rates.
   //
   std::cout << "Tests run with " << BOOST_COMPILER << ", "
      << BOOST_STDLIB << ", " << BOOST_PLATFORM << std::endl;
}


BOOST_AUTO_TEST_CASE( test_main )
{
   expected_results();
   test_spots(boost::multiprecision::cpp_bin_float_quad(0), "cpp_bin_float_quad");
}
