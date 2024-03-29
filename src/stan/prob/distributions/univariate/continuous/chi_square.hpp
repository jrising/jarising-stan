#ifndef __STAN__PROB__DISTRIBUTIONS__UNIVARIATE__CONTINUOUS__CHI_SQUARE_HPP__
#define __STAN__PROB__DISTRIBUTIONS__UNIVARIATE__CONTINUOUS__CHI_SQUARE_HPP__

#include <stan/agrad.hpp>
#include <stan/math/error_handling.hpp>
#include <stan/math/special_functions.hpp>
#include <stan/meta/traits.hpp>
#include <stan/prob/constants.hpp>
#include <stan/prob/traits.hpp>

namespace stan {

  namespace prob {

    /**
     * The log of a chi-squared density for y with the specified
     * degrees of freedom parameter.
     * The degrees of freedom prarameter must be greater than 0.
     * y must be greater than or equal to 0.
     * 
     \f{eqnarray*}{
     y &\sim& \chi^2_\nu \\
     \log (p (y \,|\, \nu)) &=& \log \left( \frac{2^{-\nu / 2}}{\Gamma (\nu / 2)} y^{\nu / 2 - 1} \exp^{- y / 2} \right) \\
     &=& - \frac{\nu}{2} \log(2) - \log (\Gamma (\nu / 2)) + (\frac{\nu}{2} - 1) \log(y) - \frac{y}{2} \\
     & & \mathrm{ where } \; y \ge 0
     \f}
     * @param y A scalar variable.
     * @param nu Degrees of freedom.
     * @throw std::domain_error if nu is not greater than or equal to 0
     * @throw std::domain_error if y is not greater than or equal to 0.
     * @tparam T_y Type of scalar.
     * @tparam T_dof Type of degrees of freedom.
     */
    template <bool propto,
              typename T_y, typename T_dof, 
              class Policy>
    typename return_type<T_y,T_dof>::type
    chi_square_log(const T_y& y, const T_dof& nu, const Policy&) {
      static const char* function = "stan::prob::chi_square_log(%1%)";

      // check if any vectors are zero length
      if (!(stan::length(y) 
            && stan::length(nu)))
	return 0.0;
      
      using stan::math::check_positive;
      using stan::math::check_finite;
      using stan::math::check_not_nan;
      using stan::math::check_consistent_sizes;
      using stan::math::value_of;
      
      typename return_type<T_y,T_dof>::type logp(0);
      if (!check_not_nan(function, y, "Random variable", &logp, Policy()))
        return logp;
      if (!check_finite(function, nu, "Degrees of freedom parameter", &logp, Policy()))
        return logp;
      if (!check_positive(function, nu, "Degrees of freedom parameter", &logp, Policy()))
        return logp;
	  
      if (!(check_consistent_sizes(function,
                                   y,nu,
				   "Random variable","Degrees of freedom parameter",
                                   &logp, Policy())))
        return logp;
	  
      
      // set up template expressions wrapping scalars into vector views
      VectorView<const T_y> y_vec(y);
      VectorView<const T_dof> nu_vec(nu);
      size_t N = max_size(y, nu);
      
      for (size_t n = 0; n < length(y); n++) 
	if (value_of(y_vec[n]) < 0)
	  return LOG_ZERO;

      // check if no variables are involved and prop-to
      if (!include_summand<propto,T_y,T_dof>::value)
	return 0.0;

      using boost::math::lgamma;
      using stan::math::multiply_log;
      
      for (size_t n = 0; n < N; n++) {
	if (include_summand<propto,T_dof>::value)
	  logp += nu_vec[n] * NEG_LOG_TWO_OVER_TWO - lgamma(0.5 * nu_vec[n]);
	if (include_summand<propto,T_y,T_dof>::value)
	  logp += multiply_log(0.5*nu_vec[n]-1.0, y_vec[n]);
	if (include_summand<propto,T_y>::value)
	  logp -= 0.5 * y_vec[n];
      }
      return logp;
    }


    template <bool propto,
              typename T_y, typename T_dof>
    inline
    typename return_type<T_y,T_dof>::type
    chi_square_log(const T_y& y, const T_dof& nu) {
      return chi_square_log<propto>(y,nu,stan::math::default_policy());
    }


    template <typename T_y, typename T_dof, 
              class Policy>
    inline
    typename return_type<T_y,T_dof>::type
    chi_square_log(const T_y& y, const T_dof& nu, const Policy&) {
      return chi_square_log<false>(y,nu,Policy());
    }


    template <typename T_y, typename T_dof>
    inline
    typename return_type<T_y,T_dof>::type
    chi_square_log(const T_y& y, const T_dof& nu) {
      return chi_square_log<false>(y,nu,stan::math::default_policy());
    }

    /** 
     * Calculates the chi square cumulative distribution function for the given
     * variate and degrees of freedom.
     * 
     * @param y A scalar variate.
     * @param nu Degrees of freedom.
     * 
     * @return The cdf of the chi square distribution
     */
    /*template <typename T_y, typename T_dof, 
              class Policy>
    typename return_type<T_y,T_dof>::type
    chi_square_cdf(const T_y& y, const T_dof& nu, const Policy&) {
      static const char* function = "stan::prob::chi_square_cdf(%1%)";

      using stan::math::check_positive;
      using stan::math::check_finite;
      using stan::math::check_not_nan;
      using return_type;

      typename return_type<T_y,T_dof>::type lp;
      if (!check_not_nan(function, y, "Random variable", &lp, Policy()))
        return lp;
      if (!check_finite(function, nu, "Degrees of freedom parameter", &lp, Policy()))
        return lp;
      if (!check_positive(function, nu, "Degrees of freedom parameter", &lp, Policy()))
        return lp;
      
	// FIXME: include when gamma_cdf() is ready
      return stan::prob::gamma_cdf(y,nu/2,0.5,Policy());
    }

    template <typename T_y, typename T_dof>
    typename return_type<T_y,T_dof>::type
    chi_square_cdf(const T_y& y, const T_dof& nu) {
      return chi_square_cdf(y, nu, stan::math::default_policy());
    }*/
  }
}

#endif

