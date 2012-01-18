#ifndef __STAN__PROB__DISTRIBUTIONS__EXPONENTIAL_HPP__
#define __STAN__PROB__DISTRIBUTIONS__EXPONENTIAL_HPP__

#include <stan/prob/traits.hpp>
#include <stan/prob/error_handling.hpp>
#include <stan/prob/constants.hpp>

namespace stan {
  namespace prob {
    using boost::math::tools::promote_args;
    using boost::math::policies::policy;

    /**
     * The log of an exponential density for y with the specified
     * inverse scale parameter.
     * Inverse scale parameter must be greater than 0.
     * y must be greater than or equal to 0.
     * 
     \f{eqnarray*}{
       y &\sim& \mbox{\sf{Expon}}(\beta) \\
       \log (p (y \,|\, \beta) ) &=& \log \left( \beta \exp^{-\beta y} \right) \\
       &=& \log (\beta) - \beta y \\
       & & \mathrm{where} \; y > 0
     \f}
     * @param y A scalar variable.
     * @param beta Inverse scale parameter.
     * @throw std::domain_error if beta is not greater than 0.
     * @throw std::domain_error if y is not greater than or equal to 0.
     * @tparam T_y Type of scalar.
     * @tparam T_inv_scale Type of inverse scale.
     */
    template <bool propto = false,
              typename T_y, typename T_inv_scale, 
              class Policy = policy<> >
    inline typename promote_args<T_y,T_inv_scale>::type
    exponential_log(const T_y& y, const T_inv_scale& beta, const Policy& = Policy()) {
      static const char* function = "stan::prob::exponential_log<%1%>(%1%)";

      typename promote_args<T_y,T_inv_scale>::type lp(0.0);
      if(!stan::prob::check_positive(function, beta, "Inverse scale", &lp, Policy()))
        return lp;
      if(!stan::prob::check_not_nan(function, y, "Random variate y", &lp, Policy()))
        return lp;
      
      if (include_summand<propto,T_inv_scale>::value)
        lp += log(beta);
      if (include_summand<propto,T_y,T_inv_scale>::value)
        lp -= beta * y;
      return lp;
    }
    
    /**
     * Calculates the exponential cumulative distribution function for the given
     * y and beta.
     *
     * Inverse scale parameter must be greater than 0.
     * y must be greater than or equal to 0.
     * 
     * @param y A scalar variable.
     * @param beta Inverse scale parameter.
     * @tparam T_y Type of scalar.
     * @tparam T_inv_scale Type of inverse scale.
     */
    template <bool propto = false, 
              typename T_y, typename T_inv_scale, 
              class Policy = policy<> >
    inline typename promote_args<T_y,T_inv_scale>::type
    exponential_p(const T_y& y, const T_inv_scale& beta, const Policy& = Policy()) {
      static const char* function = "stan::prob::exponential_p<%1%>(%1%)";

      typename promote_args<T_y,T_inv_scale>::type lp(0.0);
      if(!stan::prob::check_positive(function, beta, "Inverse scale", &lp, Policy()))
        return lp;
      if(!stan::prob::check_not_nan(function, y, "Random variate y", &lp, Policy()))
        return lp;
      
      if (y < 0)
        return lp;
      
      lp = 1.0;
      if (include_summand<propto>::value)
        lp -= exp(-beta * y);
      return lp;
    }


  }
}

#endif
