#ifndef __STAN__PROB__DISTRIBUTIONS__UNIVARIATE__CONTINUOUS__LOGISTIC_HPP__
#define __STAN__PROB__DISTRIBUTIONS__UNIVARIATE__CONTINUOUS__LOGISTIC_HPP__

#include <stan/agrad.hpp>
#include <stan/math/error_handling.hpp>
#include <stan/math/special_functions.hpp>
#include <stan/meta/traits.hpp>
#include <stan/prob/constants.hpp>
#include <stan/prob/traits.hpp>

namespace stan {
  namespace prob {

    // Logistic(y|mu,sigma)    [sigma > 0]
    // FIXME: document
    template <bool propto,
              typename T_y, typename T_loc, typename T_scale, 
              class Policy>
    typename return_type<T_y,T_loc,T_scale>::type
    logistic_log(const T_y& y, const T_loc& mu, const T_scale& sigma, 
                 const Policy&) {
      static const char* function = "stan::prob::logistic_log(%1%)";
      
      using stan::math::check_positive;
      using stan::math::check_finite;
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
      if (!check_finite(function, y, "Random variable", &logp, Policy()))
        return logp;
      if (!check_finite(function, mu, "Location parameter",
                        &logp, Policy()))
        return logp;
      if (!check_finite(function, sigma, "Scale parameter", &logp, 
                        Policy()))
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

      DoubleVectorView<!is_constant_struct<T_loc>::value, is_vector<T_loc>::value || is_vector<T_scale>::value> 
	exp_mu_div_sigma(max_size(mu,sigma));
      DoubleVectorView<!is_constant_struct<T_loc>::value, is_vector<T_y>::value || is_vector<T_scale>::value> 
	exp_y_div_sigma(max_size(y,sigma));
      if (!is_constant_struct<T_loc>::value) {
	for (size_t n = 0; n < max_size(mu,sigma); n++) 
	  exp_mu_div_sigma[n] = exp(value_of(mu_vec[n]) / value_of(sigma_vec[n]));
	for (size_t n = 0; n < max_size(y,sigma); n++) 
	  exp_y_div_sigma[n] = exp(value_of(y_vec[n]) / value_of(sigma_vec[n]));
      }

      using stan::math::log1p;
      for (size_t n = 0; n < N; n++) {
	const double y_dbl = value_of(y_vec[n]);
	const double mu_dbl = value_of(mu_vec[n]);
	
	const double y_minus_mu = y_dbl - mu_dbl;
	const double y_minus_mu_div_sigma = y_minus_mu * inv_sigma[n];
	double exp_m_y_minus_mu_div_sigma(0);
	if (include_summand<propto,T_y,T_loc,T_scale>::value)
	  exp_m_y_minus_mu_div_sigma = exp(-y_minus_mu_div_sigma);
	double inv_1p_exp_y_minus_mu_div_sigma(0);
	if (!is_constant_struct<T_y>::value || !is_constant_struct<T_scale>::value)
	  inv_1p_exp_y_minus_mu_div_sigma = 1 / (1 + exp(y_minus_mu_div_sigma));

	if (include_summand<propto,T_y,T_loc,T_scale>::value)
	  logp -= y_minus_mu_div_sigma;
	if (include_summand<propto,T_scale>::value)
	  logp -= log_sigma[n];
	if (include_summand<propto,T_y,T_loc,T_scale>::value)
	  logp -= 2.0 * log1p(exp_m_y_minus_mu_div_sigma);

	if (!is_constant_struct<T_y>::value)
	  operands_and_partials.d_x1[n] += (2 * inv_1p_exp_y_minus_mu_div_sigma - 1) * inv_sigma[n];
	if (!is_constant_struct<T_loc>::value)
	  operands_and_partials.d_x2[n] +=
	    (1 - 2 * exp_mu_div_sigma[n] / (exp_mu_div_sigma[n] + exp_y_div_sigma[n])) * inv_sigma[n];
	if (!is_constant_struct<T_scale>::value)
	  operands_and_partials.d_x3[n] += 
	    ((1 - 2 * inv_1p_exp_y_minus_mu_div_sigma)*y_minus_mu*inv_sigma[n] - 1) * inv_sigma[n];
      }
      return operands_and_partials.to_var(logp);
    }

    template <bool propto,
              typename T_y, typename T_loc, typename T_scale>
    inline
    typename return_type<T_y,T_loc,T_scale>::type
    logistic_log(const T_y& y, const T_loc& mu, const T_scale& sigma) {
      return logistic_log<propto>(y,mu,sigma,stan::math::default_policy());
    }

    template <typename T_y, typename T_loc, typename T_scale,
              class Policy>
    inline
    typename return_type<T_y,T_loc,T_scale>::type
    logistic_log(const T_y& y, const T_loc& mu, const T_scale& sigma, 
                 const Policy&) {
      return logistic_log<false>(y,mu,sigma,Policy());
    }

    template <typename T_y, typename T_loc, typename T_scale>
    inline
    typename return_type<T_y,T_loc,T_scale>::type
    logistic_log(const T_y& y, const T_loc& mu, const T_scale& sigma) {
      return logistic_log<false>(y,mu,sigma,stan::math::default_policy());
    }

      // Logistic(y|mu,sigma) [sigma > 0]
      template <typename T_y, typename T_loc, typename T_scale, class Policy>
      typename return_type<T_y, T_loc, T_scale>::type
      logistic_cdf(const T_y& y, const T_loc& mu, const T_scale& sigma, const Policy&) {
          
          // Size checks
          if ( !( stan::length(y) && stan::length(mu) && stan::length(sigma) ) ) return 0.0;
          
          // Error checks
          static const char* function = "stan::prob::logistic_cdf(%1%)";
          
          using stan::math::check_positive;
          using stan::math::check_finite;
          using stan::math::check_consistent_sizes;
          
          using boost::math::tools::promote_args;
          
          double P(1.0);
          
          if (!check_finite(function, y, "Random variable", &P, Policy()))
              return P;
          
          if (!check_finite(function, mu, "Location parameter", &P, Policy()))
              return P;
          
          if (!check_finite(function, sigma, "Scale parameter", &P, Policy()))
              return P;
          
          if (!check_positive(function, sigma, "Scale parameter", &P, Policy()))
              return P;
          
          if (!(check_consistent_sizes(function, y, mu, sigma,
                                       "Random variable", "Location parameter", "Scale parameter",
                                       &P, Policy())))
              return P;
          
          // Wrap arguments in vectors
          VectorView<const T_y> y_vec(y);
          VectorView<const T_loc> mu_vec(mu);
          VectorView<const T_scale> sigma_vec(sigma);
          size_t N = max_size(y, mu, sigma);
          
          agrad::OperandsAndPartials<T_y, T_loc, T_scale> operands_and_partials(y, mu, sigma);
          
          std::fill(operands_and_partials.all_partials,
                    operands_and_partials.all_partials + operands_and_partials.nvaris, 0.0);
          
          // Compute vectorized CDF and its gradients
          using stan::math::value_of;
          
          for (size_t n = 0; n < N; n++) {
              
              // Pull out values
              const double y_dbl = value_of(y_vec[n]);
              const double mu_dbl = value_of(mu_vec[n]);
              const double sigma_dbl = value_of(sigma_vec[n]);
              const double sigma_inv_vec = 1.0 / value_of(sigma_vec[n]);
              
              // Compute
              const double Pn = 1.0 / ( 1.0 + exp( - (y_dbl - mu_dbl) * sigma_inv_vec ) );
              
              P *= Pn;
              
              if (!is_constant_struct<T_y>::value)
                  operands_and_partials.d_x1[n] 
                  += exp(logistic_log(y_dbl, mu_dbl, sigma_dbl)) / Pn;
              
              if (!is_constant_struct<T_loc>::value)
                  operands_and_partials.d_x2[n] 
                  += - exp(logistic_log(y_dbl, mu_dbl, sigma_dbl)) / Pn;
              
              if (!is_constant_struct<T_scale>::value)
                  operands_and_partials.d_x3[n] 
                  += - (y_dbl - mu_dbl) * sigma_inv_vec * exp(logistic_log(y_dbl, mu_dbl, sigma_dbl)) / Pn;
              
          }
          
          for (size_t n = 0; n < N; n++) {
              
              if (!is_constant_struct<T_y>::value)
                  operands_and_partials.d_x1[n] *= P;
              
              if (!is_constant_struct<T_loc>::value)
                  operands_and_partials.d_x2[n] *= P;
              
              if (!is_constant_struct<T_scale>::value)
                  operands_and_partials.d_x3[n] *= P;
              
          }        
          
          return operands_and_partials.to_var(P);
          
      }
      
      template <typename T_y, typename T_loc, typename T_scale>
      inline typename return_type<T_y, T_loc, T_scale>::type
      logistic_cdf(const T_y& y, const T_loc& mu, const T_scale& sigma) {
          return logistic_cdf(y, mu, sigma, stan::math::default_policy());
      }
      
  }
}
#endif
