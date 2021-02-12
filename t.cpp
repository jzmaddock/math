#include <boost/math/concepts/real_concept.hpp>
#include <boost/math/special_functions.hpp>
#include <boost/multiprecision/float128.hpp>


int main()
{
   using mp_t = boost::float128_t;
   using mp_t2 = boost::math::concepts::real_concept;

   double a(279777.4375), b(110919.8125), x(0.632396042346954345703125);

   std::cout << std::setprecision(35);
   std::cout << boost::math::ibeta_derivative(mp_t(a), mp_t(b), mp_t(x)) << std::endl;
   std::cout << boost::math::ibeta_derivative(mp_t2(a), mp_t2(b), mp_t2(x)) << std::endl;
}
