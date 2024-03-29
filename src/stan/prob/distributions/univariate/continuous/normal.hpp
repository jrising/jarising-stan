#ifndef __STAN__PROB__DISTRIBUTIONS__UNIVARIATE__CONTINUOUS__NORMAL_HPP__
#define __STAN__PROB__DISTRIBUTIONS__UNIVARIATE__CONTINUOUS__NORMAL_HPP__

#include <boost/random/normal_distribution.hpp>
#include <boost/random/variate_generator.hpp>

#include <stan/agrad.hpp>
#include <stan/math/error_handling.hpp>
#include <stan/math/special_functions.hpp>
#include <stan/meta/traits.hpp>
#include <stan/prob/constants.hpp>
#include <stan/prob/traits.hpp>

namespace stan {

  namespace prob {

    /**
     * The log of the normal density for the specified scalar(s) given
     * the specified mean(s) and deviation(s). y, mu, or sigma can
     * each be either a scalar or a std::vector. Any vector inputs
     * must be the same length.
     *
     * <p>The result log probability is defined to be the sum of the
     * log probabilities for each observation/mean/deviation triple.
     * @param y (Sequence of) scalar(s).
     * @param mu (Sequence of) location parameter(s)
     * for the normal distribution.
     * @param sigma (Sequence of) scale parameters for the normal
     * distribution.
     * @return The log of the product of the densities.
     * @throw std::domain_error if the scale is not positive.
     * @tparam T_y Underlying type of scalar in sequence.
     * @tparam T_loc Type of location parameter.
     */
    template <bool propto, 
              typename T_y, typename T_loc, typename T_scale,
              class Policy>
    typename return_type<T_y,T_loc,T_scale>::type
    normal_log(const T_y& y, const T_loc& mu, const T_scale& sigma,
               const Policy& /*policy*/) {
      static const char* function = "stan::prob::normal_log(%1%)";

      using std::log;
      using stan::is_constant_struct;
      using stan::math::check_positive;
      using stan::math::check_finite;
      using stan::math::check_not_nan;
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
      if (!check_positive(function, sigma, "Scale parameter", 
                          &logp, Policy()))
        return logp;
      if (!(check_consistent_sizes(function,
                                   y,mu,sigma,
				   "Random variable","Location parameter","Scale parameter",
                                   &logp, Policy())))
        return logp;

      // check if no variables are involved and prop-to
      if (!include_summand<propto,T_y,T_loc,T_scale>::value)
        return 0.0;
      
      // set up template expressions wrapping scalars into vector views
      agrad::OperandsAndPartials<T_y, T_loc, T_scale> operands_and_partials(y, mu, sigma);

      VectorView<const T_y> y_vec(y);
      VectorView<const T_loc> mu_vec(mu);
      VectorView<const T_scale> sigma_vec(sigma);
      size_t N = max_size(y, mu, sigma);

      DoubleVectorView<true,is_vector<T_scale>::value> inv_sigma(length(sigma));
      DoubleVectorView<include_summand<propto,T_scale>::value,is_vector<T_scale>::value> log_sigma(length(sigma));
      for (size_t i = 0; i < length(sigma); i++) {
        inv_sigma[i] = 1.0 / value_of(sigma_vec[i]);
	if (include_summand<propto,T_scale>::value)
	  log_sigma[i] = log(value_of(sigma_vec[i]));
      }

      for (size_t n = 0; n < N; n++) {
        // pull out values of arguments
        const double y_dbl = value_of(y_vec[n]);
        const double mu_dbl = value_of(mu_vec[n]);
      
        // reusable subexpression values
        const double y_minus_mu_over_sigma 
          = (y_dbl - mu_dbl) * inv_sigma[n];
        const double y_minus_mu_over_sigma_squared 
          = y_minus_mu_over_sigma * y_minus_mu_over_sigma;

        static double NEGATIVE_HALF = - 0.5;

        // log probability
        if (include_summand<propto>::value)
          logp += NEG_LOG_SQRT_TWO_PI;
        if (include_summand<propto,T_scale>::value)
          logp -= log_sigma[n];
        if (include_summand<propto,T_y,T_loc,T_scale>::value)
          logp += NEGATIVE_HALF * y_minus_mu_over_sigma_squared;

        // gradients
        double scaled_diff = inv_sigma[n] * y_minus_mu_over_sigma;
        if (!is_constant_struct<T_y>::value)
          operands_and_partials.d_x1[n] -= scaled_diff;
        if (!is_constant_struct<T_loc>::value)
          operands_and_partials.d_x2[n] += scaled_diff;
        if (!is_constant_struct<T_scale>::value)
          operands_and_partials.d_x3[n] 
            += -inv_sigma[n] + inv_sigma[n] * y_minus_mu_over_sigma_squared;
      }
      return operands_and_partials.to_var(logp);
    }


    template <bool propto,
              typename T_y, typename T_loc, typename T_scale>
    inline
    typename return_type<T_y,T_loc,T_scale>::type
    normal_log(const T_y& y, const T_loc& mu, const T_scale& sigma) {
      return normal_log<propto>(y,mu,sigma,stan::math::default_policy());
    }

    template <typename T_y, typename T_loc, typename T_scale, 
              class Policy>
    inline
    typename return_type<T_y,T_loc,T_scale>::type
    normal_log(const T_y& y, const T_loc& mu, const T_scale& sigma, 
               const Policy&) {
      return normal_log<false>(y,mu,sigma,Policy());
    }

    template <typename T_y, typename T_loc, typename T_scale>
    inline
    typename return_type<T_y,T_loc,T_scale>::type
    normal_log(const T_y& y, const T_loc& mu, const T_scale& sigma) {
      return normal_log<false>(y,mu,sigma,stan::math::default_policy());
    }


    /**
     * Calculates the normal cumulative distribution function for the given
     * variate, location, and scale.
     * 
     * \f$\Phi(x) = \frac{1}{\sqrt{2 \pi}} \int_{-\inf}^x e^{-t^2/2} dt\f$.
     * 
     * Errors are configured by policy.  All variables must be finite
     * and the scale must be strictly greater than zero.
     * 
     * @param y A scalar variate.
     * @param mu The location of the normal distribution.
     * @param sigma The scale of the normal distriubtion
     * @return The unit normal cdf evaluated at the specified arguments.
     * @tparam T_y Type of y.
     * @tparam T_loc Type of mean parameter.
     * @tparam T_scale Type of standard deviation paramater.
     * @tparam Policy Error-handling policy.
     */
    template <typename T_y, typename T_loc, typename T_scale,
              class Policy>
    typename return_type<T_y,T_loc,T_scale>::type
    normal_cdf(const T_y& y, const T_loc& mu, const T_scale& sigma, 
             const Policy&) {
      static const char* function = "stan::prob::normal_cdf(%1%)";

      using stan::math::check_positive;
      using stan::math::check_finite;
      using stan::math::check_not_nan;
      using stan::math::check_consistent_sizes;

      typename return_type<T_y, T_loc, T_scale>::type cdf(1);
      if (!check_not_nan(function, y, "Random variable", &cdf, Policy()))
        return cdf;
      if (!check_finite(function, mu, "Location parameter", &cdf, Policy()))
        return cdf;
      if (!check_not_nan(function, sigma, "Scale parameter", 
                         &cdf, Policy()))
        return cdf;
      if (!check_positive(function, sigma, "Scale parameter", 
                          &cdf, Policy()))
        return cdf;
      if (!(check_consistent_sizes(function,
                                   y,mu,sigma,
				   "Random variable","Location parameter","Scale parameter",
                                   &cdf, Policy())))
        return cdf;

      // check if any vectors are zero length
      if (!(stan::length(y) 
            && stan::length(mu) 
            && stan::length(sigma)))
        return 0.0;

      VectorView<const T_y> y_vec(y);
      VectorView<const T_loc> mu_vec(mu);
      VectorView<const T_scale> sigma_vec(sigma);
      size_t N = max_size(y, mu, sigma);
      
      for (size_t n = 0; n < N; n++) {
	cdf *= 0.5 + 0.5 * erf((y_vec[n] - mu_vec[n]) / (sigma_vec[n] * SQRT_2));
      }
      return cdf;
    }

    template <typename T_y, typename T_loc, typename T_scale>
    inline
    typename return_type<T_y, T_loc, T_scale>::type
    normal_cdf(const T_y& y, const T_loc& mu, const T_scale& sigma) {
      return normal_cdf(y,mu,sigma,stan::math::default_policy());
    }


    template <typename T_loc, typename T_scale, class RNG>
    inline double
    normal_random(const T_loc& mu, const T_scale& sigma, RNG& rng) {
      using boost::variate_generator;
      using boost::normal_distribution;
      using stan::math::value_of;
      variate_generator<RNG&, normal_distribution<> >
        rng_unit_norm(rng, normal_distribution<>());
      return value_of(mu)  + value_of(sigma) * rng_unit_norm();
    }
  }
}
#endif
