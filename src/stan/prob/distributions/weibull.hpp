#ifndef __STAN__PROB__DISTRIBUTIONS__WEIBULL_HPP__
#define __STAN__PROB__DISTRIBUTIONS__WEIBULL_HPP__

#include <stan/prob/traits.hpp>
#include <stan/prob/error_handling.hpp>
#include <stan/prob/constants.hpp>

namespace stan {
  namespace prob {
    using boost::math::tools::promote_args;
    using boost::math::policies::policy;

    // Weibull(y|sigma,alpha)     [y >= 0;  sigma > 0;  alpha > 0]
    template <bool propto = false,
	      typename T_y, typename T_shape, typename T_scale, 
	      class Policy = policy<> >
    inline typename promote_args<T_y,T_shape,T_scale>::type
      weibull_log(const T_y& y, const T_shape& alpha, const T_scale& sigma, const Policy& = Policy()) {
      //static const char* function = "stan::prob::weibull_log<%1%>(%1%)";

      //double result;
      // FIXME: domain checks
      if (y < 0)
	return LOG_ZERO;
      
      using stan::maths::multiply_log;
      
      typename promote_args<T_y,T_shape,T_scale>::type lp(0.0);
      if (include_summand<propto,T_shape>::value)
	lp += log(alpha);
      if (include_summand<propto,T_y,T_shape>::value)
	lp += multiply_log(alpha-1.0, y);
      if (include_summand<propto,T_shape,T_scale>::value)
	lp -= multiply_log(alpha, sigma);
      if (include_summand<propto,T_y,T_shape,T_scale>::value)
	lp -= pow(y / sigma, alpha);
	
      return lp;
    }

    template <bool propto = false,
	      typename T_y, typename T_shape, typename T_scale, 
	      class Policy = policy<> >
    inline typename boost::math::tools::promote_args<T_y,T_shape,T_scale>::type
      weibull_p(const T_y& y, const T_shape& alpha, const T_scale& sigma, const Policy& = Policy()) {
      //static const char* function = "stan::prob::weibull_p<%1%>(%1%)";

      //double result;
      // FIXME: domain checks
      
      if (y < 0)
	return 0;
      if (include_summand<propto,T_y,T_shape,T_scale>::value)
	return 1.0 - exp (- pow (y / sigma, alpha));
      return 1;
    }

  }
}
#endif