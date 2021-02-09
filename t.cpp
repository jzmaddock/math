
#define BOOST_MATH_INSTRUMENT

#include <boost/math/special_functions.hpp>


int main()
{
   double x = 11.791534423828125;
   long double r = -3.53017140778223781420794006033810387155048392363051866610931e-9L;

   double r1 = boost::math::cyl_bessel_j(0, x);
   long double r2 = boost::math::cyl_bessel_j(0, (long double)x);

   std::cout << boost::math::relative_difference(r1, (double)r) << std::endl;
   std::cout << boost::math::relative_difference(r2, r) << std::endl;

}
