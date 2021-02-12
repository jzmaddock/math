#define BOOST_MATH_INSTRUMENT
#include <boost/math/concepts/real_concept.hpp>
#include <boost/math/special_functions.hpp>


int main()
{
   using mp_t2 = boost::math::concepts::real_concept;

   double a(279777.4375), b(110919.8125), x(0.632396042346954345703125);

   std::cout << std::setprecision(35);
   std::cout << boost::math::ibeta(mp_t2(a), mp_t2(b), mp_t2(x)) << std::endl;
}
