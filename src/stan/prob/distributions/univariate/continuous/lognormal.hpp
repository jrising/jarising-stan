#ifndef __STAN__PROB__DISTRIBUTIONS__UNIVARIATE__CONTINUOUS__LOGNORMAL_HPP__
#define __STAN__PROB__DISTRIBUTIONS__UNIVARIATE__CONTINUOUS__LOGNORMAL_HPP__

#include <stan/agrad.hpp>
#include <stan/math/error_handling.hpp>
#include <stan/math/special_functions.hpp>
#include <stan/meta/traits.hpp>
#include <stan/prob/constants.hpp>
#include <stan/prob/traits.hpp>

namespace stan {
  namespace prob {

    // LogNormal(y|mu,sigma)  [y >= 0;  sigma > 0]
    // FIXME: document
    template <bool propto,
              typename T_y, typename T_loc, typename T_scale, 
              class Policy>
    typename return_type<T_y,T_loc,T_scale>::type
    lognormal_log(const T_y& y, const T_loc& mu, const T_scale& sigma, 
                  const Policy&) {
      static const char* function = "stan::prob::lognormal_log(%1%)";

      using stan::is_constant_struct;
      using stan::math::check_not_nan;
      using stan::math::check_finite;
      using stan::math::check_positive;      
      using stan::math::check_consistent_sizes;
      using stan::math::value_of;
      using stan::prob::include_summand;


      // check if any vectors are zero length
      if (!(stan::length(y) 
            && stan::length(mu) 
            && stan::length(sigma)))
        return 0.0;

      // set up return value accumulator
      double logp(0.0);

      // validate args (here done over var, which should be OK)
      if (!check_not_nan(function, y, "Random variable", &logp, Policy()))
        return logp;
      if (!check_finite(function, mu, "Location parameter", 
                        &logp, Policy()))
        return logp;
      if (!check_finite(function, sigma, "Scale parameter", 
                        &logp, Policy()))
        return logp;
      if (!check_positive(function, sigma, "Scale parameter", 
                          &logp, Policy()))
        return logp;
      if (!(check_consistent_sizes(function,
                                   y,mu,sigma,
				   "Random variable","Location parameter","Scale parameter",
                                   &logp, Policy())))
        return logp;

      
      VectorView<const T_y> y_vec(y);
      VectorView<const T_loc> mu_vec(mu);
      VectorView<const T_scale> sigma_vec(sigma);
      size_t N = max_size(y, mu, sigma);
      
      for (size_t n = 0; n < length(y); n++)
	if (value_of(y_vec[n]) <= 0)
	  return LOG_ZERO;
      
      agrad::OperandsAndPartials<T_y, T_loc, T_scale> operands_and_partials(y, mu, sigma);
 
      using stan::math::square;
      using std::log;
      using stan::prob::NEG_LOG_SQRT_TWO_PI;
      

      DoubleVectorView<include_summand<propto,T_scale>::value,is_vector<T_scale>::value> log_sigma(length(sigma));
      if (include_summand<propto, T_scale>::value)
	for (size_t n = 0; n < length(sigma); n++)
	  log_sigma[n] = log(value_of(sigma_vec[n]));
      DoubleVectorView<include_summand<propto,T_y,T_loc,T_scale>::value,is_vector<T_scale>::value> inv_sigma(length(sigma));
      DoubleVectorView<include_summand<propto,T_y,T_loc,T_scale>::value,is_vector<T_scale>::value> inv_sigma_sq(length(sigma));
      if (include_summand<propto,T_y,T_loc,T_scale>::value)
	for (size_t n = 0; n < length(sigma); n++)
	  inv_sigma[n] = 1 / value_of(sigma_vec[n]);
      if (include_summand<propto,T_y,T_loc,T_scale>::value)
	for (size_t n = 0; n < length(sigma); n++)
	  inv_sigma_sq[n] = inv_sigma[n] * inv_sigma[n];
      
      DoubleVectorView<include_summand<propto,T_y,T_loc,T_scale>::value,is_vector<T_y>::value> log_y(length(y));
      if (include_summand<propto,T_y,T_loc,T_scale>::value)
	for (size_t n = 0; n < length(y); n++)
	  log_y[n] = log(value_of(y_vec[n]));
      DoubleVectorView<!is_constant_struct<T_y>::value,is_vector<T_y>::value> inv_y(length(y));
      if (!is_constant_struct<T_y>::value)
	for (size_t n = 0; n < length(y); n++)
	  inv_y[n] = 1 / value_of(y_vec[n]);

      if (include_summand<propto>::value)
	logp += N * NEG_LOG_SQRT_TWO_PI;

      for (size_t n = 0; n < N; n++) {
        const double mu_dbl = value_of(mu_vec[n]);

        double logy_m_mu(0);
	if (include_summand<propto,T_y,T_loc,T_scale>::value ||
	    !is_constant_struct<T_y>::value)
	  logy_m_mu = log_y[n] - mu_dbl;

	double logy_m_mu_sq = logy_m_mu * logy_m_mu;
	double logy_m_mu_div_sigma(0);
	if (!is_constant_struct<T_y>::value ||
	    !is_constant_struct<T_loc>::value ||
	    !is_constant_struct<T_scale>::value)
	  logy_m_mu_div_sigma = logy_m_mu * inv_sigma_sq[n];
	

        // log probability
	if (include_summand<propto,T_scale>::value)
	  logp -= log_sigma[n];
	if (include_summand<propto,T_y>::value)
	  logp -= log_y[n];
	if (include_summand<propto,T_y,T_loc,T_scale>::value)
	  logp -= 0.5 * logy_m_mu_sq * inv_sigma_sq[n];

        // gradients
	if (!is_constant_struct<T_y>::value)
	  operands_and_partials.d_x1[n] -=  (1 + logy_m_mu_div_sigma) * inv_y[n];
	if (!is_constant_struct<T_loc>::value)
	  operands_and_partials.d_x2[n] += logy_m_mu_div_sigma;
	if (!is_constant_struct<T_scale>::value)
	  operands_and_partials.d_x3[n] += (logy_m_mu_div_sigma * logy_m_mu - 1) * inv_sigma[n];
      }
      return operands_and_partials.to_var(logp);
    }

    template <bool propto,
              typename T_y, typename T_loc, typename T_scale>
    inline 
    typename return_type<T_y,T_loc,T_scale>::type
    lognormal_log(const T_y& y, const T_loc& mu, const T_scale& sigma) {
      return lognormal_log<propto>(y,mu,sigma,stan::math::default_policy());
    }

    template <typename T_y, typename T_loc, typename T_scale, 
              class Policy>
    inline 
    typename return_type<T_y,T_loc,T_scale>::type
    lognormal_log(const T_y& y, const T_loc& mu, const T_scale& sigma, 
                  const Policy&) {
      return lognormal_log<false>(y,mu,sigma,Policy());
    }

    template <typename T_y, typename T_loc, typename T_scale>
    inline
    typename return_type<T_y,T_loc,T_scale>::type
    lognormal_log(const T_y& y, const T_loc& mu, const T_scale& sigma) {
      return lognormal_log<false>(y,mu,sigma,stan::math::default_policy());
    }




    template <typename T_y, typename T_loc, typename T_scale, 
              class Policy>
    typename boost::math::tools::promote_args<T_y,T_loc,T_scale>::type
    lognormal_cdf(const T_y& y, const T_loc& mu, const T_scale& sigma, 
                const Policy&) {
      static const char* function = "stan::prob::lognormal_cdf(%1%)";

      using stan::math::check_not_nan;
      using stan::math::check_finite;
      using stan::math::check_positive;
      using boost::math::tools::promote_args;

      typename promote_args<T_y,T_loc,T_scale>::type lp;
      if (!check_not_nan(function, y, "Random variable", &lp, Policy()))
        return lp;
      if (!check_finite(function, mu, "Location parameter", &lp, Policy()))
        return lp;
      if (!check_finite(function, sigma, "Scale parameter", 
                        &lp, Policy()))
        return lp;
      if (!check_positive(function, sigma, "Scale parameter", 
                          &lp, Policy()))
        return lp;

      return 0.5 * erfc(-(log(y) - mu)/(sigma * SQRT_2));
    }

    template <typename T_y, typename T_loc, typename T_scale>
    inline
    typename boost::math::tools::promote_args<T_y,T_loc,T_scale>::type
    lognormal_cdf(const T_y& y, const T_loc& mu, const T_scale& sigma) {
      return lognormal_cdf(y,mu,sigma,stan::math::default_policy());
    }
    
  }
}
#endif
