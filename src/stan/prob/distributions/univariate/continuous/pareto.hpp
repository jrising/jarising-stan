#ifndef __STAN__PROB__DISTRIBUTIONS__PARETO_HPP__
#define __STAN__PROB__DISTRIBUTIONS__PARETO_HPP__

#include <stan/agrad.hpp>
#include <stan/math/error_handling.hpp>
#include <stan/math/special_functions.hpp>
#include <stan/meta/traits.hpp>
#include <stan/prob/constants.hpp>
#include <stan/prob/traits.hpp>


namespace stan {
  namespace prob {

    // Pareto(y|y_m,alpha)  [y > y_m;  y_m > 0;  alpha > 0]
    template <bool propto,
              typename T_y, typename T_scale, typename T_shape, 
              class Policy>
    typename return_type<T_y,T_scale,T_shape>::type
    pareto_log(const T_y& y, const T_scale& y_min, const T_shape& alpha, 
               const Policy&) {
      static const char* function = "stan::prob::pareto_log(%1%)";
      
      using stan::math::value_of;
      using stan::math::check_finite;
      using stan::math::check_positive;
      using stan::math::check_not_nan;
      using stan::math::check_consistent_sizes;

      
      // check if any vectors are zero length
      if (!(stan::length(y) 
            && stan::length(y_min) 
            && stan::length(alpha)))
        return 0.0;
      
      // set up return value accumulator
      double logp(0.0);
      
      // validate args (here done over var, which should be OK)
      if (!check_not_nan(function, y, "Random variable", &logp, Policy()))
        return logp;
      if (!check_finite(function, y_min, "Scale parameter",
                        &logp, Policy()))
        return logp;
      if (!check_positive(function, y_min, "Scale parameter", 
                          &logp, Policy()))
        return logp;
      if (!check_finite(function, alpha, "Shape parameter", 
                        &logp, Policy()))
        return logp;
      if (!check_positive(function, alpha, "Shape parameter", 
                          &logp, Policy()))
        return logp;
      if (!(check_consistent_sizes(function,
				   y,y_min,alpha,
				   "Random variable","Scale parameter","Shape parameter",
                                   &logp, Policy())))
        return logp;

      // check if no variables are involved and prop-to
      if (!include_summand<propto,T_y,T_scale,T_shape>::value)
	return 0.0;
      
      VectorView<const T_y> y_vec(y);
      VectorView<const T_scale> y_min_vec(y_min);
      VectorView<const T_shape> alpha_vec(alpha);
      size_t N = max_size(y, y_min, alpha);

      for (size_t n = 0; n < N; n++) {
	if (y_vec[n] < y_min_vec[n])
	  return LOG_ZERO;
      }

      // set up template expressions wrapping scalars into vector views
      agrad::OperandsAndPartials<T_y,T_scale,T_shape> operands_and_partials(y, y_min, alpha);
      
      DoubleVectorView<include_summand<propto,T_y,T_shape>::value,is_vector<T_y>::value> log_y(length(y));
      if (include_summand<propto,T_y,T_shape>::value)
	for (size_t n = 0; n < length(y); n++)
	  log_y[n] = log(value_of(y_vec[n]));
      DoubleVectorView<!is_constant_struct<T_y>::value||!is_constant_struct<T_shape>::value,is_vector<T_y>::value> inv_y(length(y));
      if (!is_constant_struct<T_y>::value||!is_constant_struct<T_shape>::value)
	for (size_t n = 0; n < length(y); n++)
	  inv_y[n] = 1 / value_of(y_vec[n]);
      DoubleVectorView<include_summand<propto,T_scale,T_shape>::value,is_vector<T_scale>::value> 
	log_y_min(length(y_min));
      if (include_summand<propto,T_scale,T_shape>::value)
	for (size_t n = 0; n < length(y_min); n++)
	  log_y_min[n] = log(value_of(y_min_vec[n]));
      DoubleVectorView<include_summand<propto,T_shape>::value,is_vector<T_shape>::value> log_alpha(length(alpha));
      if (include_summand<propto,T_shape>::value)
	for (size_t n = 0; n < length(alpha); n++)
	  log_alpha[n] = log(value_of(alpha_vec[n]));
      
      DoubleVectorView<!is_constant_struct<T_shape>::value,is_vector<T_shape>::value> inv_alpha(length(alpha));
      if (!is_constant_struct<T_shape>::value)
	for (size_t n = 0; n < length(alpha); n++)
	  inv_alpha[n] = 1 / value_of(alpha_vec[n]);
      
      using stan::math::multiply_log;

      for (size_t n = 0; n < N; n++) {
	const double alpha_dbl = value_of(alpha_vec[n]);
	// log probability
	if (include_summand<propto,T_shape>::value)
	  logp += log_alpha[n];
	if (include_summand<propto,T_scale,T_shape>::value)
	  logp += alpha_dbl * log_y_min[n];
	if (include_summand<propto,T_y,T_shape>::value)
	  logp -= alpha_dbl * log_y[n] + log_y[n];
	
	// gradients
	if (!is_constant_struct<T_y>::value)
	  operands_and_partials.d_x1[n] -= alpha_dbl * inv_y[n] + inv_y[n];
	if (!is_constant_struct<T_scale>::value)
	  operands_and_partials.d_x2[n] += alpha_dbl / value_of(y_min_vec[n]);
	if (!is_constant_struct<T_shape>::value)
	  operands_and_partials.d_x3[n] += 1 / alpha_dbl + log_y_min[n] - log_y[n];
      }
      return operands_and_partials.to_var(logp);
    }


    template <bool propto,
              typename T_y, typename T_scale, typename T_shape>
    inline
    typename return_type<T_y,T_scale,T_shape>::type
    pareto_log(const T_y& y, const T_scale& y_min, const T_shape& alpha) {
      return pareto_log<propto>(y,y_min,alpha,stan::math::default_policy());
    }

    template <typename T_y, typename T_scale, typename T_shape, 
              class Policy>
    inline
    typename return_type<T_y,T_scale,T_shape>::type
    pareto_log(const T_y& y, const T_scale& y_min, const T_shape& alpha, 
               const Policy&) {
      return pareto_log<false>(y,y_min,alpha,Policy());
    }

    template <typename T_y, typename T_scale, typename T_shape>
    inline
    typename return_type<T_y,T_scale,T_shape>::type
    pareto_log(const T_y& y, const T_scale& y_min, const T_shape& alpha) {
      return pareto_log<false>(y,y_min,alpha,stan::math::default_policy());
    }

      template <typename T_y, typename T_scale, typename T_shape, class Policy>
      typename return_type<T_y, T_scale, T_shape>::type
      pareto_cdf(const T_y& y, const T_scale& y_min, const T_shape& alpha, const Policy&) {
          
          // Check sizes
          // Size checks
          if ( !( stan::length(y) && stan::length(y_min) && stan::length(alpha) ) ) return 0.0;
          
          // Check errors
          static const char* function = "stan::prob::pareto_cdf(%1%)";
          
          using stan::math::check_finite;
          using stan::math::check_positive;
          using stan::math::check_not_nan;
          using stan::math::check_greater_or_equal;
          using stan::math::check_consistent_sizes;
          
          using boost::math::tools::promote_args;
          
          double P(1.0);
          
          if (!check_not_nan(function, y, "Random variable", &P, Policy()))
              return P;
          
          if (!check_greater_or_equal(function, y, y_min, "Random variable", &P, Policy()))
              return P;        
          
          if (!check_finite(function, y_min, "Scale parameter", &P, Policy()))
              return P;
          
          if (!check_positive(function, y_min, "Scale parameter", &P, Policy()))
              return P;
          
          if (!check_finite(function, alpha, "Shape parameter", &P, Policy()))
              return P;
          
          if (!check_positive(function, alpha, "Shape parameter", &P, Policy()))
              return P;
          
          if (!(check_consistent_sizes(function, y, y_min, alpha,
                                       "Random variable", "Scale parameter", "Shape parameter",
                                       &P, Policy())))
              return P;
          
          // Wrap arguments in vectors
          VectorView<const T_y> y_vec(y);
          VectorView<const T_scale> y_min_vec(y_min);
          VectorView<const T_shape> alpha_vec(alpha);
          size_t N = max_size(y, y_min, alpha);
          
          agrad::OperandsAndPartials<T_y, T_scale, T_shape> operands_and_partials(y, y_min, alpha);
          
          std::fill(operands_and_partials.all_partials,
                    operands_and_partials.all_partials + operands_and_partials.nvaris, 0.0);
          
          // Compute vectorized CDF and its gradients
          using stan::math::value_of;
          
          for (size_t n = 0; n < N; n++) {
              
              // Pull out values
              const double log_dbl = log( value_of(y_min_vec[n]) / value_of(y_vec[n]) );
              const double y_min_inv_dbl = 1.0 / value_of(y_min_vec[n]);
              const double alpha_dbl = value_of(alpha_vec[n]);
              
              // Compute
              const double Pn = 1.0 - exp( alpha_dbl * log_dbl );
              
              P *= Pn;
              
              if (!is_constant_struct<T_y>::value)
                  operands_and_partials.d_x1[n] 
                  += alpha_dbl * y_min_inv_dbl * exp( (alpha_dbl + 1) * log_dbl ) / Pn;
              
              if (!is_constant_struct<T_scale>::value)
                  operands_and_partials.d_x2[n] 
                  += - alpha_dbl * y_min_inv_dbl * exp( alpha_dbl * log_dbl ) / Pn;
              
              if (!is_constant_struct<T_shape>::value)
                  operands_and_partials.d_x3[n] 
                  += - exp( alpha_dbl * log_dbl ) * log_dbl / Pn;
              
          }
          
          for (size_t n = 0; n < N; n++) {
              
              if (!is_constant_struct<T_y>::value)
                  operands_and_partials.d_x1[n] *= P;
              
              if (!is_constant_struct<T_scale>::value)
                  operands_and_partials.d_x2[n] *= P;
              
              if (!is_constant_struct<T_shape>::value)
                  operands_and_partials.d_x3[n] *= P;
              
          }
          
          return operands_and_partials.to_var(P);
          
      }
      
      template <typename T_y, typename T_scale, typename T_shape>
      inline typename return_type<T_y, T_scale, T_shape>::type
      pareto_cdf(const T_y& y, const T_scale& y_min, const T_shape& alpha) {
          return pareto_cdf(y, y_min, alpha, stan::math::default_policy());
      }
      
  }
}
#endif
